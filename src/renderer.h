#ifndef RENDERER_H
#define RENDERER_H

#include <string>
#include <unordered_map>
#include <functional>
#include <thread>
#include <mutex>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "camera.h"
#include "framebuffer.h"
#include "shader.h"
//#include "drawable.h"
//#include "model.h"
#include "renderables.h"

struct RenderOptions
{
    int screenWidth = 1000;
    int screenHeight = 800;
    bool fullscreen = false;
};

class Renderer
{
    public:
        Renderer(RenderOptions options, Camera& camera);
        void renderFrame();
        void addRenderable(Renderable* r);
        // TODO best interface for this?
        glm::mat4 getProjection() {return projection;};

    private:
        RenderOptions options;
        Camera& camera;
        Framebuffers framebuffers;
        glm::mat4 projection;
        std::unordered_map<ShaderEnum, Shader> shaders;
        MeshRenderable framebufferQuad;
        std::vector<Renderable*> toRender;
        std::mutex toRenderMutex;

        void compileLinkShaders();

        void renderMainScene();
        void renderWarpEffects();
        int renderBloom();
        void mergeEffects(int);
        std::vector<Renderable*> getRenderablesForStage(ShaderEnum stage);
};

#endif 
