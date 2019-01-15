#include "catch.hpp"

#include "logic.h"
#include "client.h"

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
        bool isRunning() {
            return p->poll() == 0;
        }
    private:
        Popen *p;

};

TEST_CASE("Game client start", "[GameClient]") {
    
    // Start server in background
    LocalServerStarter server;
    
    GameClientTester client("localhost", 40000);
    client.startGame();
    string gameId = client.getGameId();

    REQUIRE(gameId.size() == 8);

    auto state = client.getState();

    REQUIRE(state.players.size() == 2);
    REQUIRE(state.players.front().flagshipId != 0);
}

