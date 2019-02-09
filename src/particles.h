#ifndef PARTICLES_H
#define PARTICLES_H

#include "renderables.h"
#include <memory>
#include "drawable.h"

class Particles : public needs_setup<Particles>
{
    static std::shared_ptr<InstanceMeshRenderable> particle;

    public:
    static void setup() {

        LOG_INFO << "Setting up particles";

        float vertices[] = {0, 0, 0};
        unsigned int indices[] = {0};

        // Only need to create VAO once
        unsigned int VAO, VBO, EBO;
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), &indices[0], GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*) 0);


        std::vector<unsigned int> noTextureIds = {};
        std::vector<InstanceAttribName> attribs = {
            INSTANCE_ATTRIB_MAT4,
            INSTANCE_ATTRIB_VEC3,
        };
        particle = std::make_shared<InstanceMeshRenderable>(
                SHADER_PARTICLE,
                VAO, sizeof(indices) / sizeof(indices[0]),
                attribs,
                noTextureIds,
                GL_POINTS
        );
    }

    static void queueParticleDraw(glm::vec3 position, glm::vec3 color)
    {
        struct ParticleInstanceData
        {
            glm::mat4 model;
            glm::vec3 color;
        } data;
        data.model = glm::translate(glm::mat4(1), position);
        data.color = color;
        particle->addInstance(&data);
        particle->queueDraw();
    }
};


#endif
