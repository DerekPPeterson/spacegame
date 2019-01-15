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

template <class T> 
T getObject(string path, int port)
{
    stringstream ss;
    {
        curlpp::Cleanup cleanup;

        curlpp::Easy request;
        request.setOpt(Url(path));
        request.setOpt(Port(port));

        ss << request;
    }

    T obj;
    {
        cereal::BinaryInputArchive iarchive(ss);
        iarchive(obj);
    }
    return obj;
}


vector<Action> GameClient::getActions()
{
    string path = serverAddr + "/game/" + gameId + "/action";
    return getObject<vector<Action>>(path, serverPort);
}

GameState GameClient::getState()
{
    string path = serverAddr + "/game/" + gameId + "/state";
    return getObject<GameState>(path, serverPort);
}

void GameClient::performAction(Action action)
{
    stringstream ss;
    {
        cereal::BinaryOutputArchive oarchive(ss);
        oarchive(action);
    }

    stringstream os;

    string path = serverAddr + "/game/" + gameId + "/action";
    {
        curlpp::Cleanup cleanup;
        curlpp::Easy request;

        char buf[50];
		std::list<std::string> headers;
		headers.push_back("Content-Type: application/octet-stream"); 
		sprintf(buf, "Content-Length: %d", (int) ss.str().size()); 
		headers.push_back(buf);

        request.setOpt(Url(path));
        request.setOpt(Port(serverPort));
        request.setOpt(PostFields(ss.str()));
        request.setOpt(PostFieldSize(ss.str().size()));
        request.setOpt(Timeout(1));
        os << request;
    }
}



