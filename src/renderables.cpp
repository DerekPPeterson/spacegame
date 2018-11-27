#include "renderables.h"

#include <glad/glad.h>

using namespace std;

RenderableQueues Renderable::queues;

void Renderable::queueDraw() {
    ShaderEnum stageList[] = {
        SHADER_NONE,
        SHADER_LIGHTING,
        SHADER_SIMPLE_DIFFUSE,
        SHADER_LAMP,
        SHADER_SKYBOX,
        SHADER_WARP_STEP1,
        SHADER_CARD,
    };
    for (auto s : stageList) {
        if (s & stage) {
            queues[s][drawQueue].push_back(this);
        }
    }
}

void Renderable::drawStage(ShaderEnum stage, Shader& shader)
{
    
    switch (stage) { 
        // We still need the renderables for step2
        case SHADER_WARP_STEP1:
            for (auto p : queues[stage]) {
                p.first(stage, shader, p.second);
            }
            break;
        case SHADER_WARP_STEP2:
            for (auto p : queues[SHADER_WARP_STEP1]) {
                p.first(SHADER_WARP_STEP1, shader, p.second);
            }
            queues[SHADER_WARP_STEP1].clear();
            break;
        default:
            for (auto p : queues[stage]) {
                p.first(stage, shader, p.second);
            }
            queues[stage].clear();
    }
};

void Renderable::drawQueue(ShaderEnum drawingStage, Shader& shader, vector<Renderable*> renderables)
{
    for (auto renderable : renderables) {
        // TODO probably want to not print this all the time
        switch (drawingStage) {
            case SHADER_WARP_STEP1:
            case SHADER_WARP_STEP2:
                renderable->drawWarp(shader);
                cout << "Rendering during warp a: " << typeid(*renderable).name() << endl;
                break;
            default:
                renderable->draw(shader);
                cout << "Rendering a: " << typeid(*renderable).name() << endl;
        }
    }
}

void MeshRenderable::draw(Shader& shader) 
{
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

