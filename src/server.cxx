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

#include "client.h"

using namespace std;
using namespace Pistache;
using namespace logic;

struct User
{
    string username;
    string loginToken;
    int playerId;
};

struct ActiveGame
{
    GameState state;
    vector<User> players;
};

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

			Routes::Post(router, "/createGame", Routes::bind(&GameEndpoint::createGame, this));
			Routes::Post(router, "/player/:username/login", Routes::bind(&GameEndpoint::getLoginToken, this));
			Routes::Post(router, "/game/:gameid/join", Routes::bind(&GameEndpoint::joinGame, this));
			Routes::Post(router, "/game/:gameid/state", Routes::bind(&GameEndpoint::getState, this));
			Routes::Post(router, "/game/:gameid/getactions", Routes::bind(&GameEndpoint::getActions, this));
			Routes::Post(router, "/game/:gameid/performaction", Routes::bind(&GameEndpoint::performAction, this));
			Routes::Post(router, "/game/:gameid/changes/:changeNo", Routes::bind(&GameEndpoint::getChangesSince, this));
		}

        pair<User&, string> getRequestData(string requestBody)
        {
            stringstream ss;
            ss << requestBody;

            RequestData requestData;
            {
                cereal::PortableBinaryInputArchive iarchive(ss);
                iarchive(requestData);
            }
            // TODO handle incorrect token
            auto& user = users[requestData.loginToken];
            return {user, requestData.serializedData};
        };

        void createGame(const Rest::Request& request, Http::ResponseWriter response) {
            auto r = getRequestData(request.body());
            auto& user = r.first;

            string gameId = randString(8);
            ActiveGame newGame;
            newGame.state.startGame();
            user.playerId = newGame.state.players.front().id;
            newGame.players.push_back(user);

            games[gameId] = newGame;
            LOG_INFO << "Create new game with id: " << gameId;
            response.send(Http::Code::Ok, gameId);
         }

        void joinGame(const Rest::Request& request, Http::ResponseWriter response) {
            auto r = getRequestData(request.body());
            auto& user = r.first;
             auto gameId = request.param(":gameid").as<string>();

            auto& game = games[gameId];
            user.playerId = game.state.players.back().id;
            game.players.push_back(user);

            LOG_INFO << "Player " << user.username << " joined game with id: " << gameId;
            response.send(Http::Code::Ok, gameId);
         }

         void getLoginToken(const Rest::Request& request, Http::ResponseWriter response) {
             // TODO some kind of auth check not currently logged in
             auto username = request.param(":username").as<string>();
             string playerToken = randString(8);
             LOG_INFO << "Player " << username << " logged in";
             User user = {
                 .username = username,
                 .loginToken = playerToken,
             };
             users[playerToken] = user;
             response.send(Http::Code::Ok, playerToken);
         }

         void getState(const Rest::Request& request, Http::ResponseWriter response) {
            auto gameId = request.param(":gameid").as<string>();
            LOG_INFO << "Got request for state for game id: " << gameId;
            stringstream ss;
            {
                cereal::PortableBinaryOutputArchive oarchive(ss);
                oarchive(games[gameId].state);
            }
            response.send(Http::Code::Ok, ss.str());
         }

         void getActions(const Rest::Request& request, Http::ResponseWriter response) {
            auto r = getRequestData(request.body());
            auto user = r.first;
            auto gameId = request.param(":gameid").as<string>();
            LOG_INFO << "Got request for actions for game id: " << gameId << " from user: " << user.username;

            vector<Action> actions = games[gameId].state.getPossibleActions(user.playerId);
            stringstream ss;
            {
                cereal::PortableBinaryOutputArchive oarchive(ss);
                oarchive(actions);
            }
            response.send(Http::Code::Ok, ss.str());
         }

         void performAction(const Rest::Request& request, Http::ResponseWriter response) {
            auto r = getRequestData(request.body());
            auto user = r.first;
            string serializedData = r.second;
            auto gameId = request.param(":gameid").as<string>();
             LOG_INFO << "Got request to perform action for game id: " << gameId;
            stringstream ss;
            ss << serializedData;
            Action action;
            {
                cereal::PortableBinaryInputArchive iarchive(ss);
                iarchive(action);
            }
            games[gameId].state.performAction(action);
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
                oarchive(games[gameId].state.getChangesAfter(changeNo));
            }
            response.send(Http::Code::Ok, ss.str());
         }

	    std::shared_ptr<Http::Endpoint> httpEndpoint;
		Rest::Router router;

        map<string, ActiveGame> games;
        map<string, User> users;
};

int main() {
    remove("server.log");
    plog::init(plog::verbose, "server.log");
    LOG_INFO << "Starting server";

    Port port(40000);

    int thr = 2;

    Address addr(Ipv4::any(), port);

    GameEndpoint games(addr);
    games.init(thr);
    games.start();
    
    games.shutdown();
}
