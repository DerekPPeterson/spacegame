#include "spaceThings.h"
#include "drawable.h"
#include "timer.h"
#include "util.h"
#include "event.h"

#include <cstdlib>
#include <algorithm>
#include <GLFW/glfw3.h>

#include <plog/Log.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>

using namespace std;


System::System(glm::vec3 position, int gridx, int gridy) :
    Renderable(SHADER_SIMPLE_DIFFUSE), gridx(gridx), gridy(gridy)
{ 
    this->position = position;
    sun = dynamic_pointer_cast<PointLight>(
            Light::makeLight(LIGHT_POINT, position, baseColor));

    int nPlanets = rand() % 7 + 1;
    float spacing = 1.6;
    for (int i = 0; i < nPlanets; i++) {
        Planet new_planet;
        new_planet.radius = rand_float_between(0.05, 0.4);
        new_planet.orbit.radius = rand_float_between(spacing, spacing + new_planet.radius * 3);
        spacing = new_planet.orbit.radius + new_planet.radius * 2;
        new_planet.orbit.phase = rand_float_between(0, 2 * 3.14);
        new_planet.orbit.inclination = rand_float_between(0, 3.14 / 8);

        new_planet.color = glm::vec3(
                rand_float_between(0, 1),
                rand_float_between(0, 1),
                rand_float_between(0, 1)
                );

        planets.push_back(new_planet);
    }
}

glm::vec3 calcOrbitPosition(glm::vec3 systemPosition, Orbit &orbit)
{
    glm::mat4 transform = glm::translate(glm::mat4(1.0f), systemPosition);

    transform = glm::rotate(transform, orbit.phase, {0, 1, 0});
    glm::vec3 orbitAxis = glm::rotateX(glm::vec3(0, 1, 0), orbit.inclination);
    transform = glm::rotate(transform, 
             1.0f / (float) pow(orbit.radius , 3.0/2.0) * Timer::get("start"), 
            orbitAxis);
    transform = glm::translate(transform, glm::vec3(orbit.radius, 0, 0));

    glm::vec4 position = transform * glm::vec4(0, 0, 0, 1);
    return position;
}

void System::draw(Shader& shader) 
{
    for (auto planet : planets) {
        glm::mat4 planetModel = glm::translate(glm::mat4(1.0f), 
                calcOrbitPosition(position, planet.orbit));
        planetModel = glm::scale(planetModel, glm::vec3(planet.radius, planet.radius, planet.radius));
        shader.setCommon(UNIFORM_MODEL, planetModel);
        shader.setVec3("diffuseColor", planet.color);
        Shapes::sphere->draw(shader);
    }
}

void System::queueDraw()
{
    sun->queueDraw();
    //for (auto& planet : planets) {
    //    //TODO queue instance draw for each planet
    //    //planet.queueDraw();
    //}
    // Instead use current method to draw planets
    Renderable::queueDraw();
}

void System::update(UpdateInfo& info)
{
    checkSetHoverCircle(info);

    float changeTime = 0.15; //seconds

    if (isHovered) {
        if (sun->color.b <= hoverColor.b) {
            sun->color += (hoverColor - baseColor) / changeTime * info.deltaTime;
        }
    } else {
        if (sun->color.b >= baseColor.b) {
            sun->color -= (hoverColor - baseColor) / changeTime * info.deltaTime;
        }
    }

    if (sun->color.b <= baseColor.b) {
        sun->color += (hoverColor - baseColor) / changeTime * info.deltaTime;
    }
}

glm::vec3 System::getPosition()
{
    return position;
}

void System::onClick()
{
    LOG_INFO << "System [" << gridx << ", " << gridy << "] clicked";
    shared_ptr<pair<int, int>> location(new pair(gridx, gridy));
    Event::triggerEvent(EVENT_SYSTEM_CLICK, location);
}

#define GRID_X 2
#define GRID_Y 2

SpaceGrid::SpaceGrid() : Renderable(SHADER_NONE)
{
    float distance = 30;
    for (int i = 0; i < GRID_X; i++) {
        for (int j = 0; j < GRID_Y; j++) {
            glm::vec3 position = glm::vec3(i * distance + j * distance / 2 , 0, j * distance);
            grid[i][j] = shared_ptr<System>(new System(position, i, j));
        }
    }
}

void SpaceGrid::draw(Shader& shader) {
    for (int i = 0; i < GRID_X; i++) {
        for (int j = 0; j < GRID_Y; j++) {
            grid[i][j]->draw(shader);
        }
    }
}

void SpaceGrid::queueDraw() {
    for (int i = 0; i < GRID_X; i++) {
        for (int j = 0; j < GRID_Y; j++) {
            grid[i][j]->queueDraw();
        }
    }
}

System* SpaceGrid::getSystem(int i, int j)
{
    return grid[i][j].get();;
}

vector<shared_ptr<Object>> SpaceGrid::getAllSystems()
{
    vector<shared_ptr<Object>> systems;
    systems.reserve(GRID_X * GRID_Y);
    for (int i = 0; i < GRID_X; i++) {
        for (int j = 0; j < GRID_Y; j++) {
            systems.push_back(grid[i][j]);
        }
    }
    return systems;
}

