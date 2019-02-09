#include <math.h>
#include <unistd.h>
#include <ctime>
#include <exception>
#include <cstdio>

#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <plog/Log.h>

#include <cxxopts.hpp>
#include <ctime>

#include "cubemap.h"
#include "timer.h"
#include "objectupdater.h"
#include "input.h"
#include "logic.h"
#include "gamelogic.h"
#include "client.h"
#include "camera2.h"


#include "renderer.h"

using namespace std;

//
// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------

// TODO isolate these mouse/keyboard functions
GLFWwindow* setupOpenGlContext(RenderOptions options)
{
    srand(time(NULL));

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create window object
    GLFWwindow* window;
    if (options.fullscreen) {
        window = glfwCreateWindow(options.screenWidth, options.screenHeight, 
                "Spacegame", glfwGetPrimaryMonitor(), NULL);
    } else {
        window = glfwCreateWindow(options.screenWidth, options.screenHeight, 
                "Spacegame", NULL, NULL);
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
        ("u,username", "set username", cxxopts::value<string>()->default_value("AckbarsRevenge"))
        ("j,joingame", "join a game rather than starting one", cxxopts::value<string>())
        ("joinuser", "join a game by user rather than starting one", cxxopts::value<string>())
        ;
    auto result = opts.parse(argc, argv);

    //remove(result["logfile"].as<string>().c_str());
    std::ofstream ofs;
    ofs.open(result["logfile"].as<string>().c_str(), std::ofstream::out | std::ofstream::trunc);
    ofs.close();
    plog::init(plog::verbose, result["logfile"].as<string>().c_str());
    LOG_INFO << "Starting program";

    RenderOptions options = {
        .screenWidth=result["screenwidth"].as<int>(),
        .screenHeight=result["screenheight"].as<int>(), 
        .fullscreen=(result.count("fullscreen") > 0)
    };



    Camera camera(glm::vec3(-40, 40, 40));

    GLFWwindow *window = setupOpenGlContext(options);

    runAllSetups();

    Input input(window, &camera);

    Renderer renderer(options, camera);

    GameClient client("localhost", 40000);
    client.login(result["username"].as<string>());
    if (result.count("joingame")) {
        client.joinGame(result["joingame"].as<string>());
    }else if (result.count("joinuser")) {
        client.joinUser(result["joinuser"].as<string>());
    } else {
        client.startGame();
    }

    // Animation updater
    ObjectUpdater updater(1); // Using 1 other thread for updating objects

    GraphicsObjectHandler graphicsObjectHandler(camera);
    graphicsObjectHandler.startGame(client.getState(), client.getMyPlayerId());

    // TODO create this somewhere else
    shared_ptr<Skybox> skybox(new Skybox("./res/textures/lightblue/"));
    renderer.addRenderable(skybox);

    vector<float> frameTimes; // Used for calculating average frametime
    UpdateInfo info; 
    info.camera = &camera;
    info.screenWidth = options.screenWidth;
    info.screenHeight = options.screenHeight;
    info.projection = renderer.getProjection();

    vector<logic::Action> actions;
    int lastChangeNo = 0;

    Timer::global = Timer("global"); // restart global timer
    while(not glfwWindowShouldClose(window))
    {
        // Prepare update data to update all objects for movement and such
        info.deltaTime = Timer::global.getDelta();
        info.curTime = Timer::global.get();
        info.mouse = input.mouse;
        frameTimes.push_back(info.deltaTime);
        LOG_VERBOSE << "Frametime: " << info.deltaTime;

        if (actions.size()) {
            // This will trigger animations and iterface for selecting
            // an action
            graphicsObjectHandler.setPossibleActions(actions);
        } else {
            // If there are no current actions get pending ones from the 
            // server This might return an empty list, in which case we will
            // ask again
            graphicsObjectHandler.setPossibleActions(actions);
            actions = client.getActions();
        }

        // Once the player selects an action send it to the client and clear 
        // the actions list
        graphicsObjectHandler.checkEvents();
        auto selectedAction = graphicsObjectHandler.getSelectedAction();
        if (selectedAction) {
            client.performAction(*selectedAction);
            actions.clear();
        }

        // Get a list of changes to state from the server and queue game updates
        // accordingly 
        auto changes = client.getChangesSince(lastChangeNo);
        if (changes.size()) {
            lastChangeNo = changes.back().changeNo;
        }
        graphicsObjectHandler.updateState(changes, info);

        // Update objects
        updater.updateObjects(info, graphicsObjectHandler.getObjects());
        updater.waitForUpdates(); // Cannot overlap with render yet

        // Update camera
        camera.update(info);

        // Render a frame
        renderer.setToRender(graphicsObjectHandler.getRenderables());
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
