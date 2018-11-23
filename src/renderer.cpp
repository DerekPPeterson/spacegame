#include "renderer.h"
#include "timer.h"
#include "model.h"

using namespace std;

MeshRenderable createFramebufferQuad()
{
    float vertices[] = {
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
        };

    unsigned int indices[] = {0, 1, 2, 3, 4, 5};

    unsigned int VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), &indices[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void*) 0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, 
            (void*) (sizeof(float) * 2));
    
    return MeshRenderable(VAO, sizeof(indices) / sizeof(indices[0]));
};

void Renderer::compileLinkShaders()
{
    string shaderPath = "./src/shaders/";
    shaders.try_emplace(SHADER_LIGHTING, "src/shaders/vertex.vert", "src/shaders/lighting.frag");
    shaders.try_emplace(SHADER_SIMPLE_DIFFUSE, "src/shaders/vertex.vert", "src/shaders/simpleDiffuseLighting.frag");
    shaders.try_emplace(SHADER_BLEND, "src/shaders/framebuffer.vert", "src/shaders/blend.frag");
    shaders.try_emplace(SHADER_FRAMEBUFFER, "src/shaders/framebuffer.vert", "src/shaders/framebuffer.frag");
    shaders.try_emplace(SHADER_BLUR, "src/shaders/framebuffer.vert", "src/shaders/blur.frag");
    shaders.try_emplace(SHADER_LAMP, "src/shaders/lamp.vert", "src/shaders/lamp.frag");
    shaders.try_emplace(SHADER_SKYBOX, "./src/shaders/skybox.vert", "./src/shaders/skybox.frag");
    shaders.try_emplace(SHADER_WARP_STEP1, "./src/shaders/warp.vert", "./src/shaders/warp1.frag");
    shaders.try_emplace(SHADER_WARP_STEP2, "./src/shaders/warp.vert", "./src/shaders/warp2.frag");
}

Renderer::Renderer(RenderOptions options, Camera& camera) :
    options(options), camera(camera), 
    framebuffers(Framebuffers(options.screenWidth, options.screenHeight)),
    framebufferQuad(createFramebufferQuad())
{
    glViewport(0, 0, options.screenWidth, options.screenHeight);
    //TODO should more of these be options?
	projection = glm::perspectiveRH((float) glm::radians(45.0), 
            (float) options.screenWidth / options.screenHeight, 0.1f, 10000.0f);
    compileLinkShaders(); // TODO auto gen cpp code for shaders
    Timer::create("renderer_start");
}

void Renderer::start(function<void()> swapWindowBufferCallback)
{
    //TODO end condition /multithread
    while (true) {
        loop(swapWindowBufferCallback);
    }
}

void Renderer::stop() 
{
    //TODO fill in if using threads
}

void Renderer::renderMainScene()
{
    // Clear the warp framebuffer before drawing the rest of the scene
    // because they both write to the same brightcolor texture
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffers.warpFrameBuffer.id);
    glClear(GL_COLOR_BUFFER_BIT);

    // Actually render scene
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffers.mainFramebuffer.id);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);  
   
    glClearColor(0, 0, 0, 1);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    // TODO render main scene
    // TODO render skybox

    shaders[SHADER_SKYBOX].use();
    glm::mat4 skyboxView = glm::mat4(glm::mat3(camera.GetViewMatrix()));  
    shaders[SHADER_SKYBOX].setMat4("view", skyboxView);
    shaders[SHADER_SKYBOX].setMat4("projection", projection);
    for (auto r : getRenderablesForStage(SHADER_SKYBOX)) {
        r->draw(shaders[SHADER_SKYBOX]);
    }
}

