#ifndef CUBEMAP_H
#define CUBEMAP_H

#include <vector>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"

class Skybox
{
    public:
		unsigned int textureId;

        Skybox(std::string path)
        {
            setupSkybox(path);
        }
        void draw(Shader shader);

    private:
        unsigned int VAO, VBO;
        void setupSkybox(std::string path);
};



#endif
