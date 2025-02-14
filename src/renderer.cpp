#include "renderer.h"
#include "timer.h"
#include "model.h"

// TODO change drawable name
#include "drawable.h"
#include "camera2.h"


using namespace std;

glm::vec3 calcUiLightDir(float curTime)
{
    glm::vec3 dir = {1, 1, 0};

    return dir * (float) (fmod(curTime, 10) - 5);
}


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
    shaders.try_emplace(SHADER_ANTI, "./src/shaders/warp.vert", "./src/shaders/anti.frag");
    shaders.try_emplace(SHADER_CARD, "./src/shaders/card.vert", "./src/shaders/card.frag");
    shaders.try_emplace(SHADER_TEXT, "./src/shaders/text.vert", "./src/shaders/text.frag");
    shaders.try_emplace(SHADER_UI_LIGHTING, "./src/shaders/vertex.vert", "./src/shaders/uilighting.frag");
    shaders.try_emplace(SHADER_PARTICLE, "./src/shaders/particle.vert", "./src/shaders/particle.frag");
    shaders.try_emplace(SHADER_UI_LIGHTING_CARD_IMAGE, "./src/shaders/vertex.vert", "./src/shaders/uilighting.frag");
    shaders.try_emplace(SHADER_CARD_BG, "./src/shaders/cardbg.vert", "./src/shaders/cardbg.frag");
    shaders.try_emplace(SHADER_STENCIL, "./src/shaders/stencil.vert", "./src/shaders/stencil.frag");
}

Renderer::Renderer(RenderOptions options, Camera& camera) :
    options(options), camera(camera), 
    framebuffers(Framebuffers(options.screenWidth, options.screenHeight))
{
    glViewport(0, 0, options.screenWidth, options.screenHeight);
    //TODO should more of these be options?
	projection = glm::perspectiveRH((float) glm::radians(45.0), 
            (float) options.screenWidth / options.screenHeight, 0.1f, 10000.0f);
    compileLinkShaders(); // TODO auto gen cpp code for shaders

    // Enable some options for good line drawing
    glEnable(GL_LINE_SMOOTH);
    glLineWidth(1);

    UIObject::setViewInfo(options.screenWidth, options.screenHeight, 
            projection, camera.getViewMatrix());
}

