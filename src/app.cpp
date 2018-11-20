#include <math.h>
#include <unistd.h>
#include <ctime>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"
#include "camera.h"
#include "model.h"
#include "cubemap.h"
#include "drawable.h"
#include "framebuffer.h"
#include "spaceThings.h"
#include "timer.h"

using namespace std;

int SCREEN_WIDTH = 1600;
int SCREEN_HEIGHT = 900;

// camera
Camera camera(glm::vec3(0.0f, 10.0f, 5));
float lastX = SCREEN_WIDTH / 2.0f;
float lastY = SCREEN_HEIGHT / 2.0f;
bool firstMouse = true;


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
	SCREEN_WIDTH = width;
	SCREEN_HEIGHT = height;
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


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

int main()
{
    srand(time(0));
    //Initialize glfw
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create window object
    GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, 
            "LearnOpenGL", glfwGetPrimaryMonitor(), NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    // set callback to resize viewport to window size if the window size is changed
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);


    // Initialize GLAD : loads all opengl function pointers
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        glfwTerminate();
        return -1;
    }

    Timer::create("start");

    // set size of opengl viewport to size of window
    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    // Compile needed shaders
    Shader shader("src/shaders/vertex.vert", "src/shaders/lighting.frag");
    Shader simpleDiffuse("src/shaders/vertex.vert", "src/shaders/simpleDiffuseLighting.frag");
    Shader blendShader("src/shaders/framebuffer.vert", "src/shaders/blend.frag");
    Shader framebufferShader("src/shaders/framebuffer.vert", "src/shaders/framebuffer.frag");
    Shader blurShader("src/shaders/framebuffer.vert", "src/shaders/blur.frag");
    Shader lampShader("src/shaders/lamp.vert", "src/shaders/lamp.frag");
    Shader skyboxShader("./src/shaders/skybox.vert", "./src/shaders/skybox.frag");
    Shader warpShader1("./src/shaders/warp.vert", "./src/shaders/warp1.frag");
    Shader warpShader2("./src/shaders/warp.vert", "./src/shaders/warp2.frag");

    // wireframe mode
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // Load Models/textures
    Skybox skybox("./res/textures/lightblue");
    Cube::setup();
    Quad::setup();
    PointLight::setup();
    SpaceGrid spaceGrid;
    vector<SpaceShip*> ships;
    for (int i = 0; i < 100; i++) {
        ships.push_back(new SpaceShip("SS1", spaceGrid.getSystem(0, 0)));
    }

    Framebuffers framebuffers(SCREEN_WIDTH, SCREEN_HEIGHT);
    Quad framebufferQuad({0, 0, 0});

	glm::mat4 projection(1.0);
	projection = glm::perspectiveRH((float) glm::radians(45.0), (float) SCREEN_WIDTH / SCREEN_HEIGHT, 0.1f, 10000.0f);
	//projection = glm::ortho(0.0f, (float) SCREEN_WIDTH, 0.0f, (float) SCREEN_HEIGHT, 0.1f, 10000.0f);


    vector<float> frameTimes;

    Timer::create("frametime");

    // Keep going until window should close
    float offset = 0;
    while (!glfwWindowShouldClose(window))
    {
        // Calculatetime difference and  process camera movement based on it
        float deltaTime = Timer::getDelta("frametime");
        frameTimes.push_back(deltaTime);
        processInput(window, camera, deltaTime);
        glm::mat4 view = camera.GetViewMatrix();

        // Clear the warp framebuffer before drawing the rest of the scene
        // because they both write to the same brightcolor texture
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffers.warpFrameBuffer.id);
        glClear(GL_COLOR_BUFFER_BIT);

        // Actually render scene
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffers.mainFramebuffer.id);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);  
       
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Render lights as objects
        lampShader.use();
        lampShader.setMat4("view", view);
        lampShader.setMat4("projection", projection);

        vector<shared_ptr<Light>> lights = Light::getAllLights();
        for (auto light : lights) {
            light->draw(lampShader);
        }

        // Render rest of scene lit by those lights
        shader.use();
        shader.setMat4("view", view);
        shader.setMat4("projection", projection);
		shader.setVec3("viewPos", camera.Position);
		shader.setInt("numPointLights", lights.size());
        shader.setFloat("ambientStrength", 0.00);

        for (int i = 0; i < lights.size(); i++) {
            lights[i]->setUniforms(shader, i);
        }

        for (int i = 0; i < ships.size(); i++) {
            if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS) {
                ships[i]->gotoSystem(spaceGrid.getSystem(0, 0));
            }
            if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
                ships[i]->gotoSystem(spaceGrid.getSystem(0, 1));
            }
            ships[i]->update(deltaTime);
            ships[i]->draw(shader);
        }

        simpleDiffuse.use();
        for (int i = 0; i < lights.size(); i++) {
            lights[i]->setUniforms(simpleDiffuse, i);
        }
        simpleDiffuse.setMat4("view", view);
        simpleDiffuse.setMat4("projection", projection);
		simpleDiffuse.setVec3("viewPos", camera.Position);
		simpleDiffuse.setInt("numPointLights", lights.size());
        simpleDiffuse.setFloat("ambientStrength", 0.0);
        spaceGrid.draw(simpleDiffuse);

        glm::mat4 skyboxView = glm::mat4(glm::mat3(camera.GetViewMatrix()));  
        skyboxShader.use();
        skyboxShader.setMat4("view", skyboxView);
        skyboxShader.setMat4("projection", projection);
        skybox.draw(skyboxShader);

        // Draw warp effects
        glDisable(GL_CULL_FACE);  
        //if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
        //    warpShader = Shader("./src/shaders/warp.vert", "./src/shaders/warp.frag");
        //}
        glEnable(GL_BLEND);
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffers.normalBlendFramebuffer.id);
        glClear(GL_COLOR_BUFFER_BIT);
        warpShader1.use();
        warpShader1.setMat4("view", view);
        warpShader1.setMat4("projection", projection);
        warpShader1.setVec2("screenSize", {SCREEN_WIDTH, SCREEN_HEIGHT});
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, framebuffers.mainFramebuffer.colorTextures[0]);
        warpShader1.setInt("hdrBuffer", 1);
        for (int i = 0; i < ships.size(); i++) {
            ships[i]->drawWarp(warpShader1, camera.Position);
        }
        glDisable(GL_BLEND);

        // Copy over depth info so we don't render the warp effect over planets etc
        glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffers.mainFramebuffer.id);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffers.warpFrameBuffer.id);
        glBlitFramebuffer(
                  0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GL_DEPTH_BUFFER_BIT, GL_NEAREST
                );

        glEnable(GL_DEPTH_TEST);
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffers.warpFrameBuffer.id);
        //glClear(GL_COLOR_BUFFER_BIT);
        warpShader2.use();
        warpShader2.setMat4("view", view);
        warpShader2.setMat4("projection", projection);
        warpShader2.setVec2("screenSize", {SCREEN_WIDTH, SCREEN_HEIGHT});
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, framebuffers.mainFramebuffer.colorTextures[0]);
        warpShader2.setInt("hdrBuffer", 1);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, framebuffers.normalBlendFramebuffer.colorTextures[0]);
        warpShader2.setInt("normalAdjustBuffer", 2);
        for (int i = 0; i < ships.size(); i++) {
            ships[i]->drawWarp(warpShader2, camera.Position);
        }

		bool horizontal = true, first_iteration = true;
		int amount = 4;

        // Downscale bright image
        glViewport(0, 0, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
        glDisable(GL_DEPTH_TEST);
        framebufferShader.use();
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffers.pingpongBuffers[!horizontal].id); 
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, framebuffers.mainFramebuffer.colorTextures[1]);
        framebufferShader.setInt("hdrBuffer", 0);
        //glGenerateMipmap(GL_TEXTURE_2D);
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 1);
        framebufferQuad.draw();

        // Blur bright areas for bloom
		blurShader.use();
		for (unsigned int i = 0; i < amount; i++)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, framebuffers.pingpongBuffers[horizontal].id); 
			blurShader.setInt("horizontal", horizontal);
			glBindTexture(GL_TEXTURE_2D, 
                    framebuffers.pingpongBuffers[!horizontal].colorTextures[0]
			); 

			glActiveTexture(GL_TEXTURE0);
			glDrawArrays(GL_TRIANGLES, 0, 6);

			horizontal = !horizontal;
		}


        //// Merge bloom and scene and draw result
		glBindFramebuffer(GL_FRAMEBUFFER, 0); 
        glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT );
        //glDisable(GL_DEPTH_TEST);
        //glActiveTexture(GL_TEXTURE0);
        //glBindTexture(GL_TEXTURE_2D, framebuffers.warpFrameBuffer.colorTextures[0]);
        //glActiveTexture(GL_TEXTURE1);
        //glBindTexture(GL_TEXTURE_2D, framebuffers.pingpongBuffers[!horizontal].colorTextures[0]);
        //blendShader.use();
        //blendShader.setInt("hdrBuffer1", 1);
        //framebufferQuad.draw();
        
        glDisable(GL_DEPTH_TEST);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, framebuffers.mainFramebuffer.colorTextures[0]);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, framebuffers.pingpongBuffers[!horizontal].colorTextures[0]);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, framebuffers.warpFrameBuffer.colorTextures[0]);
        blendShader.use();
        blendShader.setInt("hdrBuffer1", 1);
        blendShader.setInt("hdrBuffer2", 2);
        framebufferQuad.draw();

        // check if any events are triggered
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    float average = 0;
    for (auto t : frameTimes) {
        average += t;
    }
    average /= frameTimes.size();
    cout << "Average frametime: " << average << endl;

    glfwTerminate();
    return 0;
}
