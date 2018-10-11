#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "shader.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window) 
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS or
       glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

int main()
{
    //Initialize glfw
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create window object
    GLFWwindow* window = glfwCreateWindow(800, 600, "LearnOpenGL", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    // set callback to resize viewport to window size if the window size is changed
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Initialize GLAD : loads all opengl function pointers
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        glfwTerminate();
        return -1;
    }

    // set size of opengl viewport to size of window
    glViewport(0, 0, 800, 600);

    // Use shader class
    Shader shader("src/vertex.vert", "src/fragment.frag");

    // Triangle vertice data
    //float vertices[] = {
    //    -0.5, -0.5, 0.0,
    //    0.5, -0.5, 0.0,
    //    0.0, 0.5, 0.0
    //};

	// Rectange vertex/index data
	float vertices[] = {
	     0.5f,  0.5f, 0.0f,  // top right
	     0.5f, -0.5f, 0.0f,  // bottom right
	    -0.5f, -0.5f, 0.0f,  // bottom left
	    -0.5f,  0.5f, 0.0f   // top left 
	};
	unsigned int indices[] = {  // note that we start from 0!
	    0, 1, 3,   // first triangle
	    1, 2, 3    // second triangle
	};  

    // Create vertex buffer and vertex array objects
    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	

    // Tell opengl how to interpret the vertice data
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*) 0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // wireframe mode
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // Keep going until window should close
    while (!glfwWindowShouldClose(window))
    {
        // TODO
        processInput(window);
       
        // rendering
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // swap buffers
        shader.use();
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glfwSwapBuffers(window);
        // check if any events are triggered
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
