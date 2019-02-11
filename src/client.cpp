#include "client.h"

#include <plog/Log.h>

#include "timer.h"



using namespace std;
using namespace logic;
using namespace curlpp::options;

GameClient::GameClient(string serverAddr, int serverPort) 
    : serverAddr(serverAddr), serverPort(serverPort)
{ 
    RequestThreadData* dataPointer;

    dataPointer = &getActionsData;
    getActionsThread = thread([this, dataPointer]{requestThreadFunc(dataPointer);});

    dataPointer = &performActionsData;
    performActionsThread = thread([this, dataPointer]{requestThreadFunc(dataPointer);});

    dataPointer = &getChangesData;
    getChangesThread = thread([this, dataPointer]{requestThreadFunc(dataPointer);});
}

void endThread(RequestThreadData& threadData, thread& thread)
{
    {
        lock_guard<mutex> lk(threadData.m);
        threadData.stop = true;
    }
    bool done = false;
    while (not done) {
        done = threadData.done;
        this_thread::yield();
        threadData.start.notify_all();
    }
    thread.join();
}

GameClient::~GameClient()
{
    endThread(getActionsData, getActionsThread);
    endThread(performActionsData, performActionsThread);
    endThread(getChangesData, getChangesThread);
}

void GameClient::login(string username)
{
    this->username = username;
    string path = serverAddr + "/player/" + username + "/login";
    LOG_INFO << "Logging in as " << username;
    loginToken = makeRequest(path, username);
    if (loginToken.size()) {
        LOG_INFO << "Successfully logged in, token: " << loginToken;
    }
}

void GameClient::startGame()
{
    auto path = serverAddr + "/createGame";
    auto data = makeRequest(path, "");
    stringstream ss;
    ss << data;
    pair<string, int> ret;
    {
        cereal::PortableBinaryInputArchive iarchive(ss);
        iarchive(ret);
    }
    gameId = ret.first;
    playerId = ret.second;
    LOG_INFO << "Created game (id: " << gameId << ")";
}

void GameClient::joinGame(string gameId)
{
    auto path = serverAddr + "/game/" + gameId + "/join";
    auto data = makeRequest(path, "");
    stringstream ss;
    ss << data;
    pair<string, int> ret;
    {
        cereal::PortableBinaryInputArchive iarchive(ss);
        iarchive(ret);
    }
    this->gameId = ret.first;
    playerId = ret.second;
    LOG_INFO << "Joined game (id: " << gameId << ")";
};

void GameClient::joinUser(string username)
{
    auto path = serverAddr + "/player/" + username + "/join";
    auto data = makeRequest(path, "");
    stringstream ss;
    ss << data;
    pair<string, int> ret;
    {
        cereal::PortableBinaryInputArchive iarchive(ss);
        iarchive(ret);
    }
    this->gameId = ret.first;
    playerId = ret.second;
    LOG_INFO << "Joined game (id: " << gameId << ")";
};

string GameClient::makeRequest(string path, string data) const
{
    if (loginToken == "" and path.substr(path.size() - 6, 6) != "/login") {
        LOG_ERROR << "Attempting to make request without logging in";
    };

    LOG_INFO << "Making request using token: " << loginToken << " to: " << path;
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
    long responseCode = 0;
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
        request.setOpt(WriteStream(&os));
        //request.setOpt(Verbose(1));

        request.perform();
        curlpp::infos::ResponseCode::get(request, responseCode);
    } catch (curlpp::LogicError & e) {
		LOG_ERROR << e.what() << std::endl;
	} catch (curlpp::RuntimeError & e) {
		LOG_ERROR << e.what() << std::endl;
    }

    if (responseCode != 200) {
        LOG_ERROR << "Server did not return OK: " << responseCode;
    }

    return os.str();
};

template <class T>
string serialize(T obj)
{
    stringstream ss;
    {
        cereal::PortableBinaryOutputArchive oarchive(ss);
        oarchive(obj);
    }
    return ss.str();
}

template <class T> 
T deserialize(std::string data)
{
    stringstream ss;
    ss << data;
    T obj;
    {
        cereal::PortableBinaryInputArchive iarchive(ss);
        iarchive(obj);
    }
    return obj;
}

vector<Action> GameClient::getActions()
{
    // Check if there is a response ready from a previous request
    // return it if so
    optional<string> response = getResponseFromThread(&getActionsData);
    if (response) {
        return deserialize<vector<Action>>(*response);
    }

    // Otherwise we need to make a new request, but not if we're rate limited
    if (timer.get() - actionsLastRequest < rateLimit) {
        return {};
    }

    string path = serverAddr + "/game/" + gameId + "/getactions";
    bool madeRequest = makeRequestOnThread(&getActionsData, path, "");
    if (madeRequest) {
        actionsLastRequest = timer.get();
    }
    return {};
}

GameState GameClient::getState()
{
    string path = serverAddr + "/game/" + gameId + "/state";
    return getObject<GameState>(path);
}

vector<logic::Change> GameClient::getChangesSince(int changeNo)
{
    optional<string> response = getResponseFromThread(&getChangesData);
    if (response) {
        return deserialize<vector<logic::Change>>(*response);
    }

    if (timer.get() - changesLastRequest < rateLimit) {
        return {};
    }

    string path = serverAddr + "/game/" + gameId + "/changes/" + to_string(changeNo);
    bool madeRequest = makeRequestOnThread(&getChangesData, path, "");
    if (madeRequest) {
        changesLastRequest = timer.get();
    }

    return {};
}

void GameClient::performQueuedAction()
{
    if (pendingPerformActions.size()) {
        string path = serverAddr + "/game/" + gameId + "/performaction";
        string sendData = pendingPerformActions.front();
        bool madeRequest = makeRequestOnThread(&performActionsData, path, serialize(sendData));
        if (madeRequest) {
            pendingPerformActions.pop();
        }
    }
}

void GameClient::performAction(Action action)
{
    string path = serverAddr + "/game/" + gameId + "/performaction";
    string sendData = serialize(action);
    LOG_DEBUG << deserialize<Action>(sendData);
    bool madeRequest = makeRequestOnThread(&performActionsData, path, sendData);

    if (not madeRequest) {
        pendingPerformActions.push(sendData);
    }
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

void GameClient::requestThreadFunc(RequestThreadData* data)
{
    string path;
    string sendData;

    while (true) {
        {
            unique_lock<mutex> lk(data->m);
            data->start.wait(lk);
            if (data->stop) {
                data->done = true;
                break;
            }
            path = data->path;
            sendData = data->sendData;
        }
        LOG_DEBUG << data->sendData;
        string responseData = makeRequest(path, sendData);
        {
            lock_guard<mutex> lk(data->m);
            data->sendData = "";
            data->path = "";
            data->response = responseData;
            data->pending = false;
        }
    }
}

bool GameClient::makeRequestOnThread(
        RequestThreadData* threadData, std::string path, std::string sendData)
{
    bool madeRequest = false;
    {
        lock_guard<mutex> lk(threadData->m);
        if (not threadData->pending) {
            threadData->path = path;
            threadData->sendData = sendData;
            threadData->pending = true;
            madeRequest = true;
            LOG_DEBUG << sendData;
        }
    }
    if (madeRequest) {
        threadData->start.notify_all();
    }
    return madeRequest;
}

optional<string> GameClient::getResponseFromThread(RequestThreadData* threadData)
{
    optional<string> response;
    {
        lock_guard<mutex> lk(threadData->m);
        if (not threadData->pending and threadData->response.size()) {
            response = threadData->response;
            threadData->response = "";
        }
    }
    return response;
}

