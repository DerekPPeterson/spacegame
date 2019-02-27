#ifndef UTIL_H
#define UTIL_H

#include <random>
#include <map>
#include <string>
#include <math.h>
#include <glm/glm.hpp>
#include <type_traits>

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

inline glm::vec3 randWithinSphere(int radius)
{
    glm::vec3 ret;
    ret.x = rand_float_between(-1, 1);
    ret.y = rand_float_between(-1, 1);
    ret.z = rand_float_between(-1, 1);
    ret = glm::normalize(ret);
    ret *= cbrt(rand_float_between(0, 1)) * radius;
    return ret;
}


template <typename T>
class Interpolated
{
    public:
        Interpolated(T val, T speed) 
            : curVal(val), target(val), speed(speed) {};
        void update(float deltaTime)
        {
            if (target != curVal) {

                T dir = target - curVal;
                if constexpr(std::is_same<T, glm::vec3>::value) {
                    if (glm::length(dir) < glm::length(speed) * deltaTime) {
                        curVal = target;
                    } else {
                        dir = glm::normalize(dir);
                        T v = speed * deltaTime;
                        curVal += v;
                    }
                } else {
                    if (dir < abs(speed * deltaTime)) {
                        curVal = target;
                    } else {
                        dir = dir / abs(dir);
                        T v = speed * deltaTime;
                        curVal += v;
                    }
                }
            }
        }
        T target;
        T curVal;
        T speed;
};



#endif
