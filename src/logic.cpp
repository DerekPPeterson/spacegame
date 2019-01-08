#include "logic.h"

#include <fstream>
#include <algorithm>

using namespace logic;
using namespace std;


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
                        adjacentPos[1] < 0 or adjacentPos[0] > n) {
                    continue;
                } else {
                    systems[n * i + j].adjacent.push_back(
                            systems[n * adjacentPos[0] + adjacentPos[1]].id);
                }
            }
        }
    }

    return list<System> {begin(systems), end(systems)};
}

void GameState::startGame()
{
    LOG_INFO << "Setting up game";

    LOG_INFO << "Initilializing systems";
    systems = createSystems(4);

    LOG_INFO << "Creating player objects";
    for (int i = 0; i < 2; i++) {
        // Create player objects

        // TODO load deck dynamically
        list<logic::Card> deck;
        for (int i = 0; i < 40; i++) {
            deck.push_back({.name="Card " + to_string(i), .cost={{RESOURCE_AI, 1}}});
        }

        Player p = {
            .name = "Player" + to_string(i),
            .deck = deck,
            .resources = {{RESOURCE_WARP_BEACONS, {.amount = 0, .max=1, .perTurn=1}}},
        };
        p.draw(7);
        players.push_back(p);
        LOG_INFO << "Adding player " << p.name;

        // Give them flagships TODO load chosen flagshipss
        LOG_INFO << "Added flagship for " << p.name;
        logic::Ship sampleFlagship = {
            .type = "Sample Flagship",
            .attack = 1,
            .shield = 10,
            .armour = 10,
            .movement = 1,
            .owner = i,
            .controller = i
        };
        ships.push_back(sampleFlagship);
        p.flagshipId = sampleFlagship.id;
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

vector<Action> GameState::getValidCardActions()
{
    // TODO more complicated logic
    vector<Action> actions;

    if (turnInfo.phase.back() == PHASE_MAIN 
            or turnInfo.phase.back() == PHASE_RESOLVE_STACK) {
        auto player = getPlayerById(turnInfo.whoseTurn);
        for (auto& card : player->hand) {
            Action action = {
                .type = ACTION_PLAY_CARD,
                .playerId = turnInfo.whoseTurn,
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

vector<Action> GameState::getPossibleActions()
{
    vector<Action> actions;

    auto cardActions = getValidCardActions();
    auto moveActions = getValidBeaconActions();

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

    actions.push_back(pass);
    if (turnInfo.phase.back() == PHASE_MAIN and turnInfo.activePlayer == turnInfo.whoseTurn) {
        actions.push_back(endTurn);
    }
    actions.insert(actions.end(), cardActions.begin(), cardActions.end());
    actions.insert(actions.end(), moveActions.begin(), moveActions.end());

    return actions;
}


void GameState::performAction(Action action)
{
    switch (turnInfo.phase.back()) {
        case PHASE_UPKEEP:
            // TODO handle fast actions
            LOG_INFO << "End upkeep, start main phase";
            turnInfo.phase[0] = PHASE_MAIN;
            break;
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
                    break;
                case ACTION_SELECT_SHIPS:
                case ACTION_SELECT_SYSTEM:
                    LOG_ERROR << "Cannot select ships / system from main phase";
                    break;
                }
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
            break;
        case PHASE_SELECT_BEACON_TARGETS:
            break;
    }
}

void GameState::playCard(int cardId, int playerId)
{
    auto player = getPlayerById(playerId);
    auto it = find_if(player->hand.begin(), player->hand.end(), 
            [cardId] (Card& c) {return c.id == cardId;});
    it->playedBy = playerId;
    stack.push_back(*it);
    player->hand.erase(it);

    if (turnInfo.phase.back() != PHASE_RESOLVE_STACK) {
        turnInfo.phase.push_back(PHASE_RESOLVE_STACK);
    }
}

void GameState::resolveStackTop()
{
    auto card = stack.back();
    stack.pop_back();
    card.resolve(*this);
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

    LOG_INFO << "It is now player id " << nextPlayerId << "'s turn" << endl;
    turnInfo.phase[0] = PHASE_UPKEEP;
};

void Player::draw(int n)
{
    for (int i = 0; i < n; i++) {
        if (not deck.size()) {
            // TODO handle drawing from empty deck
            // probably instant loss
            break;
        };
        auto card = deck.back();
        deck.pop_back();
        hand.push_back(card);
        LOG_INFO << "Player " << name << " drew a card: " << card.name;
    }
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
            out << "play card"; 
            break;
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



//int main()
//{
//    static plog::ColorConsoleAppender<plog::TxtFormatter> consoleAppender;
//    plog::init(plog::verbose, &consoleAppender);
//
//    GameState state;
//    state.startGame();
//    cerr << state;
//    state.writeStateToFile("testState.json");
//    cerr << state;
//
//    while (1) {
//        auto actions = state.getPossibleActions();
//        int i = 0;
//        for (auto a : actions) {
//            cout << i++ << ": " << a << endl;
//        }
//        int selection;
//        cin >> selection;
//        state.performAction(actions[selection]);
//        cerr << state;
//    }
//}
