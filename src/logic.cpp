#include "logic.h"

#include "carddefinitions.h"

#include <fstream>
#include <algorithm>
#include <queue>

using namespace logic;
using namespace std;

int GameObject::curId = 1;

int totalCost(const ResourceAmount& r)
{
    int total = 0;
    for (auto [type, amount] : r) {
        total += amount;
    }
    return total;
}

optional<ResourceAmount> singlePossiblePayment(ResourceAmount need, ResourceAmount have)
{
    optional<ResourceAmount> notPossible;
    if (not (need <= have)) {
        return notPossible;
    }   

    int resAnyNeeded = need[RESOURCE_ANY];
    if (not resAnyNeeded) {
        return need;
    }

    need[RESOURCE_ANY] = 0;
    auto haveLeft = have - need;

    if (totalCost(haveLeft) == resAnyNeeded) {
        return have;
    }

    ResourceType typeLeft = RESOURCE_ANY;
    for (auto [type, amount] : haveLeft) {
        if (amount > 0 and typeLeft != RESOURCE_ANY) {
            return notPossible;
        } else if (amount > 0) {
            typeLeft = type;
        }
    }

    ResourceAmount usedForAny = {{typeLeft, resAnyNeeded}};
    return need + usedForAny;
}

bool operator == (ResourceAmount a, ResourceAmount b)
{
    for (auto it = a.begin(); it != a.end(); it++) {
        if (not it->second) {
            a.erase(it);
        }
    }
    for (auto it = b.begin(); it != b.end(); it++) {
        if (not it->second) {
            b.erase(it);
        }
    }
    if (a.size() != b.size()) {
        return false;
    }
    return std::equal(a.begin(), a.end(), b.begin());
}

ResourceAmount operator + (ResourceAmount a, const ResourceAmount& b)
{
    for (auto pair : b) {
        if (a.find(pair.first) != a.end()) {
            a[pair.first] += pair.second;
        } else {
            a[pair.first] = pair.second;
        }
    }
    return a;
}

ResourceAmount operator - (ResourceAmount a, const ResourceAmount& b)
{
    if (b.find(RESOURCE_ANY) != b.end()) {
        if (b.at(RESOURCE_ANY) > 0) {
            throw logic_error("Cannot subtract RESOURCE_ANY from anything");
        }
    }
    for (auto pair : b) {
        if (a.find(pair.first) != a.end()) {
            a[pair.first] -= pair.second;
        } else {
            a[pair.first] = -pair.second;
        }
        if (a[pair.first] < 0) {
            if (a.find(RESOURCE_ANY) != a.end()) {
                int needed = - a[pair.first];
                int have = a[RESOURCE_ANY];
                if (have >= needed) {
                    a[pair.first] = 0;
                    a[RESOURCE_ANY] -= needed;
                } else {
                    a[pair.first] += have;
                    a[RESOURCE_ANY] = 0;
                }
            }
        }
    }

    return a;
}

bool operator >= (const ResourceAmount& a, const ResourceAmount& b)
{
    ResourceAmount b_copy = b;
    int bResourceAnyAmount = b_copy[RESOURCE_ANY];
    b_copy[RESOURCE_ANY] = 0;

    auto difference = a - b_copy;
    int remaining = 0;
    for (auto pair : difference) {
        if (pair.second < 0) {
            return false;
        } else {
            remaining += pair.second;
        }
    }
    
    if (bResourceAnyAmount > remaining) {
        return false;
    }

    return true;
}

bool operator <= (const ResourceAmount& a, const ResourceAmount& b)
{
    ResourceAmount a_copy = a;
    int aResourceAnyAmount = a_copy[RESOURCE_ANY];
    a_copy[RESOURCE_ANY] = 0;

    auto difference = b - a_copy;
    int remaining = 0;
    for (auto pair : difference) {
        if (pair.second < 0) {
            return false;
        } else {
            remaining += pair.second;
        }
    }
    if (aResourceAnyAmount > remaining) {
        return false;
    }
    return true;
}

// TODO dynamically load deck from somewhere
void GameState::writeStateToFile(string filename)
{
    ofstream ofs(filename);
    cereal::JSONOutputArchive oarchive(ofs);
    oarchive(*this);
}

