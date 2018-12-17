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
    SHADER_WARP_STEP1     = 1 << 8,  // During postprocess step
    SHADER_WARP_STEP2     = 1 << 9, 
    SHADER_CARD           = 1 << 10,
    SHADER_TEXT           = 1 << 11, // End of main scene, no view matrix
    SHADER_UI_LIGHTING    = 1 << 12, // End of main scene, no view matrix
    SHADER_ANTI           = 1 << 13, // During postprocess step
};

class Renderable;
typedef void (*DrawQueuesType)(ShaderEnum, Shader&, std::vector<Renderable*>);

// Map shader type -> queue draw function -> pointers to renderables to draw
typedef std::unordered_map<int, 
            std::unordered_map<DrawQueuesType, 
                std::vector<Renderable*>>> RenderableQueues;

class Renderable
{
    public:
        Renderable(unsigned int stage) : stage(stage) {};
        virtual ~Renderable() {};
        virtual void queueDraw();
        virtual void draw(Shader& shader) {};
        virtual void drawWarp(Shader& shader) {};
        static void drawStage(ShaderEnum stage, Shader& shader);
        virtual void setStage(ShaderEnum stage) {this->stage = stage;};
        unsigned int stage = SHADER_NONE;
        bool isVisible() {return visible;}
        void setVisible(bool visible) {this->visible = visible;}

    private:
        static void drawQueue(ShaderEnum drawingStage, Shader& shader, std::vector<Renderable*> queue);

    protected:
        bool visible = true;
        static RenderableQueues queues;
        virtual DrawQueuesType getDrawQueuesFunc() {return drawQueue;};

};

class MeshRenderable : public Renderable
{
    public:
        MeshRenderable(ShaderEnum stage, unsigned int VAO, unsigned int nIndices) : 
            Renderable(stage), VAO(VAO), nIndices(nIndices) {};
        virtual void draw(Shader& shader) override;
    protected:
        unsigned int VAO;
        unsigned int nIndices;
};

enum InstanceAttribName {
    INSTANCE_ATTRIB_VEC4,
    INSTANCE_ATTRIB_VEC3,
    INSTANCE_ATTRIB_VEC2,
    INSTANCE_ATTRIB_MAT4,
};

class InstanceMeshRenderable : public MeshRenderable
{
    public:
        InstanceMeshRenderable(ShaderEnum stage, unsigned int VAO, unsigned int nIndices,
                std::vector<InstanceAttribName>, 
                std::vector<unsigned int> textureIds = {});
        virtual void draw(Shader& shader) override;
        virtual void queueDraw() override;

        /* Load data into a buffer for use by the vertex shader.
         * Creates a new buffer if buf == 0
         */
        void loadBufferData(unsigned int *buf, void* data, int size);
        
        /* Set the vertex attributes to use a buffer.
         * The relevant buffer must be bound
         * startAt is the vertex attribute to start at
         */
        void setVertexAttribPointersVec4(int startAt, long offset);
        void setVertexAttribPointersVec3(int startAt, long offset);
        void setVertexAttribPointersVec2(int startAt, long offset);
        void setVertexAttribPointersMat4(int startAt, long offset);

        void addInstance(void *data);

    protected:
        int maxInstances = 1000;

        std::vector<unsigned int> textureIds;

        std::vector<char> buf;
        unsigned int bufId;
        unsigned int dataSize = 0;
        unsigned int curInstance = 0;
        bool queued = false;
};

#endif 
