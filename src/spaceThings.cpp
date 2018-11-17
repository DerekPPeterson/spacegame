#include "spaceThings.h"
#include "drawable.h"

System::System(glm::vec3 position)
{
    this->position = position;
    sun = Light::makeLight(LIGHT_POINT, position, glm::vec3(100, 100, 100));
}

void System::draw(Shader shader) 
{
}

SpaceGrid::SpaceGrid()
{
    float distance = 50;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            grid[i][j] = System(glm::vec3(i * distance + j * distance / 2 , 0, j * distance));
        }
    }
}

void SpaceGrid::draw(Shader shader) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            grid[i][j].draw(shader);
        }
    }
}
