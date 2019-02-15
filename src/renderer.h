#ifndef RENDERER_H
#define RENDERER_H

#include <string>
#include <unordered_map>
#include <functional>
#include <thread>
#include <mutex>

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

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

class Camera;

class Renderer
{
    public:
        Renderer(RenderOptions options, Camera& camera);
        void renderFrame();
        void addRenderable(std::shared_ptr<Renderable>);
        void setToRender(std::vector<std::shared_ptr<Renderable>>);
        // TODO best interface for this?
        glm::mat4 getProjection() {return projection;};

    private:
        RenderOptions options;
        Camera& camera;
        Framebuffers framebuffers;
        glm::mat4 projection;
        std::unordered_map<ShaderEnum, Shader> shaders;
        std::vector<std::shared_ptr<Renderable>> toRender;
        std::vector<std::shared_ptr<Renderable>> alwaysRender;
        std::mutex toRenderMutex;

        void compileLinkShaders();

        void renderMainScene();
        void renderWarpEffects();
        int renderBloom();
        void mergeEffects(int);
};

#endif 
