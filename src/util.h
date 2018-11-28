#ifndef UTIL_H
#define UTIL_H

#include <random>

inline float rand_float_between(float LO, float HI)
{
    return LO + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(HI-LO)));
}

#endif
