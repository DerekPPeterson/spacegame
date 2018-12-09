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
#include "model.h"

//class SkyboxRenderable : virtual public Renderable
//{
//    public:
//        SkyboxRenderable(std::unique_ptr<Renderable> cube, unsigned int textureId) :
//            cube(std::move(cube)), textureId(textureId) {};
//        virtual void draw() override;
//    private:
//        unsigned int textureId;
//        std::unique_ptr<Renderable> cube;
//};

class Skybox: public Renderable
{
    public:
		unsigned int textureId;

        Skybox(std::string path) :
            Renderable(SHADER_SKYBOX),
            cubeModel(Model("./res/models/cube.obj"))
        {
            setupSkybox(path);
        }
        void draw(Shader& shader) override;
        
    private:
        Model cubeModel;
        void setupSkybox(std::string path);
};



#endif