/* Create a n x n grid of systems 
 * Initialize adjacency lists
 */
list<logic::System> createSystems(int n)
{
    vector<logic::System> systems;
    systems.resize(n * n);

    int dirs[][2] = {
        {1, 0},
        {-1, 0},
        {0, 1},
        {0, -1},
        {1, -1},
        {-1, 1}
    };
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            for (auto dir : dirs) {
                int adjacentPos[2] = {i + dir[0], j + dir[1]};
                if (adjacentPos[0] < 0 or adjacentPos[0] >= n or
                        adjacentPos[1] < 0 or adjacentPos[1] >= n) {
                    continue;
                } else {
                    systems[n * i + j].adjacent.push_back(
                            systems[n * adjacentPos[0] + adjacentPos[1]].id);
                    systems[n * i + j].i = i;
                    systems[n * i + j].j = j;
                }
            }
        }
    }

    systems[0].home = true;
    //systems[n * n - 1].home = true;
    systems[1].home = true;

    return list<System> {begin(systems), end(systems)};
}

void GameState::startGame()
{
    LOG_INFO << "Setting up game";

    LOG_INFO << "Initilializing systems";
    systems = createSystems(SPACEGRID_SIZE);

    LOG_INFO << "Creating player objects";
    for (int i = 0; i < 2; i++) {
        // Create player objects

        Player p = {
            .name = "Player" + to_string(i),
            .resources = {},
        };

        // TODO load deck dynamically
        list<logic::Card> deck;
        vector<Card> toUse = {
            //CardDefinitions::sample_ship2,
            CardDefinitions::ai_coreship,
            CardDefinitions::subtle_hack,
            //CardDefinitions::am_gatherer,
            //CardDefinitions::am_laser,
            //CardDefinitions::resource_ship,
            //CardDefinitions::diplomaticVessal,
            CardDefinitions::droneSwarm,
        };
        for (int i = 0; i < 40; i++) {
            Card card = toUse[i % toUse.size()];
            card.newId();
            card.ownerId = p.id;
            deck.push_back(card);
        }
        p.deck = deck;

        for (int i = 0; i < 7; i++) {
            p.draw();
        }
        LOG_INFO << "Adding player " << p.name;

        // Give them flagships TODO load chosen flagshipss
        LOG_INFO << "Added flagship for " << p.name;
        logic::Ship flagship = ShipDefinitions::defaultFlagship;
        flagship.controller = p.id;
        flagship.owner = p.id;
        flagship.newId();

        for (auto& s : systems) {
            if (s.home and not s.controllerId) {
                flagship.curSystemId = s.id;
                s.controllerId = p.id;
                break;
            }
        }

        p.flagshipId = flagship.id;
        ships.push_back(flagship);
        players.push_back(p);
    }

    turnInfo = {
        .whoseTurn = players.front().id,
        .activePlayer = players.front().id,
        .phase = {PHASE_MAIN},
    };

    updateSystemControllers();

    upkeep(true);
};

template <typename T>
T* getById(int id, list<T> &vec)
{
    auto it = find_if(vec.begin(), vec.end(), [id] (T& t) {return t.id == id;});
    if (it == vec.end()) {
        return nullptr;
    } else {
        return &(*it);
    }
}

Player* GameState::getPlayerById(int id) 
{
    return getById<Player>(id, players);
}

Ship* GameState::getShipById(int id) 
{
    return getById<Ship>(id, ships);
}

void GameState::deleteShipById(int id) 
{
    for (auto it = ships.begin(); it != ships.end(); it++) {
        if (it->id == id) {
            ships.erase(it);
            break;
        }
    }
}

WarpBeacon* GameState::getBeaconById(int id) 
{
    return getById<WarpBeacon>(id, beacons);
}

System* GameState::getSystemById(int id) 
{
    return getById<System>(id, systems);
}

System* GameState::getSystemByPos(int i, int j) 
{
    auto it = find_if(systems.begin(), systems.end(), [i, j] (System& s) 
            {return s.i == i and s.j == j;});
    if (it == systems.end()) {
        return nullptr;
    } else {
        return &(*it);
    }
}

