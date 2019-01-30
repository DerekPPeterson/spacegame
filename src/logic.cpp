#include "logic.h"

#include "carddefinitions.h"

#include <fstream>
#include <algorithm>

using namespace logic;
using namespace std;

int GameObject::curId = 1;


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
    systems[n * n - 1].home = true;

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
            .resources = {{RESOURCE_WARP_BEACONS, {.amount = 0, .max=1, .perTurn=1}}},
        };

        // TODO load deck dynamically
        list<logic::Card> deck;
        for (int i = 0; i < 40; i++) {
            auto card = CardDefinitions::sample_ship;
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
        logic::Ship sampleFlagship = {
            .type = "SS1",
            .attack = 1,
            .shield = 10,
            .armour = 10,
            .movement = 1,
            .owner = p.id,
            .controller = p.id
        };

        for (auto& s : systems) {
            if (s.home and not s.controllerId) {
                sampleFlagship.curSystemId = s.id;
                s.controllerId = p.id;
                break;
            }
        }
        p.flagshipId = sampleFlagship.id;
        ships.push_back(sampleFlagship);
        players.push_back(p);
    }

    turnInfo = {
        .whoseTurn = players.front().id,
        .activePlayer = players.front().id,
        .phase = {PHASE_MAIN},
    };
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

    if (turnInfo.phase.back() == PHASE_MAIN 
            or turnInfo.phase.back() == PHASE_RESOLVE_STACK) {
        auto player = getPlayerById(turnInfo.activePlayer);
        for (auto& card : player->hand) {
            Action action = {
                .type = ACTION_PLAY_CARD,
                .playerId = turnInfo.activePlayer,
                .id = card.id,
                .description = card.name,
            };
            actions.push_back(action);
        }
    }

    return actions;
}

vector<Action> GameState::getValidBeaconActions()
{
    // TODO more complicated logic
    vector<Action> actions;
    return actions;
}

vector<Action> GameState::getTargetActions()
{
    Action action;
    vector<Target> targets;
    vector<Action> actions;
    if (turnInfo.phase.back() == PHASE_SELECT_CARD_TARGETS) {
        auto card = stack.back();
        auto p = card.getValidTargets(*this);
        auto nTargets = p.first;
        auto possibleTargets = p.second;

        Action selectShipsAction = {
            .type = ACTION_SELECT_SHIPS,
            .playerId = card.playedBy,
            .id = card.id,
            .nTargets = nTargets,
            .description = "Select targets from list for card: " + card.name,
        };

        Action selectSystemsAction = {
            .type = ACTION_SELECT_SYSTEM,
            .playerId = card.playedBy,
            .id = card.id,
            .nTargets = nTargets,
            .description = "Select targets from list for card: " + card.name,
        };

        for (auto t : possibleTargets) {
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

    Action endTurn =
    {
        .type = ACTION_END_TURN,
        .playerId = turnInfo.activePlayer,
    };

    // Cannot do nothing if a target selection is needed
    if (turnInfo.phase.back() != PHASE_SELECT_CARD_TARGETS 
            and turnInfo.phase.back() != PHASE_SELECT_BEACON_TARGETS) {
        actions.push_back(pass);
    }

    // Can only end the turn if it is the main phase
    if (turnInfo.phase.back() == PHASE_MAIN and turnInfo.activePlayer == turnInfo.whoseTurn) {
        actions.push_back(endTurn);
    }

    auto cardActions = getValidCardActions();
    auto moveActions = getValidBeaconActions();
    auto targetActions = getTargetActions();

    actions.insert(actions.end(), cardActions.begin(), cardActions.end());
    actions.insert(actions.end(), moveActions.begin(), moveActions.end());
    actions.insert(actions.end(), targetActions.begin(), targetActions.end());

    if (playerId) {
        vector<Action> actionsForPlayer;
        copy_if(actions.begin(), actions.end(), back_inserter(actionsForPlayer),
                [playerId](Action a) {return a.playerId == playerId;});

        return actionsForPlayer;
    } else {
        return actions;
    }
}


void GameState::performAction(Action action)
{
    switch (turnInfo.phase.back()) {
        case PHASE_UPKEEP: {
            // TODO handle fast actions
            LOG_INFO << "End upkeep, start main phase";
            auto drawInfo = getPlayerById(turnInfo.whoseTurn)->draw();
            changes.push_back({.type = CHANGE_DRAW_CARD, .data = drawInfo});
            drawInfo = {};
            turnInfo.phase[0] = PHASE_MAIN;
            changes.push_back({.type = CHANGE_PHASE_CHANGE, .data = turnInfo});
            break;
        }
        case PHASE_MAIN:
            switch (action.type) {
                case ACTION_PLAY_CARD:
                    playCard(action.id, turnInfo.activePlayer); 
                    // May change phase to a target select phase
                    break;
                case ACTION_PLACE_BEACON:
                    placeBeacon(action.id, turnInfo.activePlayer);
                    break;
                case ACTION_NONE: 
                case ACTION_END_TURN:
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
            break;
        case PHASE_RESOLVE_STACK:
            switch(action.type) {
                case ACTION_PLAY_CARD:
                    playCard(action.id, turnInfo.activePlayer);
                    break;
                case ACTION_NONE:
                    resolveStackTop();
                    break;
                default:
                    LOG_ERROR << "Invalid action type when resolving stack";
            }
            break;
        case PHASE_SELECT_CARD_TARGETS:
            stack.back().targets = action.targets;
            turnInfo.phase.pop_back();
            break;
        case PHASE_SELECT_BEACON_TARGETS:
            break;
    }
}

void GameState::playCard(int cardId, int playerId)
{
    auto player = getPlayerById(playerId);
    auto card = find_if(player->hand.begin(), player->hand.end(), 
            [cardId] (Card& c) {return c.id == cardId;});
    card->playedBy = playerId;

    if (turnInfo.phase.back() != PHASE_RESOLVE_STACK) {
        turnInfo.phase.push_back(PHASE_RESOLVE_STACK);
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
    turnInfo.phase[0] = PHASE_MOVE;
    turnInfo.phase.push_back(PHASE_SELECT_CARD_TARGETS);
    changes.push_back({.type = CHANGE_PLACE_BEACON, .data = beacon});
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
        case ACTION_END_TURN:
            out << "end turn"; break;
        case ACTION_SELECT_SHIPS:
            out << "select ships"; break;
        case ACTION_SELECT_SYSTEM:
            out << "select system"; break;
    }
    if (c.description.size()) {
        out << " " << c.description;
    }
    if (c.targets.size()) {
        out << " pick " << c.nTargets << " targets: " << c.targets;
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

