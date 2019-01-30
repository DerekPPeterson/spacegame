#include "client.h"

#include <plog/Log.h>

#include "timer.h"


using namespace std;
using namespace logic;
using namespace curlpp::options;

GameClient::GameClient(string serverAddr, int serverPort) 
    : serverAddr(serverAddr), serverPort(serverPort)
{ 
}

void GameClient::login(string username)
{
    this->username = username;
    string path = serverAddr + "/player/" + username + "/login";
    loginToken = makeRequest(path, username);
}

void GameClient::startGame()
{
    auto path = serverAddr + "/createGame";
    gameId = makeRequest(path, "");
    LOG_INFO << "Created game (id: " << gameId << ")";
}

void GameClient::joinGame(string gameId)
{
    auto path = serverAddr + "/game/" + gameId + "/join";
    this->gameId = makeRequest(path, "");
    LOG_INFO << "Joined game (id: " << gameId << ")";
};

string GameClient::makeRequest(string path, string data) const
{
    if (loginToken == "") {
        LOG_ERROR << "Attempting to make request without logging in";
    };
    RequestData request = {
        .loginToken = loginToken,
        .serializedData = data,
    };
    stringstream ss;
    {
        cereal::PortableBinaryOutputArchive oarchive(ss);
        oarchive(request);
    }

    stringstream os;
    try {
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
    } catch (curlpp::LogicError & e) {
		LOG_ERROR << e.what() << std::endl;
	} catch (curlpp::RuntimeError & e) {
		LOG_ERROR << e.what() << std::endl;
    }

    return os.str();
};

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
    string path = serverAddr + "/game/" + gameId + "/getactions";
    return getObject<vector<Action>>(path);
}

GameState GameClient::getState()
{
    string path = serverAddr + "/game/" + gameId + "/state";
    return getObject<GameState>(path);
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
        changesLastRequest = 0; // reset timer to submit a request immediatly
        auto changes = futureChanges->get();
        if (changes.size()) {
            LOG_INFO << "Got " << changes.size() << "changes from server, biggest changeNo: " << changes.back().changeNo;
        }

        futureChanges.reset();
        return changes;
    } else {
        return {};
    }
}

vector<logic::Change> GameClient::_getChangesSince(int changeNo) const
{
    string path = serverAddr + "/game/" + gameId + "/changes/" + to_string(changeNo);
    return getObject<vector<Change>>(path);
}

void GameClient::performAction(Action action)
{
    actionPending = async(launch::async, &GameClient::_performAction, this, action);
    changesLastRequest = 0; // Start requesting changes right after performing action
}

void GameClient::_performAction(Action action) const
{
    stringstream ss;
    {
        cereal::PortableBinaryOutputArchive oarchive(ss);
        oarchive(action);
    }

    stringstream os;

    string path = serverAddr + "/game/" + gameId + "/performaction";
    makeRequest(path, ss.str());
}



