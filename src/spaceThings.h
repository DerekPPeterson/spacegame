#ifndef SPACETHINGS_H
#define SPACETHINGS_H

#include "drawable.h"

#include <vector>
#include <map>
#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

typedef struct Orbit {
    float radius;
    float phase;
    float inclination;
} Orbit;

typedef struct Planet {
    glm::vec3 color;
    float radius;
    Orbit orbit;
} Planet;


class System : public Object
{
    public:
        System() {};
        System(glm::vec3 position);
        void draw(Shader shader);
        glm::vec3 getPosition();

    private:
        std::shared_ptr<Light> sun;
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
        System* getSystem(int i, int j);
    private:
        std::shared_ptr<System> grid[4][4];
};


class SpaceShip : public Object
{
    public:
        SpaceShip(std::string type, System *system);
        void draw(Shader shader) const override;
        void drawWarp(Shader shader, glm::vec3 cameraPos);
        void gotoSystem(System *system);
        void update(float deltaTime);
    
    private:
        std::string type;
        float length = 0.1;
        glm::vec3 position;
        System* curSystem;
        System* prevSystem;
        Orbit orbit;

        // +xaxis of model will be front of ship
        glm::vec3 direction = {1, 0, 0};
        float speed = 2;
        float warp = 1;
        float turnSpeed = 2 * 3.14;

        glm::mat4 calcModelMat() const;
        static void loadModel(std::string type);
        static std::map<std::string, Model> models;
        static Model sphere;
};

#endif
