#ifndef RENDERERABLES_H
#define RENDERERABLES_H

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <memory>
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
    SHADER_CARD           = 1 << 10,
};

class Renderable;

// Map shader type -> queue draw function -> pointers to renderables to draw
typedef std::unordered_map<int, 
        std::unordered_map<void (*)(ShaderEnum, Shader&, std::vector<Renderable*>), 
            std::vector<Renderable*>>> RenderableQueues;

class Renderable
{
    public:
        Renderable() {};
        virtual void queueDraw();
        virtual void draw(Shader& shader) {};
        virtual void drawWarp(Shader& shader) {};
        static void drawStage(ShaderEnum stage, Shader& shader);
        virtual void setStage(ShaderEnum stage) {this->stage = stage;};
        unsigned int stage = SHADER_NONE;
    private:
        static RenderableQueues queues;

        static void drawQueue(ShaderEnum drawingStage, Shader& shader, std::vector<Renderable*> queue);

        friend class WarpRenderable;
};

class MeshRenderable : virtual public Renderable
{
    public:
        MeshRenderable() {};
        MeshRenderable(unsigned int VAO, unsigned int nIndices) : 
            VAO(VAO), nIndices(nIndices) {};
        virtual void draw(Shader& shader) override;
    protected:
        unsigned int VAO;
        unsigned int nIndices;
};

#endif 
