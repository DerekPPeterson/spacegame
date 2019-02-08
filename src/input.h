#ifndef INPUT_H
#define INPUT_H

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

struct MouseInfo
{
    glm::vec2 position = {-1, -1};
    bool clicked = false;
    glm::vec2 lastClickPosition = {-1, -1};
    glm::vec2 lastReleasedPosition = {-1, -1};
};

class Camera;

class Input
{
    public:
        Input(GLFWwindow *window, Camera* camera);
        void process(float deltaTime);
        MouseInfo& mouse;
    private:
        GLFWwindow *window;
        
};
#endif
