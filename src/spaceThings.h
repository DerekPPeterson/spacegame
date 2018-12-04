#ifndef SPACETHINGS_H
#define SPACETHINGS_H

#include "drawable.h"

#include <vector>
#include <map>
#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "renderables.h"

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


class System : public Object , public Selectable, public Renderable{
    public:
        System() {};
        virtual ~System() {};
        System(glm::vec3 position);
        virtual void draw(Shader& shader) override;
        virtual void queueDraw() override;
        virtual void update(UpdateInfo& info) override;
        glm::vec3 getPosition();

    protected:
        std::shared_ptr<PointLight> sun;
        //glm::vec3 position;
        std::vector<Planet> planets;
        glm::vec3 baseColor = glm::vec3(10, 10, 10);
        glm::vec3 hoverColor = glm::vec3(0, 30, 40);

        static Model sphere;
        static bool isSetup;
        void setup();
};

class SpaceGrid : public Object, public Renderable
{
    public:
        SpaceGrid();
        void draw(Shader& shader) override;
        void queueDraw() override;
        System* getSystem(int i, int j);
        std::vector<std::shared_ptr<Object>> getAllSystems();
    private:
        std::shared_ptr<System> grid[4][4];
};


// TODO use an instance renderer
class SpaceShip : public Object, public Renderable
{
    public:
        SpaceShip(std::string type, System *system);
        virtual void draw(Shader& shader) override;
        virtual void drawWarp(Shader& shader) override;
        void gotoSystem(System *system);
        virtual void update(UpdateInfo& info) override;
    
    protected:
        std::string type;
        float length = 0.1;
        glm::vec3 position;
        System* curSystem;
        System* prevSystem;
        Orbit orbit;
        glm::vec3 cameraPos;

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
