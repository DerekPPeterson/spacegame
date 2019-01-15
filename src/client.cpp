#include "client.h"

#include <sstream>
#include <plog/Log.h>


using namespace std;
using namespace curlpp::options;

GameClient::GameClient(string serverAddr, int serverPort) 
    : serverAddr(serverAddr), serverPort(serverPort)
{ 
}

void GameClient::startGame()
{
    {
        curlpp::Cleanup cleanup;

        curlpp::Easy request;
        request.setOpt(Url(serverAddr + "/createGame"));
        request.setOpt(Port(serverPort));

        stringstream ss;
        ss << request;
        gameId = ss.str();
    }
    LOG_INFO << "Created game (id: " << gameId << ")";
}

GameState GameClient::getState()
{
    stringstream ss;
    {
        curlpp::Cleanup cleanup;

        curlpp::Easy request;
        request.setOpt(Url(serverAddr + "/game/" + gameId + "/state"));
        request.setOpt(Port(serverPort));

        ss << request;
    }

    GameState state;
    {
        cereal::BinaryInputArchive iarchive(ss);
        iarchive(state);
    }
    return state;
}
