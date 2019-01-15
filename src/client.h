#ifndef CLIENT_H
#define CLIENT_H

#include <memory>
#include <vector>

#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>

#include "logic.h"

using namespace logic;

class GameClient
{
    public:
        GameClient(std::string serverAddr, int port);
        void startGame();
        void joinGame();
        GameState getState();
        std::vector<Action> getActions();
        void performAction(Action action);

    protected:
        std::string serverAddr;
        long serverPort;

        std::string playerName;
        int playerId;

        string gameId;
};

#endif

