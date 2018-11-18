#ifndef TIMER_H
#define TIMER_H

#include <string>

class Timer
{
    public:
    // Start a new timer counting from 0
    static void create(std::string name);
    // Delete a timer
    static void remove(std::string name);
    // Get the time since the timer was started
    static float get(std::string name);
    // Get the time since the timer was either started or checked w/ this func
    static float getDelta(std::string name);
};

#endif
