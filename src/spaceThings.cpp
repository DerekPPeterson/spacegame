#include "spaceThings.h"
#include "drawable.h"
#include "timer.h"
#include "util.h"
#include "event.h"
#include "camera2.h"

#include <cstdlib>
#include <algorithm>

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
             1.0f / (float) pow(orbit.radius , 3.0/2.0) * Timer::global.get(), 
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
    Event::triggerEvent(EVENT_SYSTEM_CLICK, logicId);
}

#define GRID_X SPACEGRID_SIZE
#define GRID_Y SPACEGRID_SIZE

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


LaserShot::LaserShot(glm::vec3 position, weak_ptr<has_position> target, 
        float speed, glm::vec3 color)
    : Renderable(SHADER_NONE), target(target), speed(speed), color(color)
{
    setPos(position);
    setVisible(true);
}

void LaserShot::update(UpdateInfo& info)
{
    auto targetShared = target.lock();
    if (targetShared) {
        auto moveVector = glm::normalize(targetShared->getPos() - position) * speed * info.deltaTime;
        auto distLeft = glm::length(targetShared->getPos() - position);
        if (glm::length(moveVector) > distLeft) {
            removeThis = true;
        } else {
            setPos(getPos() + moveVector);
        }
    } else {
        removeThis = true;
    }

}

void LaserShot::queueDraw()
{
    Particles::queueParticleDraw(getPos(), color);
}

Explosion::Explosion(glm::vec3 position, float size, float duration)
    : Renderable(SHADER_LAMP), size(size), duration(duration)
{
    this->position = position;
    int nParticles = 200;
    for (int i = 0; i < nParticles; i++) {
            particles.addParticle(position, randWithinSphere(3) * size, 
                    glm::vec3(
                        0.866 * rand_float_between(10, 20),
                        0.533 * rand_float_between(10, 20),
                        0.172 * rand_float_between(10, 20)
                        ));
    }
}

void Explosion::queueDraw()
{
    particles.queueDraw();
}

void Explosion::update(UpdateInfo& info)
{
    particles.update(info);
    if (not startTime) {
        startTime = info.curTime;
    }
    float timeLeft = startTime + duration - info.curTime;

    if (timeLeft < 0) {
        removeThis = true;
    } else if (timeLeft < duration / 2) {
        particles.colorMultiplier = timeLeft * 2 /  duration;
    }
}


map<string, shared_ptr<Model>> SpaceShip::models;
shared_ptr<Model> SpaceShip::warpQuad;

// TODO get different models for these
map<string, string> MODEL_PATHS = {
    {"SS1", "./res/models/SS1_OBJ/SS1.obj"},
    {"Default Flagship", "./res/models/SS1_OBJ/SS1.obj"},
    {"Mining Platform", "./res/models/SS1_OBJ/SS1.obj"},
    {"AI Coreship", "./res/models/SS1_OBJ/SS1.obj"},
    {"AM Gatherer", "./res/models/SS1_OBJ/SS1.obj"},
    {"Diplomatic Vessel", "./res/models/SS1_OBJ/SS1.obj"},
};

std::shared_ptr<SpaceShip> SpaceShip::createFrom(logic::Ship logicShip, System* s)
{
    auto ship = shared_ptr<SpaceShip>(new SpaceShip(logicShip.type, s));
    ship->logicId = logicShip.id;
    ship->logicShipInfo = logicShip;
    return ship;
}

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

void SpaceShip::setup()
{
    for (auto& [type, path] : MODEL_PATHS) {
        if (models.find(type) == models.end()) {
            models[type] = shared_ptr<Model>(new Model(MODEL_PATHS[type].c_str()));
        }
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
    glm::vec3 targetPosition;
    glm::vec3 targetDisplacement = targetPosition - position;
    glm::vec3 targetDirection = normalize(targetDisplacement);

    auto shootAtShared = shootAt.lock();
    if (shoot and shootAtShared) {
        targetPosition = shootAtShared->getPos();
        targetDisplacement = targetPosition - position;
        targetDirection = normalize(targetDisplacement);
        if (glm::length(targetDisplacement) < 0.5) {
            targetDirection = glm::rotate(glm::mat4(1.0f), 3.14f / 2, {0, 1, 0}) * glm::vec4(targetDirection, 1);
        }
    } else {
        targetPosition = calcOrbitPosition(curSystem->getPosition(), orbit);
        targetDisplacement = targetPosition - position;
        targetDirection = normalize(targetDisplacement);
    }

    bool inRange = glm::length(targetDisplacement) < range;


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
    if (shoot and inRange) {
        warp = 1;
    } else {
        warp = calculateWarp(position, 4.5, prevSystem->getPosition(), curSystem->getPosition());
    }
    
    // Reduce speed when approching target position
    float decelDist = speed * 0.20;
    float curSpeed = speed;
    if (glm::length(targetDisplacement) < decelDist and not shoot) {
        curSpeed = glm::length(targetDisplacement) / decelDist;
    }

    // Do not overshoot target position
    glm::vec3 displacement = direction * curSpeed * warp * info.deltaTime;
    if (glm::length(displacement) > glm::length(targetDisplacement)) {
        displacement = glm::length(targetDisplacement) * direction;
    }

    position += displacement;

    if (shoot and info.curTime > lastShot + shootDelay and warp < 1.05 and shootAtShared) {
        auto shot = make_shared<LaserShot>(position, shootAt, 10, glm::vec3(100, 0, 0));
        emit.push_back(shot);
        lastShot = info.curTime;
    }

    // Update camera pos for later use during drawWarp
    cameraPos = info.camera->getPos();
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

void SpaceShip::destroy()
{
    emit.push_back(make_shared<Explosion>(getPos(), length*4, 2));
    removeThis = true;
}

void SpaceShip::startShootingAt(std::shared_ptr<has_position> shootAt)
{
    shoot = true;
    this->shootAt = shootAt;
}

void SpaceShip::stopShooting()
{
    shoot = false;
}

