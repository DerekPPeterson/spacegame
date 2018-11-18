#include "spaceThings.h"
#include "drawable.h"
#include "timer.h"

#include <cstdlib>
#include <GLFW/glfw3.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>

float rand_float_between(float LO, float HI)
{
    return LO + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(HI-LO)));
}

System::System(glm::vec3 position)
{ if (not isSetup) {
        setup();
    };

    this->position = position;
    sun = Light::makeLight(LIGHT_POINT, position, glm::vec3(10, 10, 10));

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

void System::draw(Shader shader) 
{
    for (auto planet : planets) {
        glm::mat4 planetModel = glm::translate(glm::mat4(1.0f), 
                calcOrbitPosition(position, planet.orbit));
        planetModel = glm::scale(planetModel, glm::vec3(planet.radius, planet.radius, planet.radius));
        shader.setMat4("model", planetModel);
        shader.setVec3("diffuseColor", planet.color);
        sphere.draw(shader);
    }
}

glm::vec3 System::getPosition()
{
    return position;
}

// static SpaceGrid definitions
bool System::isSetup = false;
Model System::sphere;

void System::setup()
{
    sphere = Model("./res/models/sphere.obj");
}

#define GRID_X 4
#define GRID_Y 4

SpaceGrid::SpaceGrid()
{
    float distance = 30;
    for (int i = 0; i < GRID_X; i++) {
        for (int j = 0; j < GRID_Y; j++) {
            grid[i][j] = System(glm::vec3(i * distance + j * distance / 2 , 0, j * distance));
        }
    }

}

void SpaceGrid::draw(Shader shader) {
    for (int i = 0; i < GRID_X; i++) {
        for (int j = 0; j < GRID_Y; j++) {
            grid[i][j].draw(shader);
        }
    }
}

System& SpaceGrid::getSystem(int i, int j)
{
    return grid[i][j];
}

map<string, Model> SpaceShip::models;

map<string, string> MODEL_PATHS = {
    {"SS1", "./res/models/SS1_OBJ/SS1.obj"}
};

void SpaceShip::loadModel(string type)
{
    // Check if we've already loaded it
    if (models.find(type) == models.end()) {
        models[type] = Model(MODEL_PATHS[type].c_str());
    }
}

SpaceShip::SpaceShip(string type, System& system) :
    type(type), curSystem(system)
{
    this->type = type;
    loadModel(type);
    position = calcOrbitPosition(curSystem.getPosition(), orbit);
    orbit.phase = rand_float_between(0, 2 * 3.14);
    orbit.inclination = rand_float_between(0, 3.14 / 3);
    orbit.radius = rand_float_between(1, 3);
}

void SpaceShip::update(float deltaTime)
{
    glm::vec3 targetPosition = calcOrbitPosition(curSystem.getPosition(), orbit);
    glm::vec3 targetDisplacement = targetPosition - position;
    glm::vec3 targetDirection = normalize(targetDisplacement);
    float targetAngle = glm::orientedAngle(direction, targetDirection, {0, 1, 0});
    if (abs(targetAngle) > turnSpeed * deltaTime) {
        float angle = turnSpeed * deltaTime * targetAngle / abs(targetAngle);
        direction = glm::rotate(glm::mat4(1.0f), angle, {0, 1, 0}) * glm::vec4(direction, 1);
        direction[1] = targetDirection[1];
        direction = normalize(direction);
    } else {
        direction = targetDirection;
    }

    glm::vec3 displacement = direction * speed * deltaTime;
    if (glm::length(displacement) > glm::length(targetDisplacement)) {
        displacement = glm::length(targetDisplacement) * direction;
    }
    position += displacement;
}

void SpaceShip::draw(Shader shader)
{
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::inverse(glm::lookAt(position, direction + position, {0, 1, 0}));
    model = glm::rotate(model, (float) 3.1415 / 2, glm::vec3(0, 1, 0));
    model = glm::scale(model, {length, length, length}), 

    shader.setMat4("model", model);
    models[type].draw(shader);
}

void SpaceShip::gotoSystem(System& system)
{
    curSystem = system;
}
