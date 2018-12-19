#ifndef UTIL_H
#define UTIL_H

#include <random>
#include <map>

inline float rand_float_between(float LO, float HI)
{
    return LO + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(HI-LO)));
}

template <class T1, class T2>
class bimap
{
    public:

    protected:
        std::map<T1, T2> m1;
        std::map<T2, T1> m2;
};

#endif
