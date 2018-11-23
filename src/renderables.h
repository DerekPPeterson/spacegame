#ifndef RENDERERABLES_H
#define RENDERERABLES_H

#include <vector>
#include <unordered_map>
#include "shader.h"

/** 
 * Each enum refers to a particular shader
 * Also used as an indication in a RenderableInstance as to when it should
 * be rendered for renderables produced by objects
 */
enum ShaderEnum {
    SHADER_LIGHTING, // Main scene
    SHADER_SIMPLE_DIFFUSE,  // Main scene
    SHADER_BLEND,
    SHADER_FRAMEBUFFER,
    SHADER_BLUR,
    SHADER_LAMP,            // Main scene
    SHADER_SKYBOX,          // Main scene
    SHADER_WARP_STEP1,      // During warp step
    SHADER_WARP_STEP2
};

class Renderable
{
    public:
        virtual void queueDraw();
        virtual void draw(Shader& shader) {};
        ShaderEnum stage = SHADER_LIGHTING;
    private:
        static std::unordered_map<void (*)(Shader&, std::vector<Renderable>), 
            std::vector<Renderable>> drawQueues;

        static void drawQueue(Shader& shader, std::vector<Renderable> queue);
        static void drawAllQueues(Shader& shader);
};

class MeshRenderable : virtual public Renderable
{
    public:
        MeshRenderable(unsigned int VAO, unsigned int nIndices) : 
            VAO(VAO), nIndices(nIndices) {};
        virtual void draw(Shader& shader) override;
    protected:
        unsigned int VAO;
        unsigned int nIndices;
};

#endif 