Card* GameState::getCardById(int id) 
{
    list<list<Card>*> cardLists;
    for (auto& p : players) {
        cardLists.push_back(&p.deck);
        cardLists.push_back(&p.hand);
        cardLists.push_back(&p.discard);
    }
    
    Card* card = nullptr;
    for (auto l : cardLists) {
        card = getById(id, *l);
    }
    return card;
}

int GameState::otherPlayer(int playerId) {
    for (auto p : players) {
        if (p.id != playerId) {
            return p.id;
        }
    }
    return 0;
}

vector<Change> GameState::getChangesAfter(int changeNo)
{
    for (int i = 0; i < changes.size(); i++) {
        changes[i].changeNo = i + 1;
    }
    vector<Change> relevant;
    relevant.insert(relevant.end(), changes.begin() + changeNo, changes.end());
    return relevant;
}

vector<Action> GameState::getValidCardActions()
{
    // TODO more complicated logic
    vector<Action> actions;

    if (turnInfo.phase.back() == PHASE_MAIN) {
        auto player = getPlayerById(turnInfo.activePlayer);
        for (auto& card : player->hand) {
            if (card.cost <= player->resources 
                    and card.canPlay(*this)
                    and not (card.type == CARD_RESOURCE_SHIP 
                             and player->playedResourceShipThisTurn)) {

                auto payment = singlePossiblePayment(card.cost, player->resources);
                ResourceAmount nothing = {};

                Action action = {
                    .type = ACTION_PLAY_CARD,
                    .playerId = turnInfo.activePlayer,
                    .id = card.id,
                    .description = card.name,
                    .payWith = payment ? *payment : nothing,
                    .needToPickCost = not payment,
                };
                actions.push_back(action);
            }
        }
    }

    if (turnInfo.phase.back() == PHASE_RESOLVE_STACK) {
        auto player = getPlayerById(turnInfo.activePlayer);
        for (auto& card : player->hand) {
            if (card.cost <= player->resources 
                    and card.canPlay(*this)
                    and card.type == CARD_INSTANT_ACTION) {
                Action action = {
                    .type = ACTION_PLAY_CARD,
                    .playerId = turnInfo.activePlayer,
                    .id = card.id,
                    .description = card.name,
                };
                actions.push_back(action);
            }
        }
    }

    return actions;
}

set<int> GameState::shipCanReach(int shipId)
{
    set<int> canReach;
    auto ship = getShipById(shipId);

    // pairs of systemId / movement left
    queue<pair<int, int>> toVisit;
    toVisit.push({ship->curSystemId, ship->movement});
    while (toVisit.size()) {
        auto sysId = toVisit.front().first;
        auto movementLeft = toVisit.front().second;
        toVisit.pop();

        if (not movementLeft) {
            continue;
        }

        for (auto nextSysId : getSystemById(sysId)->adjacent) {
            if (nextSysId != ship->curSystemId 
                    and canReach.find(nextSysId) == canReach.end()) {
                toVisit.push({nextSysId, movementLeft - 1});
                canReach.insert(nextSysId);
            }
        }
    }
    return canReach;
}


vector<Action> GameState::getValidBeaconActions()
{
    // TODO more complicated logic
    vector<Action> actions;
    auto player = getPlayerById(turnInfo.whoseTurn);

    ResourceAmount needed = {{RESOURCE_WARP_BEACONS, 1}};
    if (turnInfo.phase.back() == PHASE_MAIN and (player->resources >= needed)) {

        set<int> possibleSystems;
        for (auto s : ships) {
            if (s.controller == turnInfo.whoseTurn) {
                auto systems = shipCanReach(s.id);
                possibleSystems.insert(systems.begin(), systems.end());
            }
        }
        for (auto sys : possibleSystems) {
            actions.push_back({
                .type = ACTION_PLACE_BEACON,
                .playerId = turnInfo.whoseTurn,
                .id = sys,
                .description = "Place beacon in system " + to_string(sys),
            });
        }
    }
    
    return actions;
}

