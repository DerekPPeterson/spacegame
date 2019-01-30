#include "gamelogic.h"
#include "spaceThings.h"
#include "uithings.h"

#include <exception>


#include "event.h"

using namespace std;

vector<shared_ptr<Object>> GraphicsObjectHandler::getObjects()
{
    return objects;
}

vector<shared_ptr<Renderable>> GraphicsObjectHandler::getRenderables()
{
    vector<shared_ptr<Renderable>> renderables;
    for (auto o : objects) {
        auto r = dynamic_pointer_cast<Renderable>(o);
        if (r) {
            renderables.push_back(r);
        }
    }
    return renderables;
}

void GraphicsObjectHandler::addObject(shared_ptr<Object> object)
{
    if (object->logicId) {
        index[object->logicId] = objects.size();
    }
    objects.push_back(object);
}

shared_ptr<Object> GraphicsObjectHandler::getObject(int logicId)
{
    if (index.find(logicId) != index.end()) {
        return objects[index[logicId]];
    } else {
        return nullptr;
    }
}

void GraphicsObjectHandler::initializePlayer(logic::Player player)
{
    shared_ptr<Deck> newDeck;
    shared_ptr<Hand> newHand;

    if (player.id == playerId) {
        newHand = make_shared<Hand>(glm::vec2(0.9, 0.9));
        newDeck = make_shared<Deck>();
        hand = newHand;
    } else {
        newHand = make_shared<Hand>(glm::vec2(0.9, 0.05));
        newDeck = make_shared<Deck>();
        enemyHand = newHand;
    }
    addObject(newHand);
    addObject(newDeck);

    for (auto logicCard : player.hand) {
        auto card = Card::createFrom(logicCard);
        if (player.id != playerId) {
            card->enemyOwned = true;
        }
        newHand->addCard(card);
        addObject(card);

        if (player.id != playerId) {
            card->setFaceUp(false);
            card->enemyOwned = true;
        }
    }
    for (auto logicCard : player.deck) {
        auto card = Card::createFrom(logicCard);
        card->setVisible(false);
        addObject(card);
    }
}

void GraphicsObjectHandler::startGame(logic::GameState initialState, int myPlayerId) 
{
    auto spacegrid = make_shared<SpaceGrid>();
    addObject(spacegrid);

    vector<std::shared_ptr<Object>> systems = spacegrid->getAllSystems();
    objects.insert(objects.end(), systems.begin(), systems.end());

    for (auto o: systems) {
        auto s = dynamic_pointer_cast<System>(o);
        s->logicId = initialState.getSystemByPos(s->gridx, s->gridy)->id;
        addObject(s);
    }

    // TODO handle players better
    logic::Player me;
    logic::Player opponent;
    for (auto p : initialState.players) {
        if (myPlayerId == p.id) {
            me = p;
        } else {
            opponent = p;
        }
    }
    playerId = me.id;
    initializePlayer(me);
    initializePlayer(opponent);

    stack = make_shared<Stack>();
    addObject(stack);
    discard = make_shared<Discard>();
    addObject(discard);

    for (auto logicShip : initialState.ships) {
        auto ship = SpaceShip::createFrom(logicShip, 
                dynamic_cast<System*>(getObject(logicShip.curSystemId).get()));
        addObject(ship);
    }

    auto button = make_shared<Button>("Pass", glm::vec3(0.9, 0.7, -5), glm::vec3(0, 2, 2), 0.1);
    addObject(button);

    turnIndicator = make_shared<TurnIndicator>(glm::vec3(0.05, 0.05, -5), playerId, initialState.turnInfo);
    addObject(turnIndicator);
}

optional<logic::Action> GraphicsObjectHandler::getSelectedAction()
{
    auto ret = selectedAction;
    selectedAction.reset();
    return ret;
}

void GraphicsObjectHandler::checkEvents()
{
     auto cardId = Event::getNextEvent(EVENT_PLAY_CARD);
     if (cardId) {
         for (auto action : actions) {
             if (action.type == logic::ACTION_PLAY_CARD and action.id == get<int>(*cardId)) {
                selectedAction = action;
                auto card = dynamic_pointer_cast<Card>(getObject(action.id));
                hand->removeCard(card);
                stack->addCard(card);
             }
         }
     }

     auto buttonString = Event::getNextEvent(EVENT_BUTTON_PRESS);
     if (buttonString and get<string>(*buttonString) == "Pass") {
         for (auto action : actions) {
             if (action.type == logic::ACTION_NONE or action.type == logic::ACTION_END_TURN) {
                 selectedAction = action;
             }
         }
     }
}

void GraphicsObjectHandler::updateState(std::vector<logic::Change> changes)
{
    for (auto change : changes) {
        switch (change.type) {
            case logic::CHANGE_RESOLVE_CARD: 
                {
                    auto cardId = get<int>(change.data);
                    auto card = dynamic_pointer_cast<Card>(getObject(cardId));
                    stack->removeCard(card);
                    discard->addCard(card);
                    break;
                }
            case logic::CHANGE_ADD_SHIP: 
                {
                    auto logicShip = get<logic::Ship>(change.data);
                    auto system = dynamic_pointer_cast<System>(getObject(logicShip.curSystemId));
                    auto ship = SpaceShip::createFrom(logicShip, system.get());
                    addObject(ship);
                    break;
                }
            case logic::CHANGE_DRAW_CARD:
                {
                    auto drawInfo = get<pair<int, logic::Card>>(change.data);
                    // TODO handle other player and if card already exists
                    auto card = Card::createFrom(drawInfo.second);
                    addObject(card);
                    if (drawInfo.first == playerId) {
                        hand->addCard(card);
                    } else {
                        card->enemyOwned = true;
                        card->setFaceUp(false);
                        enemyHand->addCard(card);
                    }
                    break;
                }
            case logic::CHANGE_PHASE_CHANGE:
                {
                    auto turnInfo = get<logic::TurnInfo>(change.data);
                    turnIndicator->changeTurn(turnInfo);
                    break;
                }
            case logic::CHANGE_PLAY_CARD:
                {
                    auto cardId = get<int>(change.data);
                    auto card = dynamic_pointer_cast<Card>(getObject(cardId));
                    hand->removeCard(card);
                    enemyHand->removeCard(card);
                    stack->addCard(card);
                    break;
                }
            default:
                LOG_ERROR << "Received unhandled change from server: " << change;
                ;
        }
    }
}
