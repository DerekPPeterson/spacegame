#include "spaceThings.h"
#include "drawable.h"

#include <cstdlib>
#include <GLFW/glfw3.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>

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
        new_planet.orbitalRadius = rand_float_between(spacing, spacing + new_planet.radius * 3);
        spacing = new_planet.orbitalRadius + new_planet.radius * 2;
        new_planet.phase = rand_float_between(0, 2 * 3.14);
        new_planet.inclination = rand_float_between(0, 3.14 / 8);

        new_planet.color = glm::vec3(
                rand_float_between(0, 1),
                rand_float_between(0, 1),
                rand_float_between(0, 1)
                );

        planets.push_back(new_planet);
    }
}

void System::draw(Shader shader) 
{
    glm::mat4 systemModel = glm::translate(glm::mat4(1.0f), position);
    for (auto planet : planets) {
        glm::vec3 orbitAxis = glm::rotateX(glm::vec3(0, 1, 0), planet.inclination);
        glm::mat4 planetModel = glm::rotate(systemModel, 
                 1.0f / (float) pow(planet.orbitalRadius , 3.0/2.0) * (float) glfwGetTime(), 
                orbitAxis);
        planetModel = glm::translate(planetModel, glm::vec3(planet.orbitalRadius, 0, 0));
        planetModel = glm::scale(planetModel, glm::vec3(planet.radius, planet.radius, planet.radius));
        shader.setMat4("model", planetModel);
        shader.setInt("useTextures", 0);
        shader.setVec3("diffuseColor", planet.color);
        sphere.draw(shader);
        shader.setVec3("diffuseColor", glm::vec3(0, 0, 0));
        shader.setInt("useTextures", 1);
    }
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
