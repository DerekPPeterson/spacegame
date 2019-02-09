#include "particles.h"

std::shared_ptr<InstanceMeshRenderable> Particles::particle;
volatile Particles particles; // need an instance of Particles to run setup
