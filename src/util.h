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

template <class T>
class Interpolated
{
    public:
        Interpolated(T min, T max, T speed) 
            : curVal(min), min(min), max(max), speed(speed) {};
        void gotoMax() {goToMax = true;};
        void gotoMin() {goToMax = false;};
        void update(float deltaTime)
        {
            if (goToMax) {
                curVal += speed * deltaTime;
            } else {
                curVal -= speed * deltaTime;
            }
            if (curVal > max) {
                curVal = max;
            } else if (curVal < min) {
                curVal = min;
            }
        }
        bool goToMax = true;
        T curVal;
    protected:
        T min;
        T max;
        T speed;
};

#endif
