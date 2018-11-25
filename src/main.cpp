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
#include "cubemap.h"
//#include "drawable.h"
//#include "framebuffer.h"
#include "spaceThings.h"
#include "timer.h"


#include "renderer.h"

using namespace std;

// TODO clean up this global var as well
Camera camera(glm::vec3(-100.0f, 100.0f, 0));
//
// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------

// TODO clean up these global vars
double MOUSE_X = -1;
double MOUSE_Y = -1;
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    MOUSE_X = xpos;
    MOUSE_Y = ypos;
    cout << "Mouse position: " << xpos << " " << ypos << endl;
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    cout << "Camera offset: " << yoffset << endl;
    camera.ProcessMouseScroll(yoffset);
}

void processInput(GLFWwindow *window, Camera &camera, float deltaTime) 
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS or
       glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        camera.ProcessKeyboard(LEFT, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        camera.ProcessKeyboard(RIGHT, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        camera.ProcessKeyboard(FORWARD, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    }
}

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
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
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

void updateObjects(vector<shared_ptr<Object>> objects, UpdateInfo info) {
    for (auto o : objects) {
        o->update(info);
    }
}

int main()
{

    RenderOptions options = {
        .screenWidth=1000, 
        .screenHeight=800, 
        .fullscreen=false
    };

    GLFWwindow *window = setupOpenGlContext(options.screenWidth, options.screenHeight);

    // TODO initialize camera not as global
    //Camera camera(glm::vec3(-100.0f, 100.0f, 0));
    Renderer renderer(
            {.screenWidth=1000, .screenHeight=800, .fullscreen=false},
            camera);

    Skybox skybox("./res/textures/lightblue/");
    renderer.addRenderable(&skybox);
    SpaceGrid spacegrid;
    renderer.addRenderable(&spacegrid);
    vector<std::shared_ptr<Object>> ships;
    for (int i = 0; i < 30; i++) {
        ships.emplace_back(new SpaceShip("SS1", spacegrid.getSystem(0, 0)));
        renderer.addRenderable(dynamic_cast<Renderable*>(ships.back().get()));
    }

    int nUpdateThreads = 0;
    vector<thread> updateThreads(nUpdateThreads);

    Timer::create("frametime");
    vector<float> frameTimes;
    while(not glfwWindowShouldClose(window))
    {
        // TODO come up with a general update setup
        // TODO fix the deltatime not working
        UpdateInfo info; 
        info.deltaTime = Timer::getDelta("frametime");
        info.curTime = Timer::get("frametime");
        info.cameraPos = camera.Position;
        frameTimes.push_back(info.deltaTime);

        for (auto& s : spacegrid.getAllSystems()) {
            if (s->checkSetHover(
                        renderer.getProjection(), camera.GetViewMatrix(), 
                        MOUSE_X, MOUSE_Y, 
                        options.screenWidth, options.screenHeight)) {
                for (int i = 0; i < ships.size(); i++) {
                    dynamic_cast<SpaceShip*>(ships[i].get())->gotoSystem(s);
                }
            }
            s->update(info);
        }

        if (nUpdateThreads) {
            int portionSize = ships.size() / nUpdateThreads;
            for (int i=0; i < nUpdateThreads; i++) {
                vector<shared_ptr<Object>> portion(ships.begin() + i * portionSize, 
                        ships.begin() + (i+1) * portionSize + (i==nUpdateThreads-1 ? ships.size() % nUpdateThreads : 0));
                updateThreads[i] = thread(updateObjects, portion, info);
            }
        } else {
            updateObjects(ships, info);
        }

        renderer.renderFrame();
        glfwSwapBuffers(window);
        glfwPollEvents();
        processInput(window, camera, info.deltaTime);

        for (auto& t : updateThreads) {
            t.join();
        }

    };

    float average = 0;
    for (auto t : frameTimes) {
        average += t;
    }
    average /= frameTimes.size();
    cout << "Average frametime: " << average << endl;

    glfwTerminate();

    return 0;
}