vector<Action> GameState::getValidShipsForBeacon(int beaconId)
{
    auto beacon = getBeaconById(beaconId);
    if (turnInfo.phase.back() == PHASE_SELECT_BEACON_TARGETS) {
        set<int> possibleShips;

        for (auto s : ships) {
            if (s.controller == turnInfo.whoseTurn) {
                auto systems = shipCanReach(s.id);
                if (systems.find(beacon->systemId) != systems.end()) {
                    possibleShips.insert(s.id);
                }
            }
        }
        vector<int> targets;
        targets.insert(targets.begin(), possibleShips.begin(), possibleShips.end());
        stringstream ss;
        ss << targets;
        Action action = {
            .type = ACTION_SELECT_SHIPS,
            .playerId = turnInfo.whoseTurn,
            .targets = targets,
            .minTargets = 0,
            .maxTargets = (int) targets.size(),
            .description = "Select any number of the given ships: " + ss.str(),
        };
        return {action};
    }
    return {};
}

vector<Action> GameState::getTargetActions()
{
    Action action;
    vector<Target> targets;
    vector<Action> actions;
    if (turnInfo.phase.back() == PHASE_SELECT_CARD_TARGETS) {
        auto card = stack.back();
        auto [minTargets, maxTargets, targets] = card.getValidTargets(*this);

        Action selectShipsAction = {
            .type = ACTION_SELECT_SHIPS,
            .playerId = card.playedBy,
            .id = card.id,
            .minTargets = minTargets,
            .maxTargets = maxTargets,
            .description = "Select targets from list for card: " + card.name,
        };

        Action selectSystemsAction = {
            .type = ACTION_SELECT_SYSTEM,
            .playerId = card.playedBy,
            .id = card.id,
            .minTargets = minTargets,
            .maxTargets = maxTargets,
            .description = "Select targets from list for card: " + card.name,
        };

        for (auto t : targets) {
            switch (t.type) {
                case TARGET_SHIP:
                    selectShipsAction.targets.push_back(t.id);
                    break;
                case TARGET_SYSTEM:
                    selectSystemsAction.targets.push_back(t.id);
                    break;
            }
        }
        if (selectShipsAction.targets.size()) {
            actions.push_back(selectShipsAction);
        }
        if (selectSystemsAction.targets.size()) {
            actions.push_back(selectSystemsAction);
        }
    }
    return actions;
}

vector<Action> GameState::getPossibleActions(int playerId)
{
    vector<Action> actions;

    Action pass = 
    {
        .type = ACTION_NONE,
        .playerId = turnInfo.activePlayer,
    };


    // Cannot do nothing if a target selection is needed
    if (turnInfo.phase.back() != PHASE_SELECT_CARD_TARGETS 
            and turnInfo.phase.back() != PHASE_SELECT_BEACON_TARGETS) {
        actions.push_back(pass);
    }

    auto cardActions = getValidCardActions();
    auto moveActions = getValidBeaconActions();
    auto targetActions = getTargetActions();
    auto beaconTargetActions = getValidShipsForBeacon(beacons.back().id);

    actions.insert(actions.end(), cardActions.begin(), cardActions.end());
    actions.insert(actions.end(), moveActions.begin(), moveActions.end());
    actions.insert(actions.end(), targetActions.begin(), targetActions.end());
    actions.insert(actions.end(), beaconTargetActions.begin(), beaconTargetActions.end());

    if (playerId) {
        vector<Action> actionsForPlayer;
        copy_if(actions.begin(), actions.end(), back_inserter(actionsForPlayer),
                [playerId](Action a) {return a.playerId == playerId;});

        return actionsForPlayer;
    } else {
        return actions;
    }
}

void GameState::upkeep(bool firstTurn)
{
    auto player = getPlayerById(turnInfo.whoseTurn);

    if (not firstTurn) {
        drawCard(player->id);
    }

    for (auto& ship : ships) {
        if (ship.controller == turnInfo.whoseTurn) {
            ship.upkeep(*this, ship);
        }
    }
    changes.push_back({
            .type = CHANGE_PLAYER_RESOURCES, 
            .data=pair<int, ResourceAmount>(player->id, player->resources)
            });

    player->playedResourceShipThisTurn = false;
}

void GameState::drawCard(int playerId) {
    auto drawInfo = getPlayerById(turnInfo.whoseTurn)->draw();
    changes.push_back({.type = CHANGE_DRAW_CARD, .data = drawInfo});
}

void GameState::updateSystemControllers()
{
    for (auto& sys : systems) {
        sys.controllerId = 0;
    }
    for (auto ship : ships) {
        auto sys = getSystemById(ship.curSystemId);
        sys->controllerId = ship.controller;
    }
}

