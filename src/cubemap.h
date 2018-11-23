#ifndef CUBEMAP_H
#define CUBEMAP_H

#include <vector>
#include <string>
#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"
#include "nocopy.h"
#include "renderables.h"

class SkyboxRenderable : public Renderable
{
    public:
        SkyboxRenderable(unsigned int VAO, unsigned int textureId) :
            VAO(VAO), textureId(textureId) {};
        virtual void draw() override;
    private:
        unsigned int VAO, textureId;
};

class Skybox  
{
    public:
		unsigned int textureId;

        Skybox(std::string path)
        {
            setupSkybox(path);
        }
        void draw(Shader shader);
        std::unique_ptr<Renderable*> getRenderable();
        

    private:
        unsigned int VAO, VBO;
        void setupSkybox(std::string path);
};



#endif
