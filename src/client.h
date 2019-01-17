#ifndef CLIENT_H
#define CLIENT_H

#include <memory>
#include <vector>
#include <future>
#include <optional>

#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>

#include "logic.h"
#include "timer.h"


class GameClient
{
    public:
        GameClient(std::string serverAddr, int port);
        void startGame();
        void joinGame();
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
        std::vector<logic::Action> _getActions() const;
        std::optional<std::future<std::vector<logic::Action>>> futureActions;

        void _performAction(logic::Action action) const;
        std::optional<std::future<void>> actionPending;

        float changesLastRequest = 0;
        std::vector<logic::Change> _getChangesSince(int changeNo) const;
        std::optional<std::future<std::vector<logic::Change>>> futureChanges;

        Timer timer;
};

#endif

