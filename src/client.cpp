#include "client.h"

#include <sstream>
#include <plog/Log.h>

#include "timer.h"


using namespace std;
using namespace logic;
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
    LOG_DEBUG << "Making server request to: " << path << " port: " << port;
    stringstream ss;
    try {
        curlpp::Cleanup cleanup;

        curlpp::Easy request;
        request.setOpt(Url(path));
        request.setOpt(Port(port));

        ss << request;
    } catch (curlpp::LogicError & e) {
		LOG_ERROR << e.what() << std::endl;
	} catch (curlpp::RuntimeError & e) {
		LOG_ERROR << e.what() << std::endl;
    }

    T obj;
    {
        cereal::PortableBinaryInputArchive iarchive(ss);
        iarchive(obj);
    }
    return obj;
}

vector<Action> GameClient::getActions()
{
    if (timer.get() - actionsLastRequest < rateLimit) {
        return {};
    }
    if (not futureActions)
    {
        actionsLastRequest = timer.get();
        futureActions = async(launch::async, &GameClient::_getActions, this);
        return {};
    } else if (futureActions->wait_for(chrono::seconds(0)) == future_status::ready) {
        auto actions = futureActions->get();
        futureActions.reset();
        return actions;
    } else {
        return {};
    }
}

vector<Action> GameClient::_getActions() const
{
    if (actionPending) {
        actionPending->wait();
    }
    string path = serverAddr + "/game/" + gameId + "/action";
    return getObject<vector<Action>>(path, serverPort);
}

GameState GameClient::getState()
{
    string path = serverAddr + "/game/" + gameId + "/state";
    return getObject<GameState>(path, serverPort);
}

vector<logic::Change> GameClient::getChangesSince(int changeNo)
{
    if (timer.get() - changesLastRequest < rateLimit) {
        return {};
    }
    if (not futureChanges)
    {
        changesLastRequest = timer.get();
        futureChanges = async(launch::async, &GameClient::_getChangesSince, this, changeNo);
        return {};
    } else if (futureChanges->wait_for(chrono::seconds(0)) == future_status::ready) {
        auto changes = futureChanges->get();
        futureChanges.reset();
        return changes;
    } else {
        return {};
    }
}

vector<logic::Change> GameClient::_getChangesSince(int changeNo) const
{
    string path = serverAddr + "/game/" + gameId + "/changes/" + to_string(changeNo);
    return getObject<vector<Change>>(path, serverPort);
}

void GameClient::performAction(Action action)
{
    actionPending = async(launch::async, &GameClient::_performAction, this, action);
}

void GameClient::_performAction(Action action) const
{
    stringstream ss;
    {
        cereal::PortableBinaryOutputArchive oarchive(ss);
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