void Renderer::renderMainScene()
{
    // Clear the warp framebuffer before drawing the rest of the scene
    // because they both write to the same brightcolor texture
    unsigned int attachments[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffers.warpFrameBuffer.id);
    glDrawBuffers(2, attachments);
    glClear(GL_COLOR_BUFFER_BIT);

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffers.mainFramebufferMultisampled.id);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);  

    glDrawBuffers(2, attachments);
   
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // TODO improve uniform setting
    shaders[SHADER_LAMP].use();
    shaders[SHADER_LAMP].setMat4("view", camera.getViewMatrix());
    shaders[SHADER_LAMP].setMat4("projection", projection);
    Renderable::drawStage(SHADER_LAMP, shaders[SHADER_LAMP]);

    shaders[SHADER_PARTICLE].use();
    shaders[SHADER_PARTICLE].setMat4("view", camera.getViewMatrix());
    shaders[SHADER_PARTICLE].setMat4("projection", projection);
    Renderable::drawStage(SHADER_PARTICLE, shaders[SHADER_PARTICLE]);

    std::vector<std::shared_ptr<Light>> lights = Light::getAllLights();
    shaders[SHADER_SIMPLE_DIFFUSE].use();
    for (int i = 0; i < lights.size(); i++) {
        lights[i]->setUniforms(shaders[SHADER_SIMPLE_DIFFUSE], i);
    }
    shaders[SHADER_SIMPLE_DIFFUSE].setMat4("view", camera.getViewMatrix());
    shaders[SHADER_SIMPLE_DIFFUSE].setMat4("projection", projection);
    shaders[SHADER_SIMPLE_DIFFUSE].setVec3("viewPos", camera.getPos());
    shaders[SHADER_SIMPLE_DIFFUSE].setInt("numPointLights", lights.size());
    shaders[SHADER_SIMPLE_DIFFUSE].setFloat("ambientStrength", 0.0);
    Renderable::drawStage(SHADER_SIMPLE_DIFFUSE, shaders[SHADER_SIMPLE_DIFFUSE]);
    
    shaders[SHADER_LIGHTING].use();
    shaders[SHADER_LIGHTING].setMat4("view", camera.getViewMatrix());
    shaders[SHADER_LIGHTING].setMat4("projection", projection);
    shaders[SHADER_LIGHTING].setVec3("viewPos", camera.getPos());
    shaders[SHADER_LIGHTING].setInt("numPointLights", lights.size());
    shaders[SHADER_LIGHTING].setFloat("ambientStrength", 0.00);
    for (int i = 0; i < lights.size(); i++) {
        lights[i]->setUniforms(shaders[SHADER_LIGHTING], i);
    }
    Renderable::drawStage(SHADER_LIGHTING, shaders[SHADER_SIMPLE_DIFFUSE]);

    shaders[SHADER_CARD].use();
    shaders[SHADER_CARD].setMat4("projection", projection);
    Renderable::drawStage(SHADER_CARD, shaders[SHADER_CARD]);

    shaders[SHADER_UI_LIGHTING].use();
    shaders[SHADER_UI_LIGHTING].setMat4("projection", projection);
    shaders[SHADER_UI_LIGHTING].setMat4("view", glm::mat4(1.0f));
    shaders[SHADER_UI_LIGHTING].setVec3("lightDir", calcUiLightDir(Timer::global.get()));
    Renderable::drawStage(SHADER_UI_LIGHTING, shaders[SHADER_UI_LIGHTING]);

    glEnable(GL_STENCIL_TEST);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);  
    glStencilMask(0xFF);
    glDisable(GL_CULL_FACE);  // No face culling, warp effects are only 2d quads
    glColorMask(false, false, false, false);
    //glDepthMask(false);

    // update stencils/ using depth buffer
    shaders[SHADER_STENCIL].use();
    shaders[SHADER_STENCIL].setBool("resetDepth", false);
    shaders[SHADER_STENCIL].setMat4("projection", projection);
    shaders[SHADER_STENCIL].setMat4("view", glm::mat4(1.0f));
    Renderable::drawStage(SHADER_STENCIL, shaders[SHADER_STENCIL], false);

    //glStencilMask(0x00);
    //glDisable(GL_DEPTH_TEST);
    //shaders[SHADER_STENCIL].setBool("resetDepth", true);
    //Renderable::drawStage(SHADER_STENCIL, shaders[SHADER_STENCIL], false);

    glColorMask(true, true, true, true);
    //glDepthMask(true);
    glEnable(GL_CULL_FACE);  
    //glStencilMask(0x00);

    glDepthFunc(GL_ALWAYS);
    shaders[SHADER_CARD_BG].use();
    shaders[SHADER_CARD_BG].setMat4("projection", projection);
    shaders[SHADER_CARD_BG].setMat4("view", glm::mat4(1.0f));
    Renderable::drawStage(SHADER_CARD_BG, shaders[SHADER_CARD_BG]);

    glDepthFunc(GL_LESS);
    shaders[SHADER_UI_LIGHTING_CARD_IMAGE].use();
    shaders[SHADER_UI_LIGHTING_CARD_IMAGE].setMat4("projection", projection);
    shaders[SHADER_UI_LIGHTING_CARD_IMAGE].setMat4("view", glm::mat4(1.0f));
    Renderable::drawStage(SHADER_UI_LIGHTING_CARD_IMAGE, shaders[SHADER_UI_LIGHTING_CARD_IMAGE]);

    glDisable(GL_CULL_FACE);  // No face culling, warp effects are only 2d quads
    glColorMask(false, false, false, false);
    shaders[SHADER_STENCIL].use();
    shaders[SHADER_STENCIL].setBool("resetDepth", false);
    shaders[SHADER_STENCIL].setMat4("projection", projection);
    shaders[SHADER_STENCIL].setMat4("view", glm::mat4(1.0f));
    Renderable::drawStage(SHADER_STENCIL, shaders[SHADER_STENCIL]);
    glColorMask(true, true, true, true);
    glEnable(GL_CULL_FACE);  // No face culling, warp effects are only 2d quads
    glDepthFunc(GL_LESS);

    glDisable(GL_STENCIL_TEST);
    glStencilMask(0xFF);

    shaders[SHADER_SKYBOX].use();
    glm::mat4 skyboxView = glm::mat4(glm::mat3(camera.getViewMatrix()));  
    shaders[SHADER_SKYBOX].setMat4("view", skyboxView);
    shaders[SHADER_SKYBOX].setMat4("projection", projection);
    Renderable::drawStage(SHADER_SKYBOX, shaders[SHADER_SKYBOX]);


    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 
    shaders[SHADER_TEXT].use();
    shaders[SHADER_TEXT].setMat4("view", camera.getViewMatrix());
    shaders[SHADER_TEXT].setMat4("projection", projection);
    Renderable::drawStage(SHADER_TEXT, shaders[SHADER_TEXT]);
    glDisable(GL_BLEND);

    // Demultisample
    for (int i = 0; i < 2; i++) {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffers.mainFramebufferMultisampled.id);
        glReadBuffer( GL_COLOR_ATTACHMENT0 + i);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffers.mainFramebuffer.id);
        glDrawBuffers( 1, &attachments[i]);
        glBlitFramebuffer(0, 0, options.screenWidth, options.screenHeight, 0, 0, 
                options.screenWidth, options.screenHeight, 
                GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST); 
    }
}