void GameState::performAction(Action action)
{
    switch (turnInfo.phase.back()) {
        case PHASE_UPKEEP: {
            // TODO handle fast actions
            LOG_INFO << "End upkeep, start main phase";
            turnInfo.phase[0] = PHASE_MAIN;
            changes.push_back({.type = CHANGE_PHASE_CHANGE, .data = turnInfo});
            break;
        }
        case PHASE_MAIN:
            switch (action.type) {
                case ACTION_PLAY_CARD:
                    playCard(action.id, turnInfo.activePlayer, action.payWith); 
                    // May change phase to a target select phase
                    break;
                case ACTION_PLACE_BEACON:
                    placeBeacon(action.id, turnInfo.activePlayer);
                    break;
                case ACTION_NONE: 
                    turnInfo.phase[0] = PHASE_END;
                    changes.push_back({.type = CHANGE_PHASE_CHANGE, .data = turnInfo});
                    break;
                case ACTION_SELECT_SHIPS:
                case ACTION_SELECT_SYSTEM:
                    LOG_ERROR << "Cannot select ships / system from main phase";
                    break;
            }
            break;
        case PHASE_MOVE:
            break; // TODO
        case PHASE_END:
            // TODO handle other player issues
            endTurn();
            upkeep();
            break;
        case PHASE_RESOLVE_STACK:
            switch(action.type) {
                case ACTION_PLAY_CARD:
                    playCard(action.id, turnInfo.activePlayer, action.payWith);
                    break;
                case ACTION_NONE:
                    {
                        auto card = stack.back();
                        if (turnInfo.activePlayer == card.playedBy) {
                            turnInfo.activePlayer = otherPlayer(turnInfo.activePlayer);
                        } else {
                            resolveStackTop();
                            turnInfo.activePlayer = otherPlayer(turnInfo.activePlayer);
                        }
                        if (stack.size() == 0) {
                            turnInfo.activePlayer = turnInfo.whoseTurn;
                        }
                        break;
                    }
                default:
                    LOG_ERROR << "Invalid action type when resolving stack";
            }
            break;
        case PHASE_SELECT_CARD_TARGETS:
            stack.back().targets = action.targets;
            turnInfo.phase.pop_back();
            break;
        case PHASE_SELECT_BEACON_TARGETS:
            turnInfo.phase.pop_back();
            moveShipsToBeacon(beacons.back().id, action.targets);
            beacons.pop_back();
            break;
    }
}

void GameState::playCard(int cardId, int playerId, ResourceAmount payWith)
{
    auto player = getPlayerById(playerId);
    auto card = find_if(player->hand.begin(), player->hand.end(), 
            [cardId] (Card& c) {return c.id == cardId;});
    card->playedBy = playerId;

    if (card->cost[RESOURCE_ANY] == 0) {
        payWith = card->cost;
    } else if (totalCost(payWith) == 0) {
        LOG_ERROR << "Did not specify how to pay for card";
    }

    if (player->resources >= payWith and payWith >= card->cost) {
        player->resources = player->resources - payWith;
    } else {
        LOG_ERROR << "Amount specified to play card is either not enough or the player does not have enough";
    }

    changes.push_back({
            .type = CHANGE_PLAYER_RESOURCES, 
            .data=pair<int, ResourceAmount>(player->id, player->resources)
            });


    if (turnInfo.phase.back() != PHASE_RESOLVE_STACK) {
        turnInfo.phase.push_back(PHASE_RESOLVE_STACK);
        turnInfo.activePlayer = otherPlayer(turnInfo.activePlayer);
    }

    if (card->getValidTargets) {
        turnInfo.phase.push_back(PHASE_SELECT_CARD_TARGETS);
    }

    changes.push_back({.type = CHANGE_PLAY_CARD, .data = card->id});
    stack.push_back(*card);
    player->hand.erase(card);

    LOG_INFO << "Played card: " << card->name;

}

void GameState::resolveStackTop()
{
    auto card = stack.back();
    changes.push_back({.type = CHANGE_RESOLVE_CARD, .data=card.id});
    card.resolve(*this);
    stack.pop_back();
    auto player = getPlayerById(card.playedBy);
    player->discard.push_back(card);
    if (stack.size() == 0) {
        turnInfo.phase.pop_back();
    }
}