void Renderer::renderWarpEffects()
{
    Shader warpShader1 = shaders[SHADER_WARP_STEP1];
    Shader warpShader2 = shaders[SHADER_WARP_STEP2];
    glDisable(GL_CULL_FACE);  
    glEnable(GL_BLEND);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffers.normalBlendFramebuffer.id);
    glClear(GL_COLOR_BUFFER_BIT);
    warpShader1.use();
    warpShader1.setMat4("view", camera.GetViewMatrix());
    warpShader1.setMat4("projection", projection);
    warpShader1.setVec2("screenSize", {options.screenWidth, options.screenHeight});
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, framebuffers.mainFramebuffer.colorTextures[0]);
    warpShader1.setInt("hdrBuffer", 1);

    // TODO draw warp effects
    
    //for (int i = 0; i < ships.size(); i++) {
    //    ships[i]->drawWarp(warpShader1, camera.Position);
    //}
    glDisable(GL_BLEND);

    // Copy over depth info so we don't render the warp effect over planets etc
    glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffers.mainFramebuffer.id);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffers.warpFrameBuffer.id);
    glBlitFramebuffer(
              0, 0, options.screenWidth, options.screenHeight, 0, 0, options.screenWidth, options.screenHeight, GL_DEPTH_BUFFER_BIT, GL_NEAREST
            );

    glEnable(GL_DEPTH_TEST);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffers.warpFrameBuffer.id);
    //glClear(GL_COLOR_BUFFER_BIT);
    warpShader2.use();
    warpShader2.setMat4("view", camera.GetViewMatrix());
    warpShader2.setMat4("projection", projection);
    warpShader2.setVec2("screenSize", {options.screenWidth, options.screenHeight});
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, framebuffers.mainFramebuffer.colorTextures[0]);
    warpShader2.setInt("hdrBuffer", 1);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, framebuffers.normalBlendFramebuffer.colorTextures[0]);
    warpShader2.setInt("normalAdjustBuffer", 2);
    //TODO draw warpeffects
    //for (int i = 0; i < ships.size(); i++) {
    //    ships[i]->drawWarp(warpShader2, camera.Position);
    //}
}

int Renderer::renderBloom()
{
    bool horizontal = true, first_iteration = true;
    int amount = 4;

    // Downscale bright image
    Shader framebufferShader = shaders[SHADER_FRAMEBUFFER];
    Shader blurShader = shaders[SHADER_BLUR];
    glViewport(0, 0, options.screenWidth / 2, options.screenHeight / 2);
    glDisable(GL_DEPTH_TEST);
    framebufferShader.use();
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffers.pingpongBuffers[!horizontal].id); 
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, framebuffers.mainFramebuffer.colorTextures[1]);
    framebufferShader.setInt("hdrBuffer", 0);
    //glGenerateMipmap(GL_TEXTURE_2D);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 1);
    framebufferQuad.draw(framebufferShader);

    // Blur bright areas for bloom
    blurShader.use();
    //float intensityChange = rand_float_between2(0, 2.0 * 3.141 * deltaTime);
    for (unsigned int i = 0; i < amount; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffers.pingpongBuffers[horizontal].id); 
        blurShader.setInt("horizontal", horizontal);
        glBindTexture(GL_TEXTURE_2D, 
                framebuffers.pingpongBuffers[!horizontal].colorTextures[0]
        ); 

        glActiveTexture(GL_TEXTURE0);
        framebufferQuad.draw(blurShader);

        horizontal = !horizontal;
    }
    return horizontal;
}

void Renderer::mergeEffects(int bloomOutputTextureNo)
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0); 
    glViewport(0, 0, options.screenWidth, options.screenHeight );
    //glDisable(GL_DEPTH_TEST);
    //glActiveTexture(GL_TEXTURE0);
    //glBindTexture(GL_TEXTURE_2D, framebuffers.warpFrameBuffer.colorTextures[0]);
    //glActiveTexture(GL_TEXTURE1);
    //glBindTexture(GL_TEXTURE_2D, framebuffers.pingpongBuffers[!horizontal].colorTextures[0]);
    //blendShader.use();
    //blendShader.setInt("hdrBuffer1", 1);
    //framebufferQuad.draw();
    
    glDisable(GL_DEPTH_TEST);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, framebuffers.mainFramebuffer.colorTextures[0]);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, framebuffers.pingpongBuffers[!bloomOutputTextureNo].colorTextures[0]);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, framebuffers.warpFrameBuffer.colorTextures[0]);

    Shader blendShader = shaders[SHADER_BLEND];
    blendShader.use();
    blendShader.setInt("hdrBuffer1", 1);
    blendShader.setInt("hdrBuffer2", 2);
    framebufferQuad.draw(blendShader);
}

void Renderer::loop(function<void()> swapWindowBufferCallback) 
{
    renderMainScene();
    renderWarpEffects();
    // TODO better way to pass this info?
    int buffno = renderBloom();
    mergeEffects(buffno);

    // TODO swap buffer function here?
    swapWindowBufferCallback();
}


std::vector<Renderable*> Renderer::getRenderablesForStage(ShaderEnum stage)
{
    vector<Renderable*> ret;
    for (auto r : toRender) {
        if (r->stage == stage) {
            ret.push_back(r);
        }
    }
    return ret;
}

