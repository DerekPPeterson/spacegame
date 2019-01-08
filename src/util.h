#ifndef UTIL_H
#define UTIL_H

#include <random>
#include <map>
#include <string>

inline float rand_float_between(float LO, float HI)
{
    return LO + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(HI-LO)));
}

inline std::string randString(int n)
{
    std::string charset = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    std::string ret;
    for (int i = 0; i < n; i++) {
        ret.push_back(charset[rand() % charset.size()]);
    }
    return ret;
};

#endif
