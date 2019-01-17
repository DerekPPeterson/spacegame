#include "timer.h"

#include <map>
#include <GLFW/glfw3.h>
#include <exception>

using namespace std;

Timer Timer::global;

Timer::Timer(string name)
{
    this->name = name;
    started = std::chrono::system_clock::now();
    lastChecked = started;
}

float Timer::get()
{
    auto curTime = std::chrono::system_clock::now();
    auto deltaTime = curTime - started;
    return deltaTime.count();
}

float Timer::getDelta()
{
    auto curTime = std::chrono::system_clock::now();
    auto deltaTime = curTime - started;
    lastChecked = curTime;
    return deltaTime.count();
}