void Renderer::renderWarpEffects()
{
    Shader warpShader1 = shaders[SHADER_WARP_STEP1];
    Shader warpShader2 = shaders[SHADER_WARP_STEP2];

    glDisable(GL_CULL_FACE);  // No face culling, warp effects are only 2d quads
    glEnable(GL_BLEND);       // The first pass will add together all the warp
    glDisable(GL_DEPTH_TEST);
    glBlendFunc(GL_ONE, GL_ONE);  // effect 2d vectors
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffers.normalBlendFramebuffer.id);
    glClear(GL_COLOR_BUFFER_BIT);
    warpShader1.use();
    warpShader1.setMat4("view", camera.getViewMatrix());
    warpShader1.setMat4("projection", projection);
    warpShader1.setVec2("screenSize", {options.screenWidth, options.screenHeight});
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, framebuffers.mainFramebuffer.colorTextures[0]);
    warpShader1.setInt("hdrBuffer", 1);
    Renderable::drawStage(SHADER_WARP_STEP1, shaders[SHADER_WARP_STEP1]);
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);

    // Copy over depth info so we don't render the warp effect over planets etc
    glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffers.mainFramebuffer.id);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffers.warpFrameBuffer.id);
    glBlitFramebuffer( 
            0, 0, options.screenWidth, options.screenHeight, 0, 0, 
            options.screenWidth, options.screenHeight, 
            GL_DEPTH_BUFFER_BIT, GL_NEAREST);

    // Second pass will use the vectors from the first pass and the prerendered
    // texture to render the warped effect
    glEnable(GL_DEPTH_TEST);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffers.warpFrameBuffer.id);
    warpShader2.use();
    warpShader2.setMat4("view", camera.getViewMatrix());
    warpShader2.setMat4("projection", projection);
    warpShader2.setVec2("screenSize", {options.screenWidth, options.screenHeight});
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, framebuffers.mainFramebuffer.colorTextures[0]);
    warpShader2.setInt("hdrBuffer0", 1);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, framebuffers.normalBlendFramebuffer.colorTextures[0]);
    warpShader2.setInt("normalAdjustBuffer", 2);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, framebuffers.mainFramebuffer.colorTextures[1]);
    warpShader2.setInt("hdrBuffer1", 3);
    Renderable::drawStage(SHADER_WARP_STEP2, shaders[SHADER_WARP_STEP2]);

    // We also render the antimatter stuff here
    Shader antiShader = shaders[SHADER_ANTI];
    antiShader.use();
    antiShader.setMat4("view", camera.getViewMatrix());
    antiShader.setMat4("projection", projection);
    antiShader.setVec2("screenSize", {options.screenWidth, options.screenHeight});
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, framebuffers.mainFramebuffer.colorTextures[0]);
    antiShader.setInt("hdrBuffer0", 1);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, framebuffers.mainFramebuffer.colorTextures[1]);
    antiShader.setInt("hdrBuffer1", 3);
    Renderable::drawStage(SHADER_ANTI, antiShader);
}

int Renderer::renderBloom()
{
    bool horizontal = true;
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
    Shapes::framebufferQuad->draw(framebufferShader);

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
        Shapes::framebufferQuad->draw(blurShader);

        horizontal = !horizontal;
    }
    return horizontal;
}

void Renderer::mergeEffects(int bloomOutputTextureNo)
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0); 
    glViewport(0, 0, options.screenWidth, options.screenHeight );
    
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
    Shapes::framebufferQuad->draw(blendShader);
}

void Renderer::renderFrame() 
{
    UIObject::setViewInfo(options.screenWidth, options.screenHeight, 
            projection, camera.getViewMatrix());

    toRenderMutex.lock();
    for (auto r : toRender) {
        if (r->isVisible()) {
            r->queueDraw();
        }
    }

    renderMainScene();
    renderWarpEffects();
    toRenderMutex.unlock();
    // TODO better way to pass this info?
    int buffno = renderBloom();
    mergeEffects(buffno);
}


void Renderer::addRenderable(shared_ptr<Renderable> r)
{
    std::lock_guard<std::mutex> guard(toRenderMutex);
    alwaysRender.push_back(r);
}

void Renderer::setToRender(std::vector<std::shared_ptr<Renderable>> toRender)
{
    std::lock_guard<std::mutex> guard(toRenderMutex);
    this->toRender = toRender;
    this->toRender.insert(this->toRender.begin(), alwaysRender.begin(), alwaysRender.end());
}



