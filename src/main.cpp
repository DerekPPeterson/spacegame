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
#include "objectupdater.h"


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
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
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

int main()
{

    RenderOptions options = {
        .screenWidth=1000, 
        .screenHeight=800, 
        .fullscreen=false
    };

    GLFWwindow *window = setupOpenGlContext(options);


    // TODO initialize camera not as global
    //Camera camera(glm::vec3(-100.0f, 100.0f, 0));
    Renderer renderer(options, camera);
    
    // TODO testing only
    LineModel card("./res/models/card/card.obj");
    renderer.addRenderable(&card);

    // TODO move this stuff into a dedicated game logic setup areaa
    Skybox skybox("./res/textures/lightblue/");
    renderer.addRenderable(&skybox);

    vector<std::shared_ptr<Object>> objects;
    SpaceGrid spacegrid;
    //renderer.addRenderable(&spacegrid);
    vector<std::shared_ptr<Object>> systems = spacegrid.getAllSystems();
    objects.insert(objects.end(), make_move_iterator(systems.begin()), 
            make_move_iterator(systems.end()));
    for (int i = 0; i < 30; i++) {
        objects.emplace_back(new SpaceShip("SS1", spacegrid.getSystem(0, 0)));
        // TODO how will we remove objects that don't need to be rendered
        //renderer.addRenderable(dynamic_cast<Renderable*>(objects.back().get()));
    }
    cout << "Will render " << objects.size() << endl;

    ObjectUpdater updater(1); // Using 1 other thread for updating objects

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
        info.mousePos.x = MOUSE_X;
        info.mousePos.y = MOUSE_Y;
        frameTimes.push_back(info.deltaTime);
        cout << "Frametime: " << info.deltaTime << endl;

        updater.updateObjects(info, objects);

        renderer.renderFrame();
        glfwSwapBuffers(window);
        glfwPollEvents();
        processInput(window, camera, info.deltaTime);

        updater.waitForUpdates();
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
