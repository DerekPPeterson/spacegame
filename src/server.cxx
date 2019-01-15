#include "logic.h"
#include "util.h"

#include "pistache/endpoint.h"
#include "pistache/router.h"
#include "pistache/endpoint.h"

#include <plog/Log.h>
#include <backward.hpp>

#include <iostream>
#include <vector>
#include <sstream>

using namespace std;
using namespace Pistache;
using namespace logic;

class GameEndpoint
{
    public:
        GameEndpoint(Address addr) 
            : httpEndpoint(std::make_shared<Http::Endpoint>(addr))
        { }

		void init(size_t thr = 2) {
			auto opts = Http::Endpoint::options()
				.threads(thr)
				.flags(Tcp::Options::InstallSignalHandler);
			httpEndpoint->init(opts);
			setupRoutes();
		}

		void start() {
			httpEndpoint->setHandler(router.handler());
			httpEndpoint->serve();
		}

		void shutdown() {
			httpEndpoint->shutdown();
		}

	private:
		void setupRoutes() {
			using namespace Rest;

			Routes::Get(router, "/createGame", Routes::bind(&GameEndpoint::createGame, this));
			Routes::Get(router, "/game/:gameid/state", Routes::bind(&GameEndpoint::getState, this));
			Routes::Get(router, "/game/:gameid/action", Routes::bind(&GameEndpoint::getActions, this));
			Routes::Post(router, "/game/:gameid/action", Routes::bind(&GameEndpoint::performAction, this));
		}

         void createGame(const Rest::Request& request, Http::ResponseWriter response) {
             string gameId = randString(8);
             GameState newGameState;
             newGameState.startGame();
             gamestates[gameId] = newGameState;
             response.send(Http::Code::Ok, gameId);
         }

         void getState(const Rest::Request& request, Http::ResponseWriter response) {
            auto gameId = request.param(":gameid").as<string>();
            stringstream ss;
            {
                cereal::BinaryOutputArchive oarchive(ss);
                oarchive(gamestates[gameId]);
            }
            response.send(Http::Code::Ok, ss.str());
         }

         void getActions(const Rest::Request& request, Http::ResponseWriter response) {
            auto gameId = request.param(":gameid").as<string>();

            vector<Action> actions = gamestates[gameId].getPossibleActions();
            stringstream ss;
            {
                cereal::BinaryOutputArchive oarchive(ss);
                oarchive(actions);
            }
            response.send(Http::Code::Ok, ss.str());
         }

         void performAction(const Rest::Request& request, Http::ResponseWriter response) {
            auto gameId = request.param(":gameid").as<string>();
            stringstream ss;
            ss << request.body();
            Action action;
            {
                cereal::BinaryInputArchive iarchive(ss);
                iarchive(action);
            }
            gamestates[gameId].performAction(action);
         }

	    std::shared_ptr<Http::Endpoint> httpEndpoint;
		Rest::Router router;

        map<string, GameState> gamestates;
};

int main() {
    Port port(40000);

    int thr = 2;

    Address addr(Ipv4::any(), port);

    GameEndpoint games(addr);
    games.init(thr);
    games.start();
    
    games.shutdown();
}
