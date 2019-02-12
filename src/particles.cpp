#include "particles.h"

std::shared_ptr<InstanceMeshRenderable> Particles::particle;
volatile Particles particles; // need an instance of Particles to run setup

void ParticleGroup::addParticle(glm::vec3 position, glm::vec3 velocity, glm::vec3 color)
{
    positions.push_back(position);
    velocities.push_back(velocity);
    colors.push_back(color);
}

void ParticleGroup::queueDraw() 
{
    for (int i = 0; i < positions.size(); i++) {
        Particles::queueParticleDraw(positions[i], colors[i] * colorMultiplier);
    }
}

void ParticleGroup::update(UpdateInfo& info) 
{
    for (int i = 0; i < positions.size(); i++) {
        positions[i] += velocities[i] * info.deltaTime;
    }
}


