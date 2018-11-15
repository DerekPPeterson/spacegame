#include <iostream>
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

int SCREEN_WIDTH = 800;
int SCREEN_HEIGHT = 600;;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
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
            "LearnOpenGL", NULL, NULL);
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

    // Use shader class
    Shader shader("src/vertex.vert", "src/fragment.frag");
    
	// rectangle with color and texture coords data
	float cubeVertices[] = {
		-0.5f, -0.5f, -0.5f,  
		 0.5f, -0.5f, -0.5f,  
		 0.5f,  0.5f, -0.5f,  
		 0.5f,  0.5f, -0.5f,  
		-0.5f,  0.5f, -0.5f,  
		-0.5f, -0.5f, -0.5f,  

		-0.5f, -0.5f,  0.5f,  
		 0.5f, -0.5f,  0.5f,  
		 0.5f,  0.5f,  0.5f,  
		 0.5f,  0.5f,  0.5f,  
		-0.5f,  0.5f,  0.5f,  
		-0.5f, -0.5f,  0.5f,  

		-0.5f,  0.5f,  0.5f,  
		-0.5f,  0.5f, -0.5f,  
		-0.5f, -0.5f, -0.5f,  
		-0.5f, -0.5f, -0.5f,  
		-0.5f, -0.5f,  0.5f,  
		-0.5f,  0.5f,  0.5f,  

		 0.5f,  0.5f,  0.5f,  
		 0.5f,  0.5f, -0.5f,  
		 0.5f, -0.5f, -0.5f,  
		 0.5f, -0.5f, -0.5f,  
		 0.5f, -0.5f,  0.5f,  
		 0.5f,  0.5f,  0.5f,  

		-0.5f, -0.5f, -0.5f,  
		 0.5f, -0.5f, -0.5f,  
		 0.5f, -0.5f,  0.5f,  
		 0.5f, -0.5f,  0.5f,  
		-0.5f, -0.5f,  0.5f,  
		-0.5f, -0.5f, -0.5f,  

		-0.5f,  0.5f, -0.5f,  
		 0.5f,  0.5f, -0.5f,  
		 0.5f,  0.5f,  0.5f,  
		 0.5f,  0.5f,  0.5f,  
		-0.5f,  0.5f,  0.5f,  
		-0.5f,  0.5f, -0.5f  
	};

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Setup light objects
    Shader lampShader("src/lamp.vert", "src/lamp.frag");
    lampShader.use();
    unsigned int lightVBO, lightVAO;
    glGenVertexArrays(1, &lightVAO);
    glGenBuffers(1, &lightVBO);
    glBindVertexArray(lightVAO);
    glBindBuffer(GL_ARRAY_BUFFER, lightVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*) 0);
    glEnableVertexAttribArray(0);


    // wireframe mode
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	//depth testing
	glEnable(GL_DEPTH_TEST);

    Model spaceship("./res/models/Viper/Viper-mk-IV-fighter.obj");
    Model starship("./res/models/SS1_OBJ/SS1.obj");

    Shader skyboxShader("./src/shaders/skybox.vert", "./src/shaders/skybox.frag");
    Skybox skybox("./res/textures/lightblue");

	glm::mat4 projection(1.0);
	projection = glm::perspective((float) glm::radians(45.0), (float) SCREEN_WIDTH / SCREEN_HEIGHT, 0.1f, 100.0f);

    // Keep going until window should close
    float offset = 0;
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        processInput(window, camera, deltaTime);
        glm::mat4 view = camera.GetViewMatrix();
       
        // rendering
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        glm::vec3 lightPos(1, 1, -2);
        glm::mat4 lightRotation = glm::rotate(glm::mat4(1.0f), 3.0f * (float) glfwGetTime(), glm::vec3(0, 0, 1));
        lightPos = lightRotation * glm::vec4(lightPos[0], lightPos[1], lightPos[2], 1);

        lampShader.use();
        glBindVertexArray(lightVAO);
        glm::mat4 lightModel(1.0f);
        lightModel = glm::translate(lightModel, lightPos);
        lightModel = glm::scale(lightModel, glm::vec3(0.2, 0.2, 0.2));

        lampShader.setMat4("view", view);
        lampShader.setMat4("projection", projection);
        lampShader.setMat4("model", lightModel);

        glDrawArrays(GL_TRIANGLES, 0, 36);

            
        shader.use();
        shader.setMat4("view", view);
        shader.setMat4("projection", projection);

        shader.setVec3("lightColor", glm::vec3(0.5, 0.5, .5));
		shader.setVec3("lightPos", lightPos);
		shader.setVec3("viewPos", camera.Position);


        glm::mat4 spaceshipModel(1.0f);
        spaceshipModel = glm::scale(spaceshipModel, glm::vec3(0.1, 0.1, 0.1));
        spaceshipModel = glm::translate(spaceshipModel, glm::vec3(0, 0, 10));
        glm::mat4 shipRotation = glm::rotate(glm::mat4(1.0f), 2.0f * (float) glfwGetTime(), glm::vec3(0.2, 1, 0));
        spaceshipModel = shipRotation * spaceshipModel;
        shader.setMat4("model", spaceshipModel);
        spaceship.draw(shader);

        glm::mat4 starshipModel(1.0f);
        starshipModel = glm::scale(starshipModel, glm::vec3(0.1, 0.1, 0.1));
        starshipModel = glm::translate(starshipModel, glm::vec3(0.0, 1, 0));
        starshipModel = shipRotation * starshipModel;
        shader.setMat4("model", starshipModel);
        starship.draw(shader);

        glm::mat4 skyboxView = glm::mat4(glm::mat3(camera.GetViewMatrix()));  
        skyboxShader.use();
        skyboxShader.setMat4("view", skyboxView);
        skyboxShader.setMat4("projection", projection);
        skybox.draw(skyboxShader);

        glfwSwapBuffers(window);
        // check if any events are triggered
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
