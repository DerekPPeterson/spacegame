#include <math.h>
#include <unistd.h>
#include <ctime>
#include <exception>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//#include "shader.h"
//#include "camera.h"
//#include "model.h"
//#include "cubemap.h"
//#include "drawable.h"
//#include "framebuffer.h"
//#include "spaceThings.h"
//#include "timer.h"


#include "renderer.h"

GLFWwindow* setupOpenGlContext(int SCREEN_WIDTH, int SCREEN_HEIGHT)
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create window object
    GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, 
            "LearnOpenGL", NULL, NULL);
    if (window == NULL) {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }
    glfwMakeContextCurrent(window);
    // set callback to resize viewport to window size if the window size is changed
    //glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    //glfwSetCursorPosCallback(window, mouse_callback);
    //glfwSetScrollCallback(window, scroll_callback);
    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);


    // Initialize GLAD : loads all opengl function pointers
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress))
    {
        glfwTerminate();
        throw std::runtime_error("Failed to initialize GLAD");
    }

    //Timer::create("start");

    // set size of opengl viewport to size of window
    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    return window;
}


int main()
{
    RenderOptions options = {
        .screenWidth=1000, 
        .screenHeight=800, 
        .fullscreen=false
    };

    GLFWwindow *window = setupOpenGlContext(options.screenWidth, options.screenHeight);

    Camera camera; 
    Renderer renderer(
            {.screenWidth=1000, .screenHeight=800, .fullscreen=false},
            camera);

    auto swapWindowsCallback = [window]() {glfwSwapBuffers(window);};
    renderer.start(swapWindowsCallback);

    return 0;
}
