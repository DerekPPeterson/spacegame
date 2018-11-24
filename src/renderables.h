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
    SHADER_NONE           = 0,
    SHADER_LIGHTING       = 1 << 1,  // Main scene
    SHADER_SIMPLE_DIFFUSE = 1 << 2,  // Main scene
    SHADER_BLEND          = 1 << 3, 
    SHADER_FRAMEBUFFER    = 1 << 4, 
    SHADER_BLUR           = 1 << 5, 
    SHADER_LAMP           = 1 << 6,  // Main scene
    SHADER_SKYBOX         = 1 << 7,  // Main scene
    SHADER_WARP_STEP1     = 1 << 8,  // During warp step
    SHADER_WARP_STEP2     = 1 << 9, 
};

class Renderable;

// Map shader type -> queue draw function -> pointers to renderables to draw
typedef std::unordered_map<ShaderEnum, 
        std::unordered_map<void (*)(Shader&, std::vector<Renderable*>), 
            std::vector<Renderable*>>> RenderableQueues;

class Renderable
{
    public:
        virtual void queueDraw();
        virtual void draw(Shader& shader) {};
        static void drawStage(ShaderEnum stage, Shader& shader);
        ShaderEnum stage = SHADER_NONE;
    private:
        static RenderableQueues queues;

        static void drawQueue(Shader& shader, std::vector<Renderable*> queue);
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
