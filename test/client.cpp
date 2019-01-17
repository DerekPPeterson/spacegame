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
    REQUIRE(actions.front().type == ACTION_NONE);

    int p1Id = state.turnInfo.whoseTurn;

    // Test ending turn
    client.performAction(actions.front());
    usleep(2e5);
    state = client.getState();

    REQUIRE(state.turnInfo.phase.back() == PHASE_END);

    actions = client.getActions();
    REQUIRE(actions.front().type == ACTION_NONE);
    client.performAction(actions.front());

    state = client.getState();
    REQUIRE(state.turnInfo.phase.back() == PHASE_UPKEEP);
    REQUIRE(state.turnInfo.whoseTurn != p1Id);
}

