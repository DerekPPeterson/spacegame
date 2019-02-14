#include "gamelogic.h"
#include "spaceThings.h"
#include "uithings.h"
#include "camera2.h"

#include <exception>


#include "event.h"

using namespace std;

vector<shared_ptr<Object>> GraphicsObjectHandler::getObjects()
{
    return objects;
}

vector<shared_ptr<Renderable>> GraphicsObjectHandler::getRenderables()
{
    // TODO this seems like the wrong place for these
    checkObjectEmits();
    removeObjects();

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

void GraphicsObjectHandler::removeObjects()
{
    for (auto& o : objects) {
        if (o and o->removeThis) {
            index.erase(o->logicId);
            o.reset();
        }
    }
}

void GraphicsObjectHandler::checkObjectEmits()
{
    for (auto o : objects) {
        if (o and o->emit.size()) {
            for (auto emit : o->emit) {
                addObject(emit);
            }
            o->emit.clear();
        }
    }

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
        myResources = make_shared<ResourceDisplay>(glm::vec3(0, 0.90, -5));
        myResources->set(player.resources);
        addObject(myResources);
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

    camera.reset(camera.getPos(), spacegrid->getSystem(1, 1)->getPos());

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

    for (auto logicSys : initialState.systems) {
        auto sys = dynamic_pointer_cast<System>(getObject(logicSys.id));
        sysInfos[logicSys.id] = make_shared<SystemInfo>(sys, playerId);
        for (auto ship : initialState.ships) {
            if (ship.curSystemId == logicSys.id) {
                sysInfos[logicSys.id]->addShip(ship);
            }
        }
        addObject(sysInfos[logicSys.id]);
    }

    passButton = make_shared<Button>("Pass", glm::vec3(0.9, 0.7, -5), glm::vec3(0, 2, 2), 0.1);
    addObject(passButton);

    confirmButton = make_shared<Button>("Confirm", glm::vec3(0.9, 0.78, -5), glm::vec3(0, 2, 0), 0.1);
    addObject(confirmButton);

    turnIndicator = make_shared<TurnIndicator>(glm::vec3(0.05, 0.05, -5), playerId, initialState.turnInfo);
    addObject(turnIndicator);

    debugInfo = make_shared<DebugInfo>();
    addObject(debugInfo);
}

void GraphicsObjectHandler::updateSysInfoActiveButtons()
{
    optional<logic::Action> selectShipAction;
    for (auto action : actions) {
        if (action.type == logic::ACTION_SELECT_SHIPS) {
            selectShipAction = action;
            break;
        }
    }

    if (selectShipAction) {
        for (auto [sysId, sysInfo] : sysInfos) {
            for (auto shipId : selectShipAction->targets) {
                sysInfo->setShipButtonActive(shipId, true);
            }
            sysInfo->setShipButtonSelected(0, false);
            for (auto shipId : shipIdsSelected) {
                sysInfo->setShipButtonSelected(shipId, true);
            }
        }
    } else {
        for (auto [sysId, sysInfo] : sysInfos) {
            sysInfo->setShipButtonActive(0, false);
        }
    }
}

void GraphicsObjectHandler::setPossibleActions(std::vector<logic::Action> actions) 
{
    this->actions = actions;

    if (pendingChanges.size()) {
        return;
    }
    
    // Reset buttons
    passButton->setActive(false);
    confirmButton->setActive(false);

    // If there is only one choice to make then make it
    if (actions.size() == 1 
            and (actions[0].minTargets == actions[0].maxTargets == actions[0].targets.size()
                or actions[0].targets.size() ==0)) {
        selectedAction = actions[0];
    } else {
        // Otherwise update buttons and will wait for new updates
        for (auto a : actions) {
            if (a.type == logic::ACTION_NONE) {
                passButton->setActive(true);
            } else if (a.type == logic::ACTION_SELECT_SHIPS) {
                confirmButton->setActive(true);
                updateSysInfoActiveButtons();
            }
        }
    }
    
    // TODO debug only
    //stringstream ss;
    //ss << actions;
    //debugInfo->addInfo(ss.str());
    
};

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
                break;
             }
         }
     }

     auto buttonString = Event::getNextEvent(EVENT_BUTTON_PRESS);
     if (buttonString) {
         string label = get<string>(*buttonString);
         if (label == "Pass") {
             for (auto action : actions) {
                 if (action.type == logic::ACTION_NONE) {
                     selectedAction = action;
                     break;
                 }
             }
         } else if (label.substr(0, 6) == "shipid") {
             int shipId = stoi(label.substr(6));
             if (shipIdsSelected.count(shipId)) {
                 shipIdsSelected.erase(shipId);
             } else {
                 for (auto action : actions) {
                     if (action.type == logic::ACTION_SELECT_SHIPS) {
                         if (action.maxTargets > shipIdsSelected.size()) {
                             shipIdsSelected.insert(shipId);
                         } else if (action.maxTargets == 1) {
                             shipIdsSelected.clear();
                             shipIdsSelected.insert(shipId);
                         }
                     }
                 }
             }
         } else if (label == "Confirm") {
             for (auto action : actions) {
                 if (action.type == logic::ACTION_SELECT_SHIPS) {
                     action.targets = {};
                     for (auto shipId : shipIdsSelected) {
                         action.targets.push_back(shipId);
                     }
                     shipIdsSelected = {};
                     selectedAction = action;
                     actions = {};
                     updateSysInfoActiveButtons();
                     break;
                 }
             }
         }
     }

     auto logicSysId = Event::getNextEvent(EVENT_SYSTEM_CLICK);
     if (logicSysId) {
         for (auto action : actions) {
             if (action.type == logic::ACTION_PLACE_BEACON 
                     and action.id == get<int>(*logicSysId)) {
                 selectedAction = action;
                 break;
             }
         }
     }

}
float GraphicsObjectHandler::resolveCard(logic::Change change)
{
    auto cardId = get<int>(change.data);
    auto card = dynamic_pointer_cast<Card>(getObject(cardId));
    stack->removeCard(card);
    discard->addCard(card);
    return 0;
}

