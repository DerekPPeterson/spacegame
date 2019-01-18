#include "gamelogic.h"
#include "spaceThings.h"
#include "cards.h"
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
        throw runtime_error("Could not find logicId in graphics object index");
    }
}

void GraphicsObjectHandler::initializePlayer(logic::Player player)
{
    shared_ptr<Hand> hand(new Hand());
    addObject(hand);
    shared_ptr<Deck> deck(new Deck({}));
    addObject(deck);

    for (auto logicCard : player.hand) {
        auto card = Card::createFrom(logicCard);
        hand->addCard(card);
        addObject(card);
    }
    for (auto logicCard : player.deck) {
        auto card = Card::createFrom(logicCard);
        card->setVisible(false);
        addObject(card);
    }
}

void GraphicsObjectHandler::startGame(logic::GameState initialState) 
{
    shared_ptr<SpaceGrid> spacegrid(new SpaceGrid);
    addObject(spacegrid);

    vector<std::shared_ptr<Object>> systems = spacegrid->getAllSystems();
    objects.insert(objects.end(), systems.begin(), systems.end());

    for (auto o: systems) {
        auto s = dynamic_pointer_cast<System>(o);
        s->logicId = initialState.getSystemByPos(s->gridx, s->gridy)->id;
        addObject(s);
    }

    initializePlayer(initialState.players.front());

    for (auto logicShip : initialState.ships) {
        auto ship = SpaceShip::createFrom(logicShip, 
                dynamic_cast<System*>(getObject(logicShip.curSystemId).get()));
        addObject(ship);
    }
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
             }
         }
     }
}

void GraphicsObjectHandler::updateState(std::vector<logic::Change> changes)
{
    for (auto change : changes) {
        switch (change.type) {
            case logic::CHANGE_PLAY_CARD:
                {
                    auto cardId = get<int>(change.data);
                    auto card = dynamic_pointer_cast<Card>(getObject(cardId));
                    //card->play();
                    break;
                }
            default:
                ;
        }
    }
}
