#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "drawable.h"

#include <vector>

class Camera 
{
    public:
        Camera(glm::vec3 position = {0, 0, 0}, glm::vec3 at = {1, 0, 0}, 
                glm::vec3 up = {0, 1, 0})
            : position(position), at(at), up(up), 
            initialPos(position), initialAt(at)
        {
            lookAt(position, at, 0);
        }

        glm::mat4 getViewMatrix()
        {
            return glm::lookAt(curPosition, curAt, up);
        }

        glm::vec3 getPos()
        {
            return position;
        }

        void lookAt(glm::vec3 newPosition, glm::vec3 newAt, float duration = 0.5)
        {
            position = newPosition;
            at = newAt;

            if (duration) {
                posSpeed = glm::length(position - curPosition) / duration;
                atSpeed = glm::length(at - curAt) / duration;
            } else {
                curPosition = position;
                curAt = at;
                posSpeed = 0;
                atSpeed = 0;
            }
        }

        void update(UpdateInfo info)
        {
            auto distLeft = glm::length(position - curPosition);
            if (distLeft > 0) {
                auto moveVec = glm::normalize(position - curPosition) * posSpeed * info.deltaTime;
                if (glm::length(moveVec) < distLeft) {
                    curPosition += moveVec;
                } else {
                    curPosition = position;
                }
            }

            auto atDistLeft = glm::length(at - curAt);
            if (atDistLeft > 0) {
                auto atVec = glm::normalize(at - curAt) * atSpeed * info.deltaTime;
                if (glm::length(atVec) < atDistLeft) {
                    curAt += atVec;
                } else {
                    curAt = at;
                }
            }
        }

        void reset(float duration = 0.5)
        {
            lookAt(initialPos, initialAt, duration);
        }

        void reset(glm::vec3 position, glm::vec3 at, float duration = 0.5)
        {
            initialPos = position;
            initialAt = at;
            lookAt(position, at, duration);
        }

    private:
        glm::vec3 curPosition;
        glm::vec3 curAt;

        glm::vec3 position;
        glm::vec3 at;
        glm::vec3 up;

        glm::vec3 initialPos;
        glm::vec3 initialAt;

        float posSpeed;
        float atSpeed;
};

#endif
