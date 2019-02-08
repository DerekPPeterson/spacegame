#include "timer.h"

#include <map>
#include <exception>

using namespace std;

Timer Timer::global;

typedef chrono::duration<float> fsec;

Timer::Timer(string name)
{
    this->name = name;
    started = std::chrono::system_clock::now();
    lastChecked = started;
}

float Timer::get()
{
    auto curTime = std::chrono::system_clock::now();
    fsec deltaTime = curTime - started;
    return deltaTime.count();
}

float Timer::getDelta()
{
    auto curTime = std::chrono::system_clock::now();
    fsec deltaTime = curTime - lastChecked;
    lastChecked = curTime;
    return deltaTime.count();
}