float GraphicsObjectHandler::addShip(logic::Change change)
{
    auto logicShip = get<logic::Ship>(change.data);
    auto system = dynamic_pointer_cast<System>(getObject(logicShip.curSystemId));
    auto ship = SpaceShip::createFrom(logicShip, system.get());
    sysInfos[logicShip.curSystemId]->addShip(logicShip);
    addObject(ship);
    return 0;
}

float GraphicsObjectHandler::drawCard(logic::Change change)
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
    return 0;
}

float GraphicsObjectHandler::phaseChange(logic::Change change)
{
    auto turnInfo = get<logic::TurnInfo>(change.data);
    turnIndicator->changeTurn(turnInfo);
    return 0;
}

float GraphicsObjectHandler::playCard(logic::Change change)
{
    auto cardId = get<int>(change.data);
    auto card = dynamic_pointer_cast<Card>(getObject(cardId));
    card->setFaceUp(true);
    hand->removeCard(card);
    enemyHand->removeCard(card);
    stack->addCard(card);
    return 2;
}

float GraphicsObjectHandler::changePlayerResources(logic::Change change)
{
    auto data = get<pair<int, ResourceAmount>>(change.data);
    auto changedPlayer = data.first;
    auto newAmount = data.second;
    if (changedPlayer == playerId) {
        myResources->set(newAmount);
    } 
    // TODO add enemy counter
    return 0;
}

float GraphicsObjectHandler::moveShip(logic::Change change)
{
    auto data = get<pair<int, int>>(change.data);
    auto shipId = data.first;
    auto newSysId = data.second;
    auto ship = dynamic_pointer_cast<SpaceShip>(getObject(shipId));
    auto oldSysId = ship->getCurSystemId();
    auto sys = dynamic_pointer_cast<System>(getObject(newSysId));
    ship->gotoSystem(sys.get());
    sysInfos[oldSysId]->removeShip(shipId);
    sysInfos[newSysId]->addShip(ship->logicShipInfo);
    return 0;
}

