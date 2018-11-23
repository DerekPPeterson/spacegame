#ifndef RENDERER_H
#define RENDERER_H

#include <string>
#include <unordered_map>
#include <functional>

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

/** 
 * Each enum refers to a particular shader
 * Also used as an indication in a RenderableInstance as to when it should
 * be rendered
 */
enum ShaderEnum {
    SHADER_LIGHTING, // Main scene
    SHADER_SIMPLE_DIFFUSE,  // Main scene
    SHADER_BLEND,
    SHADER_FRAMEBUFFER,
    SHADER_BLUR,
    SHADER_LAMP,            // Main scene
    SHADER_SKYBOX,
    SHADER_WARP_STEP1,      // During warp step
    SHADER_WARP_STEP2
};


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
        void start(std::function<void()>);
        void stop();

    private:
        RenderOptions options;
        Camera& camera;
        Framebuffers framebuffers;
        glm::mat4 projection;
        std::unordered_map<ShaderEnum, Shader> shaders;
        // TODO remove this for a Renderable
        MeshRenderable framebufferQuad;

        void compileLinkShaders();
        void loop(std::function<void()>);

        void renderMainScene();
        void renderWarpEffects(glm::mat4 view, glm::mat4 projection);
        int renderBloom();
        void mergeEffects(int);
};

#endif 
