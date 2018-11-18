#include "timer.h"

#include <map>
#include <GLFW/glfw3.h>
#include <exception>

using namespace std;

struct TimerData {
    string name;
    float started;
    float lastChecked;
};

map<string, TimerData> TIMERS;

void Timer::create(string name)
{
    TimerData newTimer;
    newTimer.name = name;
    newTimer.started = glfwGetTime();
    newTimer.lastChecked = newTimer.started;

    if (TIMERS.find(name) == TIMERS.end()) {
        TIMERS[name] = newTimer;
    } else {
        throw runtime_error("Timer already exists with name: " + name);
    }
}

void Timer::remove(string name)
{
    TIMERS.erase(name);
}

float Timer::get(string name)
{
    float curTime = glfwGetTime();
    float deltaTime = curTime - TIMERS[name].started;
    return deltaTime;
}

float Timer::getDelta(string name)
{
    float curTime = glfwGetTime();
    float deltaTime = curTime - TIMERS[name].lastChecked;
    TIMERS[name].lastChecked = curTime;
    return deltaTime;
}