float GraphicsObjectHandler::removeShip(logic::Change change)
{
    auto shipId = get<int>(change.data);
    auto ship = dynamic_pointer_cast<SpaceShip>(getObject(shipId));
    sysInfos[ship->getCurSystemId()]->removeShip(shipId);
    ship->destroy();
    return rand_float_between(0, 0.5);
}

float GraphicsObjectHandler::shipChange(logic::Change change)
{
    auto logicShip = get<logic::Ship>(change.data);
    sysInfos[logicShip.curSystemId]->removeShip(logicShip.id);
    sysInfos[logicShip.curSystemId]->addShip(logicShip);
    return 0;
}

float GraphicsObjectHandler::combatStart(logic::Change change)
{
    int systemId = get<int>(change.data);
    auto sys = dynamic_pointer_cast<System>(getObject(systemId));
    auto newPos = sys->getPos() + glm::vec3(-10, 5, -10);
    camera.lookAt(newPos, sys->getPos(), 1.0);
    return 3;
}

float GraphicsObjectHandler::combatRound(logic::Change change)
{
    return 0.5;
}

float GraphicsObjectHandler::combatEnd(logic::Change change)
{
    camera.reset(1.0);
    return 0;
}

float GraphicsObjectHandler::shipTargets(logic::Change change)
{
    auto targets = get<vector<pair<int, int>>>(change.data);
    for (auto [shooterId, targetid] : targets) {
        auto shooter = dynamic_pointer_cast<SpaceShip>(getObject(shooterId));
        auto target = dynamic_pointer_cast<SpaceShip>(getObject(targetid));
        shooter->startShootingAt(target);
    }
    return 1;
}

float GraphicsObjectHandler::returnCardToHand(logic::Change change)
{
    auto cardId = get<int>(change.data);
    auto card = dynamic_pointer_cast<Card>(getObject(cardId));
    stack->removeCard(card);
    if (card->enemyOwned) {
        enemyHand->addCard(card);
    } else {
        hand->addCard(card);
    }
    return 0.5;
}


void GraphicsObjectHandler::updateState(std::vector<logic::Change> changes, UpdateInfo info)
{
    for (auto change : changes) {
        pendingChanges.push(change);
    }

    while (pendingChanges.size() and timeToProcessNextChange < info.curTime) {
        auto change = pendingChanges.front();
        pendingChanges.pop();
        float delay = 0;
        LOG_INFO << "Received: " << change;
        switch (change.type) {
            case logic::CHANGE_RESOLVE_CARD: 
                delay = resolveCard(change); break;
            case logic::CHANGE_ADD_SHIP: 
                delay = addShip(change); break;
            case logic::CHANGE_DRAW_CARD:
                delay = drawCard(change); break;
            case logic::CHANGE_PHASE_CHANGE:
                delay = phaseChange(change); break;
            case logic::CHANGE_PLAY_CARD:
                delay = playCard(change); break;
            case logic::CHANGE_PLAYER_RESOURCES:
                delay = changePlayerResources(change); break;
            case logic::CHANGE_MOVE_SHIP:
                delay = moveShip(change); break;
            case logic::CHANGE_REMOVE_SHIP:
                delay = removeShip(change); break;
            case logic::CHANGE_SHIP_CHANGE:
                delay = shipChange(change); break;
            case logic::CHANGE_COMBAT_START:
                delay = combatStart(change); break;
            case logic::CHANGE_COMBAT_ROUND_END:
                delay = combatRound(change); break;
            case logic::CHANGE_COMBAT_END:
                delay = combatEnd(change); break;
            case logic::CHANGE_SHIP_TARGETS:
                delay = shipTargets(change); break;
            case logic::CHANGE_RETURN_CARD_STACK_TO_HAND:
                delay = returnCardToHand(change); break;
            default:
                LOG_ERROR << "Received unhandled change from server: " << change;
        }

        if (delay) {
            timeToProcessNextChange = info.curTime + delay;
        }
    }
}
