#include "drawable.h"

#include "event.h"
#include "camera2.h"

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

#include <plog/Log.h>


using namespace std;

/* More of the setup black magic */
std::vector<void(*)()>& _getSetupFuncs()
{
    static std::vector<void(*)()> funcs;
    return funcs;
}
void _addSetupFunc(void (*func)())
{
    _getSetupFuncs().push_back(func);
}
void runAllSetups()
{
    for (auto f : _getSetupFuncs()) {
        f();
    }
}
/* End setup black magic */


int UIObject::screenWidth;
int UIObject::screenHeight;
glm::mat4 UIObject::projection;
glm::mat4 UIObject::view;

glm::vec3 UIObject::calcWorldSpaceCoords(glm::vec2 screenCoords, float depth)
{
    glm::vec4 clipVec = projection * glm::vec4(0, 0, -depth, 1);
    float clipDepth = clipVec.z / clipVec.w;
    glm::vec4 clipCoords = glm::vec4(
            (2 * screenCoords.x / screenWidth - 1) /* * screenCoords.w */ ,
            -(2 * screenCoords.y / screenHeight - 1) /* * screenCoords.w */ ,
            clipDepth, 1);
    glm::vec4 worldCoords = glm::inverse(projection ) * clipCoords;
    worldCoords /= worldCoords.w;
    return worldCoords;
}

glm::vec2 UIObject::calcScreenSpaceCoords(glm::vec3 position)
{
    return ::calcScreenSpaceCoords(position, projection, view, screenWidth, screenHeight);
}

void UIObject::setPosScreenspace(glm::vec2 screenCoords, float depth)
{
    setPos(calcWorldSpaceCoords(screenCoords, depth));
}

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

void Selectable::updateClickState(UpdateInfo info)
{
        if (isHovered) {
            beingHovered = this;

            if (info.mouse.clicked) {
                wasClickedOn = true;
            } else if (wasClickedOn) {
                onClick();
                wasClickedOn = false;
                //Event::triggerEvent(EVENT_CLICK, shared_ptr<void>());
            }

        } else {
            beingHovered = NULL;
            wasClickedOn = false;
        }
}

bool Selectable::checkSetHoverQuad(UpdateInfo info, bool screenSpace) 
{
    if (isHovered or beingHovered == this or not beingHovered) {
        vector<glm::vec2> screenQuadCoords;
        for (auto quadVertex : quadVertices) {
            screenQuadCoords.push_back(calcScreenSpaceCoords(
                    getModel() * glm::vec4(quadVertex, 1),
                    info.projection, 
                    screenSpace ? glm::mat4(1.0f): info.camera->getViewMatrix(), 
                    info.screenWidth, info.screenHeight));
        }
        isHovered = pointInsideQuad(info.mouse.position, screenQuadCoords);

        updateClickState(info);
    }

    return isHovered;
}

Selectable* Selectable::beingHovered = NULL;

bool Selectable::checkSetHoverCircle(UpdateInfo info, bool screenSpace) 
{
    if (isHovered or (beingHovered == this or not beingHovered)) {
        glm::vec2 screenCoords = calcScreenSpaceCoords(position,
                info.projection, 
                screenSpace ? glm::mat4(1.0f): info.camera->getViewMatrix(), 
                info.screenWidth, info.screenHeight);
        
        isHovered = glm::length(screenCoords - info.mouse.position) <= targetRadius;
        updateClickState(info);
    }
    return isHovered;
}

void Selectable::onClick()
{
    LOG_INFO << "Clicked a selectable without a click handler";
}


Dragable* Dragable::beingDragged = NULL;

void Dragable::updateClickState(UpdateInfo info)
{
    // This solves other objects becoming hovered when another object is 
    // being dragged
    if (dragging) {
        isHovered = true;
    }
    Selectable::updateClickState(info);
}

void Dragable::checkSetDrag(UpdateInfo info, bool screenSpace)
{
    if (not dragEnabled) {
        return;
    }

    glm::vec3 curDragPos = calcWorldSpaceCoords(info.mouse.position, position.z);
    curDragPos.x = screenWidth - curDragPos.x;
    curDragPos.y = screenHeight - curDragPos.y;

    // Nothing else is being dragged, start dragging this
    if (isHovered and info.mouse.clicked and not beingDragged) {
        lastDragPos = curDragPos - position;
        dragging = true;
        beingDragged = this;
        
    // stop dragging on mouse release
    } else if (not info.mouse.clicked) {
        dragging = false;
        if (beingDragged == this) {
            beingDragged = NULL;
            onRelease();
        }
    }

    // update position if we are being dragged
    if (dragging) {
        position = curDragPos - lastDragPos;
    }
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

PointLight::PointLight(glm::vec3 position, glm::vec3 color)
    : Renderable(SHADER_LAMP)
{
    this->position = position;
    this->color = color;
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
    Shapes::sphere->draw(shader);
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
    model = shared_ptr<Model>(new Model("./res/models/cube.obj"));
} 
void Cube::draw(Shader shader)
{
    glm::mat4 model = glm::translate(glm::mat4(1.0f), position);
    model = glm::scale(model, glm::vec3(0.1, 0.1, 0.1));
    shader.setMat4("model", model);
    Cube::model->draw(shader);
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

shared_ptr<Model> Shapes::sphere;
shared_ptr<Model> Shapes::warpQuad;
volatile Shapes shapes; // Needed to force setup func to run
void Shapes::setup()
{
    sphere = shared_ptr<Model>(new Model("./res/models/sphere.obj"));
    warpQuad = shared_ptr<Model>(new Model("./res/models/quad/quad.obj"));
}

shared_ptr<Model> Cube::model;

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


