#ifndef TIMER_H
#define TIMER_H

#include <string>
#include <chrono>

class Timer
{
    public:
        // Start a new timer counting from 0
        Timer(std::string name = "");
        // Get the time since the timer was started
        float get();
        // Get the time since the timer was either started or checked w/ this func
        float getDelta();

        std::string name;
        std::chrono::time_point<std::chrono::system_clock> started;
         std::chrono::time_point<std::chrono::system_clock> lastChecked;

        static Timer global;
};

#endif
