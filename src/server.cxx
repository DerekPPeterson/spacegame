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
				.threads(thr);
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
			Routes::Get(router, "/game/:gameid/changes/:changeNo", Routes::bind(&GameEndpoint::getChangesSince, this));
		}

         void createGame(const Rest::Request& request, Http::ResponseWriter response) {
             string gameId = randString(8);
             GameState newGameState;
             newGameState.startGame();
             gamestates[gameId] = newGameState;
             LOG_INFO << "Create new game with id: " << gameId;
             response.send(Http::Code::Ok, gameId);
         }

         void getState(const Rest::Request& request, Http::ResponseWriter response) {
            auto gameId = request.param(":gameid").as<string>();
            LOG_INFO << "Got request for state for game id: " << gameId;
            stringstream ss;
            {
                cereal::PortableBinaryOutputArchive oarchive(ss);
                oarchive(gamestates[gameId]);
            }
            response.send(Http::Code::Ok, ss.str());
         }

         void getActions(const Rest::Request& request, Http::ResponseWriter response) {
            auto gameId = request.param(":gameid").as<string>();
            LOG_INFO << "Got request for actions for game id: " << gameId;

            vector<Action> actions = gamestates[gameId].getPossibleActions();
            stringstream ss;
            {
                cereal::PortableBinaryOutputArchive oarchive(ss);
                oarchive(actions);
            }
            response.send(Http::Code::Ok, ss.str());
         }

         void performAction(const Rest::Request& request, Http::ResponseWriter response) {
            auto gameId = request.param(":gameid").as<string>();
             LOG_INFO << "Got request to perform action for game id: " << gameId;
            stringstream ss;
            ss << request.body();
            Action action;
            {
                cereal::PortableBinaryInputArchive iarchive(ss);
                iarchive(action);
            }
            gamestates[gameId].performAction(action);
            response.send(Http::Code::Ok, "");
         }

         void getChangesSince(const Rest::Request& request, Http::ResponseWriter response) {
            auto gameId = request.param(":gameid").as<string>();
            auto changeNo = request.param(":changeNo").as<int>();
            LOG_INFO << "Got request for changes for game id: " << gameId 
                << " since changeNo: " << changeNo;

            // TODO actually implement this
            stringstream ss;
            {
                cereal::PortableBinaryOutputArchive oarchive(ss);
                oarchive(gamestates[gameId].getChangesAfter(changeNo));
            }
            response.send(Http::Code::Ok, ss.str());
         }

	    std::shared_ptr<Http::Endpoint> httpEndpoint;
		Rest::Router router;

        map<string, GameState> gamestates;
};

int main() {
    remove("server.log");
    plog::init(plog::verbose, "server.log");

    Port port(40000);

    int thr = 2;

    Address addr(Ipv4::any(), port);

    GameEndpoint games(addr);
    games.init(thr);
    games.start();
    
    games.shutdown();
}
