#include <math.h>
#include <unistd.h>

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

int SCREEN_WIDTH = 1600;
int SCREEN_HEIGHT = 900;;

// camera
Camera camera(glm::vec3(0.0f, 10.0f, 5));
float lastX = SCREEN_WIDTH / 2.0f;
float lastY = SCREEN_HEIGHT / 2.0f;
bool firstMouse = true;

float deltaTime = 0;
float lastFrame = 0;


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

    // set size of opengl viewport to size of window
    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    // Compile needed shaders
    Shader shader("src/shaders/vertex.vert", "src/shaders/lighting.frag");
    Shader blendShader("src/shaders/framebuffer.vert", "src/shaders/blend.frag");
    Shader framebufferShader("src/shaders/framebuffer.vert", "src/shaders/framebuffer.frag");
    Shader blurShader("src/shaders/framebuffer.vert", "src/shaders/blur.frag");
    Shader lampShader("src/shaders/lamp.vert", "src/shaders/lamp.frag");
    Shader skyboxShader("./src/shaders/skybox.vert", "./src/shaders/skybox.frag");

    // wireframe mode
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // Load Models/textures
    //Model spaceship("./res/models/Viper/Viper-mk-IV-fighter.obj");
    Model starship("./res/models/SS1_OBJ/SS1.obj");
    Skybox skybox("./res/textures/lightblue");
    Cube::setup();
    Quad::setup();
    PointLight::setup();
    SpaceGrid spaceGrid = SpaceGrid();

    Framebuffers framebuffers(SCREEN_WIDTH, SCREEN_HEIGHT);
    Quad framebufferQuad({0, 0, 0});

	glm::mat4 projection(1.0);
	projection = glm::perspectiveRH((float) glm::radians(45.0), (float) SCREEN_WIDTH / SCREEN_HEIGHT, 0.1f, 10000.0f);
	//projection = glm::ortho(0.0f, (float) SCREEN_WIDTH, 0.0f, (float) SCREEN_HEIGHT, 0.1f, 10000.0f);

    // Keep going until window should close
    float offset = 0;
    while (!glfwWindowShouldClose(window))
    {
        // Calculatetime difference and  process camera movement based on it
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        processInput(window, camera, deltaTime);
        glm::mat4 view = camera.GetViewMatrix();

        // Actually render scene
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffers.mainFramebuffer.id);
        glEnable(GL_DEPTH_TEST);
       
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

        //glm::vec3 lightPos(1, 1, -2);
        //glm::mat4 lightRotation = glm::rotate(glm::mat4(1.0f), 0.5f* (float) glfwGetTime(), glm::vec3(0, 0, 1));
        //lightPos = lightRotation * glm::vec4(lightPos[0], lightPos[1], lightPos[2], 1);

        //lampShader.use();
        //glBindVertexArray(lightVAO);
        //glm::mat4 lightModel(1.0f);
        //lightModel = glm::translate(lightModel, lightPos);
        //lightModel = glm::scale(lightModel, glm::vec3(0.02, 0.02, 0.02));

        //lampShader.setMat4("view", view);
        //lampShader.setMat4("projection", projection);
        //lampShader.setMat4("model", lightModel);

        //glDrawArrays(GL_TRIANGLES, 0, 36);

        //    
        //shader.use();
        //shader.setMat4("view", view);
        //shader.setMat4("projection", projection);

        //shader.setVec3("lightColor", glm::vec3(3, 3, 3));
		//shader.setVec3("lightPos", lightPos);
		//shader.setVec3("viewPos", camera.Position);

        glm::mat4 shipRotation = glm::rotate(glm::mat4(1.0f), 2.0f * (float) glfwGetTime(), glm::vec3(0.2, 1, 0));

        //glm::mat4 spaceshipModel(1.0f);
        //spaceshipModel = glm::scale(spaceshipModel, glm::vec3(0.1, 0.1, 0.1));
        //spaceshipModel = glm::translate(spaceshipModel, glm::vec3(0, 0, 10));
        //shader.setMat4("model", spaceshipModel);
        ////spaceship.draw(shader);

        glm::mat4 starshipModel(1.0f);
        starshipModel = glm::scale(starshipModel, glm::vec3(0.5, 0.5, 0.5));

        for (int i = 0; i < 0; i++) {
            glm::mat4 shipTranslation = glm::translate(starshipModel, glm::vec3(0, i * 5, i * 10));
            shader.setMat4("model", shipTranslation  * shipRotation * starshipModel);
            starship.draw(shader);
        }

        spaceGrid.draw(shader);

        glm::mat4 skyboxView = glm::mat4(glm::mat3(camera.GetViewMatrix()));  
        skyboxShader.use();
        skyboxShader.setMat4("view", skyboxView);
        skyboxShader.setMat4("projection", projection);
        skybox.draw(skyboxShader);

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
		glBindFramebuffer(GL_FRAMEBUFFER, 0); 

        glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT );

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Merge bloom and scene and draw result
        glDisable(GL_DEPTH_TEST);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, framebuffers.mainFramebuffer.colorTextures[0]);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, framebuffers.pingpongBuffers[!horizontal].colorTextures[0]);
        blendShader.use();
        blendShader.setInt("hdrBuffer1", 1);
        framebufferQuad.draw();

        // check if any events are triggered
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
