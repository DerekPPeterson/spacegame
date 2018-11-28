#include "input.h"

#include <plog/Log.h>

MouseInfo MOUSE_INFO;
Camera* camera;


void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    MOUSE_INFO.position.x = xpos;
    MOUSE_INFO.position.y = ypos;
    LOG_VERBOSE << "Mouse position: " << xpos << " " << ypos;
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (action == GLFW_PRESS) {
        MOUSE_INFO.clicked = true;
        MOUSE_INFO.lastClickPosition = MOUSE_INFO.position;
    } else if (action == GLFW_RELEASE) {
        MOUSE_INFO.clicked = false;;
        MOUSE_INFO.lastReleasedPosition = MOUSE_INFO.position;
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    LOG_VERBOSE << "Camera offset: " << yoffset;
    camera->ProcessMouseScroll(yoffset);
}

void Input::process(float deltaTime) 
{
    glfwPollEvents();
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS or
       glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        camera->ProcessKeyboard(LEFT, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        camera->ProcessKeyboard(RIGHT, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        camera->ProcessKeyboard(FORWARD, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        camera->ProcessKeyboard(BACKWARD, deltaTime);
    }
}

Input::Input(GLFWwindow *window, Camera* camera) :
    mouse(MOUSE_INFO), window(window)
{
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    ::camera = camera;
}