void GameState::placeBeacon(int systemId, int ownerId)
{
    WarpBeacon beacon = {
        .ownerId = ownerId,
        .systemId = systemId,
    };
    beacons.push_back(beacon);
    turnInfo.phase.push_back(PHASE_SELECT_BEACON_TARGETS);
    changes.push_back({.type = CHANGE_PLACE_BEACON, .data = beacon});

    ResourceAmount needed = {{RESOURCE_WARP_BEACONS, 1}};
    auto player = getPlayerById(ownerId);
    player->resources = player->resources - needed;
    changes.push_back({
            .type = CHANGE_PLAYER_RESOURCES, 
            .data = pair<int, ResourceAmount>(ownerId, player->resources)
            });
}

void GameState::moveShipsToBeacon(int beaconId, vector<int> ships)
{
    auto beacon = getBeaconById(beaconId);
    for (auto shipId : ships) {
        auto ship = getShipById(shipId);
        ship->curSystemId = beacon->systemId;
        changes.push_back({
                .type = CHANGE_MOVE_SHIP, 
                .data=pair<int, int>(shipId, beacon->systemId)
                });
    };
    resolveCombats();
    updateSystemControllers();
    LOG_INFO << "Moving ship ids: " << ships << " to system id: " << beacon->systemId;
}

void GameState::endTurn()
{
    // Advance the turn to the next player
    auto it = find_if(players.begin(), players.end(), 
            [this] (Player& p) {return p.id == turnInfo.whoseTurn;});
    LOG_INFO << "Turn ending for playerid " << it->id;
    it++;
    if (it == players.end()) {
        it = players.begin();
    }
    auto nextPlayerId = it->id;
    turnInfo.whoseTurn = nextPlayerId;
    turnInfo.activePlayer = nextPlayerId;

    changes.push_back({.type = CHANGE_PHASE_CHANGE, .data = turnInfo});

    LOG_INFO << "It is now player id " << nextPlayerId << "'s turn" << endl;
    turnInfo.phase[0] = PHASE_UPKEEP;
};

vector<Ship*> GameState::getShipsBySystem(int sysId)
{
    vector<Ship*> ret;
    for (auto it = ships.begin(); it != ships.end(); it++) {
        if (it->curSystemId == sysId) {
            ret.push_back(&(*it));
        }
    }
    return ret;
}

void Ship::applyDamage(int damage)
{
    shield -= damage;
    if (shield < 0) {
        armour += shield;
        shield = 0;
    }
}

void GameState::resolveCombats()
{
    for (auto sys : systems) {
        // Get all the ships sorted by player
        auto ships = getShipsBySystem(sys.id);
        map<int, vector<Ship*>> playerShips;
        for (auto player : players) {
            playerShips[player.id] = {};
        }
        for (auto ship : ships) {
            playerShips[ship->controller].push_back(ship);
        }
        
        // Go to next system if there will be no combat here
        bool noWinner = true;
        for (auto player : players) {
            if (playerShips[player.id].size() == 0) {
                goto next_system;
            }
        }
        changes.push_back({.type = CHANGE_COMBAT_START, .data = sys.id});
        LOG_INFO << "Combat starting in system: " << sys.id;

        // Sort ships by "impressiveness"
        for (auto player : players) {
            sort(playerShips[player.id].begin(), playerShips[player.id].end(),
                    [](Ship* a, Ship* b) {
                        return a->attack + a->armour + a->shield 
                            < b->attack + b->armour + b->shield;
                    });
        }

        
        // Continue until there are no ships left
        while (noWinner) {
            // Each players ships deal damage to each other players ships
            
            vector<pair<int, int>> shipTargets;
            for (auto player : players) {
                auto myShips = playerShips[player.id];
                int otherPlayer = find_if(players.begin(), players.end(), 
                        [player](Player p) {return player.id != p.id;})->id;
                auto enemyShips = playerShips[otherPlayer];

                auto enemyShipIt = enemyShips.begin();
                for (auto myShip : myShips) {
                    if (myShip->kind == SHIP_RESOURCE) {
                        continue;
                    }
                    shipTargets.push_back({myShip->id, (*enemyShipIt)->id});
                    (*enemyShipIt)->applyDamage(myShip->attack);
                    enemyShipIt++;
                    if (enemyShipIt == enemyShips.end()) {
                        enemyShipIt = enemyShips.begin();
                    }
                }
            }
            changes.push_back({.type = CHANGE_SHIP_TARGETS, .data = shipTargets});

            // Remove destroyed ships
            for (auto player : players) {
                vector<Ship*> newShipList;
                for (auto ship : playerShips[player.id]) {
                    if (ship->isDestroyed()) {
                        changes.push_back({.type = CHANGE_REMOVE_SHIP, .data = ship->id});
                        deleteShipById(ship->id);
                        LOG_INFO << "Ship id: " << ship->id << " destroyed";
                    } else {
                        changes.push_back({.type = CHANGE_SHIP_CHANGE, .data = *ship});
                        newShipList.push_back(ship);
                    }
                }
                playerShips[player.id] = newShipList;
                if (newShipList.size() == 0) {
                    noWinner = false;
                }
            }

            changes.push_back({.type = CHANGE_COMBAT_ROUND_END});
        }
        changes.push_back({.type = CHANGE_COMBAT_END});

        next_system:;
    }
}