map<string, shared_ptr<Model>> SpaceShip::models;
shared_ptr<Model> SpaceShip::warpQuad;

map<string, string> MODEL_PATHS = {
    {"SS1", "./res/models/SS1_OBJ/SS1.obj"}
};

void SpaceShip::loadModel(string type)
{
    // Check if we've already loaded it
    if (models.find(type) == models.end()) {
        models[type] = shared_ptr<Model>(new Model(MODEL_PATHS[type].c_str()));
    }
    static bool sphereLoaded = false;
    if (not sphereLoaded) {
        warpQuad = shared_ptr<Model>(new Model("./res/models/quad/quad.obj"));
        sphereLoaded = true;
    }
}

SpaceShip::SpaceShip(string type, System* system) :
    Renderable(SHADER_LIGHTING),
    type(type), curSystem(system), prevSystem(system)
{
    this->type = type;
    loadModel(type);
    orbit.phase = rand_float_between(0, 2 * 3.14);
    orbit.inclination = rand_float_between(0, 3.14 / 3);
    orbit.radius = rand_float_between(1, 3);
    position = calcOrbitPosition(curSystem->getPosition(), orbit);
    stage = SHADER_LIGHTING | SHADER_WARP_STEP1;
}

float calculateWarp(glm::vec3 position, float margin, glm::vec3 start, glm::vec3 end)
{
    float fromStart = glm::length(position - start) - margin;
    float fromEnd = glm::length(position - end) - margin;
    float warp = 1;
    float maxWarpSpeed = 10;
    float warpAccelerationDist = 3;
    if (fromStart > 0 and fromEnd > 0) {
        if (fromStart < fromEnd) {
            warp += maxWarpSpeed * fromStart / warpAccelerationDist;
        } else {
            warp += maxWarpSpeed * fromEnd / warpAccelerationDist;
        }
    }
    return min(warp, maxWarpSpeed);
}

void SpaceShip::update(UpdateInfo& info)
{
    glm::vec3 targetPosition = calcOrbitPosition(curSystem->getPosition(), orbit);
    glm::vec3 targetDisplacement = targetPosition - position;
    glm::vec3 targetDirection = normalize(targetDisplacement);
    float targetAngle = glm::orientedAngle(direction, targetDirection, {0, 1, 0});

    // Limit turn speed
    if (abs(targetAngle) > turnSpeed * info.deltaTime) {
        float angle = turnSpeed * info.deltaTime * targetAngle / abs(targetAngle);
        direction = glm::rotate(glm::mat4(1.0f), angle, {0, 1, 0}) * glm::vec4(direction, 1);
        direction[1] = targetDirection[1]; // Do not limit pitch change speed
        direction = normalize(direction);
    } else {
        direction = targetDirection;
    }
    
    // Warp affects speed when far from planets
    warp = calculateWarp(position, 4.5, prevSystem->getPosition(), curSystem->getPosition());
    
    // Reduce speed when approching target position
    float decelDist = speed * 0.20;
    float curSpeed = speed;
    if (glm::length(targetDisplacement) < decelDist) {
        curSpeed = glm::length(targetDisplacement) / decelDist;
    }

    // Do not overshoot target position
    glm::vec3 displacement = direction * curSpeed * warp * info.deltaTime;
    if (glm::length(displacement) > glm::length(targetDisplacement)) {
        displacement = glm::length(targetDisplacement) * direction;
    }

    position += displacement;

    // Update camera pos for later use during drawWarp
    cameraPos = info.camera->Position;
}

glm::mat4 SpaceShip::calcModelMat() const
{
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::inverse(glm::lookAt(position, direction + position, {0, 1, 0}));
    model = glm::rotate(model, (float) 3.1415 / 2, glm::vec3(0, 1, 0));
    model = glm::scale(model, {length * warp, length, length});
    return model;
}

void SpaceShip::draw(Shader& shader)
{
    shader.setCommon(UNIFORM_MODEL, calcModelMat());
    models[type]->draw(shader);
}

void SpaceShip::drawWarp(Shader& shader)
{
    if (warp < 1.05) {
        return;
    }

    glm::mat4 transforms = calcModelMat();

    glm::vec3 modelCameraPos = glm::vec3(glm::inverse(transforms) * glm::vec4(cameraPos, 1));
    glm::mat4 model = glm::inverse(glm::lookAt({0, 0, 0}, modelCameraPos, {0, 1, 0}));
    
    float warpScale = (10 / ((warp-3)*(warp-3) + 1) + warp) * 0.2 * min(warp - 1, 1.0f);
    transforms = glm::scale(transforms, glm::vec3(warpScale));
    transforms = glm::scale(transforms, glm::vec3(1, 2, 2));

    shader.setCommon(UNIFORM_MODEL, transforms * model);
    warpQuad->draw(shader);
}


void SpaceShip::gotoSystem(System *system)
{
    if (curSystem != system) {
        prevSystem = curSystem;
        curSystem = system;
    }
}
