#include <math.h>
#include <unistd.h>
#include <ctime>
#include <exception>
#include <cstdio>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <plog/Log.h>

#include <cxxopts.hpp>

//#include "shader.h"
//#include "camera.h"
//#include "model.h"
#include "cubemap.h"
//#include "drawable.h"
//#include "framebuffer.h"
#include "timer.h"
#include "objectupdater.h"
#include "input.h"
#include "gamelogic.h"


#include "renderer.h"

using namespace std;

//
// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------

// TODO isolate these mouse/keyboard functions
GLFWwindow* setupOpenGlContext(RenderOptions options)
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create window object
    GLFWwindow* window;
    if (options.fullscreen) {
        window = glfwCreateWindow(options.screenWidth, options.screenHeight, 
                "LearnOpenGL", glfwGetPrimaryMonitor(), NULL);
    } else {
        window = glfwCreateWindow(options.screenWidth, options.screenHeight, 
                "LearnOpenGL", NULL, NULL);
    }
    if (window == NULL) {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }
    glfwMakeContextCurrent(window);
    // set callback to resize viewport to window size if the window size is changed
    //glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);


    // Initialize GLAD : loads all opengl function pointers
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress))
    {
        glfwTerminate();
        throw std::runtime_error("Failed to initialize GLAD");
    }

    Timer::create("start");

    // set size of opengl viewport to size of window
    glViewport(0, 0, options.screenWidth, options.screenHeight);
    return window;
}

int main(int argc, char **argv)
{
    cxxopts::Options opts("Spacegame", "A game");
    opts.add_options()
        ("w,screenwidth", "Horizontal display resolution", cxxopts::value<int>()->default_value("1000"))
        ("h,screenheight", "Vertical display resolution", cxxopts::value<int>()->default_value("800"))
        ("f,fullscreen", "Enable fullscreen")
        ("l,logfile", "Log output location", cxxopts::value<string>()->default_value("spacegame.log"))
        ;
    auto result = opts.parse(argc, argv);

    remove(result["logfile"].as<string>().c_str());
    plog::init(plog::verbose, result["logfile"].as<string>().c_str());
    LOG_INFO << "Starting program";

    RenderOptions options = {
        .screenWidth=result["screenwidth"].as<int>(),
        .screenHeight=result["screenheight"].as<int>(), 
        .fullscreen=(result.count("fullscreen") > 0)
    };

    Camera camera(glm::vec3(-100.0f, 100.0f, 0));

    GLFWwindow *window = setupOpenGlContext(options);

    runAllSetups();

    Input input(window, &camera);

    Renderer renderer(options, camera);

    ObjectUpdater updater(1); // Using 1 other thread for updating objects

    GameLogic gameLogic;
    gameLogic.startGame(options, renderer);

    shared_ptr<Skybox> skybox(new Skybox("./res/textures/lightblue/"));
    renderer.addRenderable(skybox);

    Timer::create("frametime");
    vector<float> frameTimes; // Used for calculating average frametime
    UpdateInfo info; 
    info.camera = &camera;
    info.screenWidth = options.screenWidth;
    info.screenHeight = options.screenHeight;
    info.projection = renderer.getProjection();

    while(not glfwWindowShouldClose(window))
    {
        // Prepare update data to update all objects for movement and such
        info.deltaTime = Timer::getDelta("frametime");
        info.curTime = Timer::get("frametime");
        info.mouse = input.mouse;
        frameTimes.push_back(info.deltaTime);
        LOG_INFO << "Frametime: " << info.deltaTime;

        gameLogic.updateState();
        renderer.setToRender(gameLogic.getRenderables());

        // Update objects
        updater.updateObjects(info, gameLogic.getObjects());
        updater.waitForUpdates(); // Cannot overlap with render yet

        renderer.renderFrame();
        glfwSwapBuffers(window);

        input.process(info.deltaTime);
    };
    
    float average = 0;
    for (auto t : frameTimes) {
        average += t;
    }
    average /= frameTimes.size();
    LOG_INFO << "Average frametime: " << average;

    glfwTerminate();

    return 0;
}