pair<int, Card> Player::draw()
{
    if (not deck.size()) {
        // TODO handle drawing from empty deck
        // probably instant loss
        return {id, Card()};
        LOG_INFO << "Player " << name << " drew from an empty deck";
    };
    auto card = deck.back();
    deck.pop_back();
    hand.push_back(card);
    LOG_INFO << "Player " << name << " drew a card: " << card.name;
    return {id, card};
};

ostream & logic::operator<< (ostream &out, const Action &c)
{
    out << "(Action: ";
    switch (c.type) {
        case ACTION_NONE:
            out << "none"; break;
        case ACTION_PLACE_BEACON:
            out << "place beacon"; break;
        case ACTION_PLAY_CARD:
            out << "play card"; break;
        case ACTION_SELECT_SHIPS:
            out << "select ships"; break;
        case ACTION_SELECT_SYSTEM:
            out << "select system"; break;
    }
    if (c.description.size()) {
        out << " " << c.description;
    }
    if (c.targets.size()) {
        out << " pick " << c.minTargets << "-" << c.maxTargets << " targets: " << c.targets;
    }
    out << ")";

    return out;
}

ostream & logic::operator<< (ostream &out, const TurnPhases &c)
{
    switch (c) {
        case PHASE_UPKEEP:
            out << "upkeep"; break;
        case PHASE_MAIN:
            out << "main"; break;
        case PHASE_MOVE:
            out << "move"; break;
        case PHASE_END:
            out << "end"; break;
        case PHASE_SELECT_CARD_TARGETS:
            out << "select card targets"; break;
        case PHASE_SELECT_BEACON_TARGETS:
            out << "select beacon targets"; break;
        case PHASE_RESOLVE_STACK:
            out << "resolve stack"; break;
    }
    return out;
}

ostream & logic::operator<< (ostream &out, const CardType c)
{
    switch (c) {
        case CARD_SHIP:
            out << "Construct Ship"; break;
        case CARD_RESOURCE_SHIP:
            out << "Construct Resource Ship"; break;
        case CARD_ACTION:
            out << "Action"; break;
        case CARD_INSTANT_ACTION:
            out << "Instant Action"; break;
        case CARD_STRUCTURE:
            out << "Construct Structure"; break;
    }
    return out;
}

ostream & logic::operator<< (ostream &out, const GameState &c)
{
    out << "GameState:\n";
    out << "\tPlayer id of current turn: " << c.turnInfo.whoseTurn << endl;
    out << "\tCurrent phase: " << c.turnInfo.phase << endl;
    out << "\tPlayers:" << endl;
    for (auto& p : c.players) {
        cout << "\t\t" << p << endl;
    }
    out << "\tShips:" << endl;
    for (auto& s : c.ships) {
        cout << "\t\t" << s << endl;
    }
    out << "\tWarp Beacons:" << endl;
    for (auto& b : c.beacons) {
        cout << "\t\t" << b << endl;
    }
    out << "\tStack:" << endl;
    for (auto& card : c.stack) {
        out << "\t\t" << card << endl;
    }
    return out;
}

