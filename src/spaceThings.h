#ifndef SPACETHINGS_H
#define SPACETHINGS_H

#include "drawable.h"

#include <vector>
#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

typedef struct Planet {
    glm::vec3 color;
    float radius;
    float orbitalRadius;
    float phase;
    float inclination;
} Planet;

class System : public Object
{
    public:
        System() {};
        System(glm::vec3 position);
        void draw(Shader shader);

    private:
        shared_ptr<Light> sun;
        glm::vec3 position;
        std::vector<Planet> planets;
        static Model sphere;
        static bool isSetup;
        void setup();
};

class SpaceGrid : public Object
{
    public:
        SpaceGrid();
        void draw(Shader shader);
    private:
        System grid[4][4];
};

#endif
