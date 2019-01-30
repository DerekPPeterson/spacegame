#include "catch.hpp"

#include "logic.h"
#include "client.h"
#include "timer.h"

#include <subprocess.hpp>

#include <string>

using namespace logic;
using namespace std;
using namespace subprocess;

class GameClientTester : public GameClient
{
    public:
        GameClientTester(string serverAddr, int port) 
            : GameClient(serverAddr, port) {};

        string getGameId() {return gameId;};
        string getLoginToken() {return loginToken;};
};

class LocalServerStarter
{
    public:
        LocalServerStarter() {
            p = new Popen({"./server"});
            usleep(1e5);
        }
        ~LocalServerStarter() {
            p->kill();
            delete p;
        }
    private:
        Popen *p;

};

TEST_CASE("Basic Game Client Tests", "[GameClient]") {
    
    // Start server in background
    LocalServerStarter server;
    
    GameClientTester client("localhost", 40000);
    client.login("AckbarsRevenge");
    REQUIRE(client.isLoggedIn());
    client.startGame();
    string gameId = client.getGameId();

    REQUIRE(gameId.size() == 8);

    auto state = client.getState();

    // Some simple sanity checks
    REQUIRE(state.players.size() == 2);
    REQUIRE(state.players.front().flagshipId != 0);
    REQUIRE(state.turnInfo.phase.back() == PHASE_MAIN);

    // Test getting actions
    auto actions = client.getActions();
    while (not actions.size()) {
        actions = client.getActions();
    }
    REQUIRE(actions.size() != 0);
    auto t = actions.front().type;
    REQUIRE(t == ACTION_NONE);

    int p1Id = state.turnInfo.whoseTurn;

    // Test ending turn
    client.performAction(actions.front());
    usleep(2e5);
    state = client.getState();

    REQUIRE(state.turnInfo.phase.back() == PHASE_END);

    actions = client.getActions();
    while (not actions.size()) {
        actions = client.getActions();
    }
    REQUIRE(actions.front().type == ACTION_NONE);
    client.performAction(actions.front());
    usleep(1e5);

    state = client.getState();
    REQUIRE(state.turnInfo.phase.back() == PHASE_UPKEEP);
    REQUIRE(state.turnInfo.whoseTurn != p1Id);

    // Test getting changelist
    auto changes = client.getChangesSince(0);
    while (not changes.size()) {
        changes = client.getChangesSince(0);
    }
    REQUIRE(changes.size() > 0);
}

TEST_CASE("Multiple Player Tests", "[GameClient]")
{
    LocalServerStarter server;

    GameClientTester client1("localhost", 40000);
    client1.login("player1");
    client1.startGame();

    GameClientTester client2("localhost", 40000);
    client2.login("player2");
    client2.joinGame(client1.getGameId());

    REQUIRE(client1.getLoginToken() != client2.getLoginToken());

    REQUIRE(client1.getGameId() == client2.getGameId());

    auto state = client1.getState();
    auto client1LogicId = state.players.front().id;
    auto client2LogicId = state.players.back().id;

    auto actions = client1.getActions();
    while (not actions.size()) {
        actions = client1.getActions();
    }

    cout << actions << endl;
    REQUIRE(actions.size() > 0);
    for (auto a : actions) {
        REQUIRE(client1LogicId == a.playerId);
    }

    actions = client2.getActions();
    usleep(1e6);
    actions = client2.getActions();
    cout << actions << endl;
    for (auto a : actions) {
        REQUIRE(client2LogicId == a.playerId);
    }
}

