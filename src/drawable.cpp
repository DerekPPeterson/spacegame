#include "drawable.h"

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>


using namespace std;

glm::vec2 calcScreenSpaceCoords(glm::vec3 position, 
        glm::mat4 projection, glm::mat4 view,
        int screenWidth, int screenHeight)
{
    glm::vec4 clipCoords = 
        projection * view * glm::vec4(position, 1);
    glm::vec2 screenCoords(
            (clipCoords.x / clipCoords.w + 1) / 2 * screenWidth, 
            screenHeight - (clipCoords.y / clipCoords.w + 1) / 2 * screenHeight);
    return screenCoords;
}

bool pointInsideQuad(glm::vec2 point, vector<glm::vec2> quad)
{
    glm::vec3 p(point, 0);
    float lastz = 1;;
    for (int i=0; i < 4; i++) {
        glm::vec3 v1(quad[i], 0);
        glm::vec3 v2;
        if (i != 3) {
            v2 = glm::vec3(quad[i+1], 0);
        } else {
            v2 = glm::vec3(quad[0], 0);
        }
        float zcross = (p.x - v1.x) * (v2.y - v1.y) - (v2.x - v1.x) * (p.y - v1.y);
        if (i > 0  and zcross * lastz < 0) {
            return false;
        }
        lastz = zcross;
    }
    return true;
}

bool Selectable::checkSetHoverQuad(const glm::mat4 projection, const glm::mat4 view, 
        float mouseX, float mouseY,
        int screenWidth, int screenHeight) 
{
    vector<glm::vec2> screenQuadCoords;
    for (auto quadVertex : quadVertices) {
        screenQuadCoords.push_back(calcScreenSpaceCoords(
                    getModel() * glm::vec4(quadVertex, 1),
                    projection, view, screenWidth, screenHeight));
    }
    isHovered = pointInsideQuad(glm::vec2(mouseX, mouseY), screenQuadCoords);
    return isHovered;
}


bool Selectable::checkSetHoverCircle(const glm::mat4 projection, const glm::mat4 view, 
        float mouseX, float mouseY,
        int screenWidth, int screenHeight) 
{
    glm::vec2 screenCoords = calcScreenSpaceCoords(position,
            projection, view, screenWidth, screenHeight);
    
    isHovered = glm::length(screenCoords - glm::vec2(mouseX, mouseY)) <= targetRadius;
    return isHovered;
}

//Object::Object()
//{
//    //objects.push_back
//};

// TODO pretty sure these lights will never get destroyed automatically
vector<shared_ptr<Light>> Light::allLights = {};

vector<shared_ptr<Light>> Light::getAllLights()
{
    return allLights;
}

shared_ptr<Light> Light::makeLight(LightType type, glm::vec3 position, glm::vec3 color)
{
    shared_ptr<Light> new_light;
    switch(type) {
        case LIGHT_POINT:
            new_light = shared_ptr<Light>(new PointLight(position, color));
    }
    
    allLights.push_back(new_light);

    return new_light;
}

void PointLight::setup()
{
    sphere = Model("./res/models/sphere.obj");
}

PointLight::PointLight(glm::vec3 position, glm::vec3 color)
{
    this->position = position;
    this->color = color;
    stage = SHADER_LAMP;
}

void PointLight::setUniforms(Shader shader, int iPointLight)
{
    string arrayElem = "pointLights[" + to_string(iPointLight) + "]";

    shader.setVec3(arrayElem + ".position", position);
    shader.setVec3(arrayElem + ".color", color);
    shader.setVec3(arrayElem + ".attenuation", attenuation);
}

void PointLight::draw(Shader& shader)
{
    shader.setVec3("color", color);
    glm::mat4 model = glm::translate(glm::mat4(1.0f), position);
    model = glm::scale(model, glm::vec3(size, size, size));
    shader.setMat4("model", model);
    sphere.draw(shader);
}

void Light::setColor(glm::vec3 color)
{
    this->color = color;
}

Cube::Cube(glm::vec3 position)
{
    this->position = position;
}

void Cube::setup()
{
    model = Model("./res/models/cube.obj");
    isSetup = true;
} 
void Cube::draw(Shader shader)
{
    glm::mat4 model = glm::translate(glm::mat4(1.0f), position);
    model = glm::scale(model, glm::vec3(0.1, 0.1, 0.1));
    shader.setMat4("model", model);
    Cube::model.draw(shader);
}

Quad::Quad(glm::vec3 position)
{
    this->position = position;
}

void Quad::setup()
{
    if (not isSetup) {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, 4 * numVertices * sizeof(float), vertices, 
                GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 
                4 * sizeof(float), (void*) 0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 
                4 * sizeof(float), (void*) (2 * sizeof(float)));
        glEnableVertexAttribArray(1);
        isSetup = true;
    }
} 

void Quad::draw()
{
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, numVertices);
}

Model PointLight::sphere = Model();

Model Cube::model;
bool Cube::isSetup = false;

unsigned int Quad::VBO = -1;
unsigned int Quad::VAO = -1;
int Quad::numVertices = 6;
bool Quad::isSetup = false;
float Quad::vertices[] = {  
    // positions   // texCoords
    -1.0f,  1.0f,  0.0f, 1.0f,
    -1.0f, -1.0f,  0.0f, 0.0f,
     1.0f, -1.0f,  1.0f, 0.0f,

    -1.0f,  1.0f,  0.0f, 1.0f,
     1.0f, -1.0f,  1.0f, 0.0f,
     1.0f,  1.0f,  1.0f, 1.0f
    };	


