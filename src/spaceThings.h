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
#include "logic.h"
#include "particles.h"

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

glm::vec3 calcOrbitPosition(glm::vec3 systemPosition, Orbit &orbit);

class System : public Object , public Selectable, public Renderable{
    public:
        System(glm::vec3 position, int gridx, int gridy);
        virtual void draw(Shader& shader) override;
        virtual void queueDraw() override;
        virtual void update(UpdateInfo& info) override;
        glm::vec3 getPosition();
        int gridx, gridy;

    protected:
        void onClick() override;
        std::shared_ptr<PointLight> sun;
        //glm::vec3 position;
        std::vector<Planet> planets;
        glm::vec3 baseColor = glm::vec3(5, 5, 5);
        glm::vec3 hoverColor = glm::vec3(0, 15, 30);
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
        std::shared_ptr<System> grid[SPACEGRID_SIZE][SPACEGRID_SIZE];
};

class LaserShot : public Object, public Renderable, has_position
{
    public:
        LaserShot(glm::vec3 position, std::weak_ptr<has_position> target, 
                float speed, glm::vec3 color);
        virtual void queueDraw() override;
        virtual void update(UpdateInfo& info) override;

    protected:
        std::weak_ptr<has_position> target;
        float speed;
        glm::vec3 color;
};

class Explosion : public Object, public Renderable, public has_position
{
    public:
        Explosion(glm::vec3 position, float size, float duration);
        virtual void queueDraw() override;
        virtual void update(UpdateInfo& info) override;
    protected:
        float size;
        float startTime = 0;
        float duration;
        ParticleGroup particles;
};

extern std::map<std::string, std::shared_ptr<Model>> shipModels;

// TODO use an instance renderer
class SpaceShip : public Object, public Renderable, public has_position
{
    public:
        SpaceShip(std::string type, System *system);
        virtual void draw(Shader& shader) override;
        virtual void drawWarp(Shader& shader) override;
        void gotoSystem(System *system);
        virtual void update(UpdateInfo& info) override;

        static std::shared_ptr<SpaceShip> createFrom(logic::Ship logicShip, System *s);
        int getCurSystemId() {return curSystem->logicId;};
        void destroy();
        logic::Ship logicShipInfo;

        void startShootingAt(std::shared_ptr<has_position> shootAt);
        void stopShooting();

    protected:
        std::string type;
        float length = 0.1;
        System* curSystem;
        System* prevSystem;
        Orbit orbit;
        glm::vec3 cameraPos;

        // +xaxis of model will be front of ship
        glm::vec3 direction = {1, 0, 0};
        float speed = 4;
        float warp = 1;
        float turnSpeed = 2 * 3.14;

        bool shoot = false;
        float lastShot = 0;
        float shootDelay = 0.2;
        float range = 1;
        std::weak_ptr<has_position> shootAt;

        glm::mat4 calcModelMat() const;
        static void loadModel(std::string type);
        static std::shared_ptr<Model> warpQuad;
        std::shared_ptr<Model> meshModel;
};

#endif
