#ifndef CLIENT_H
#define CLIENT_H

#include <memory>
#include <vector>
#include <queue>
#include <future>
#include <sstream>
#include <optional>
#include <thread>

#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/Infos.hpp>

#include "logic.h"
#include "timer.h"

struct RequestData
{
    std::string loginToken;
    std::string serializedData;
    SERIALIZE(loginToken, serializedData);
};

struct RequestThreadData
{
    std::mutex m;
    bool pending = false;
    bool done = false;
    bool stop = false;
    std::string path; 
    std::string sendData;
    std::string response;
    std::condition_variable start;
};


class GameClient
{
    public:
        GameClient(std::string serverAddr, int port);
        ~GameClient();

        void login(std::string username);
        bool isLoggedIn() { return loginToken.size() > 0;};


        void startGame();
        int getMyPlayerId() {return playerId;};
        void joinGame(std::string gameId);
        void joinUser(std::string username);
        logic::GameState getState();

        /* Get needed actions from the server. 
         * This is done asynchronously - if there are actions that have already
         * been retrieved it will return those, and if there are no actions
         * then it will submit a request to the server for some and then return 
         * an empty list. Calls while a request is pending will also return 
         * empty lists
         */
        std::vector<logic::Action> getActions();

        /* Send a selected action tot he server.
         * This is done asynchonously. If this request it pending it will 
         * prevent getActions requests from being made until this request is
         * finished
         */
        void performAction(logic::Action action);

        /* Call every frame, just in case there are pending actions that
         * have not been sent to the server */
        void performQueuedAction();

        /* Get changes to state since a particular change number.
         * Asynchonous in the same way a the getActions function
         */
        std::vector<logic::Change> getChangesSince(int changeNo);

    protected:
        std::string serverAddr;
        long serverPort;

        std::string playerName;
        int playerId;

        std::string gameId;

        // Futures
        const float rateLimit = 0.5;
       
        float actionsLastRequest = 0;

        void _performAction(logic::Action action) const;
        std::optional<std::future<void>> actionPending;

        float changesLastRequest = 0;

        std::string makeRequest(std::string path, std::string data) const;

        Timer timer;

        std::string username;
        std::string loginToken;

        template <class T> 
        T getObject(std::string path) const
        {
            LOG_DEBUG << "Making server request to: " << path << " port: " << serverPort;
            auto data = makeRequest(path, "");
            std::stringstream ss;
            ss << data;

            T obj;
            {
                cereal::PortableBinaryInputArchive iarchive(ss);
                iarchive(obj);
            }
            return obj;
        }

        std::thread getActionsThread;
        RequestThreadData getActionsData;

        std::queue<std::string> pendingPerformActions;
        std::thread performActionsThread;
        RequestThreadData performActionsData;

        std::thread getChangesThread;
        RequestThreadData getChangesData;

        std::thread create(std::string path);
        void requestThreadFunc(RequestThreadData* data);
        bool makeRequestOnThread(RequestThreadData* threadData, std::string path, std::string sendData);
        std::optional<std::string> getResponseFromThread(RequestThreadData* threadData);
};

#endif

