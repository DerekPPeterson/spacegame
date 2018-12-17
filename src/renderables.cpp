#include "renderables.h"

#include <glad/glad.h>
#include <plog/Log.h>

#include <cstring>

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
        SHADER_ANTI,
        SHADER_CARD,
        SHADER_TEXT,
        SHADER_UI_LIGHTING,
    };
    for (auto s : stageList) {
        if (s & stage) {
            queues[s][getDrawQueuesFunc()].push_back(this);
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
            case SHADER_ANTI:
                renderable->drawWarp(shader);
                LOG_VERBOSE << "Rendering during warp a: " << typeid(*renderable).name();
                break;
            default:
                renderable->draw(shader);
                LOG_VERBOSE << "Rendering a: " << typeid(*renderable).name();
        }
    }
}

void MeshRenderable::draw(Shader& shader) 
{
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);
}


InstanceMeshRenderable::InstanceMeshRenderable(
        ShaderEnum stage, unsigned int VAO, unsigned int nIndices,
        std::vector<InstanceAttribName> attribs,
        vector<unsigned int> textureIds) : 
    MeshRenderable(stage, VAO, nIndices), textureIds(textureIds)
{
    int curAttribNo = 3;
    glGenBuffers(1, &bufId);
    glBindBuffer(GL_ARRAY_BUFFER, bufId);
    for (int i = 0; i < attribs.size(); i++) {
        switch (attribs[i]) {
            case INSTANCE_ATTRIB_VEC4:
                dataSize += sizeof(glm::vec4);
                break;
            case INSTANCE_ATTRIB_VEC3:
                dataSize += sizeof(glm::vec3);
                break;
            case INSTANCE_ATTRIB_VEC2:
                dataSize += sizeof(glm::vec2);
                break;
            case INSTANCE_ATTRIB_MAT4:
                dataSize += sizeof(glm::mat4);
                break;
        }
    }

    buf.resize(dataSize * maxInstances);
    glBufferData(GL_ARRAY_BUFFER, dataSize * maxInstances, buf.data(), GL_STREAM_DRAW);

    long offset = 0;
    for (int i = 0; i < attribs.size(); i++) {
        switch (attribs[i]) {
            case INSTANCE_ATTRIB_VEC4:
                setVertexAttribPointersVec4(curAttribNo, offset);
                offset += 4 * sizeof(float);
                curAttribNo++;
                break;
            case INSTANCE_ATTRIB_VEC3:
                setVertexAttribPointersVec3(curAttribNo, offset);
                offset += 3 * sizeof(float);
                curAttribNo++;
                break;
            case INSTANCE_ATTRIB_VEC2:
                setVertexAttribPointersVec2(curAttribNo, offset);
                offset += 2 * sizeof(float);
                curAttribNo++;
                break;
            case INSTANCE_ATTRIB_MAT4:
                setVertexAttribPointersMat4(curAttribNo, offset);
                offset += 16 * sizeof(float);
                curAttribNo += 4;
                break;
        }
    }
}

void InstanceMeshRenderable::addInstance(void *data)
{
    if (buf.size() < (curInstance + 1) * dataSize) {
        buf.resize(buf.size() + dataSize);
    }
    memcpy(&buf[curInstance*dataSize], data, dataSize);

    curInstance++;
}

void InstanceMeshRenderable::loadBufferData(unsigned int *buf, void* data, int size)
{
    if (not buf) {
        glGenBuffers(1, buf);
    }
    glBindBuffer(GL_ARRAY_BUFFER, *buf);
    glBufferSubData(GL_ARRAY_BUFFER, 0, dataSize * curInstance, &buf[0]);
}

void InstanceMeshRenderable::setVertexAttribPointersVec4(int startAt, long offset)
{
    glBindVertexArray(VAO);
    glEnableVertexAttribArray(startAt);
    glVertexAttribPointer(startAt, 4, GL_FLOAT, GL_FALSE, dataSize, (void*) offset);
    glVertexBindingDivisor(startAt, 1);
}

void InstanceMeshRenderable::setVertexAttribPointersVec3(int startAt, long offset)
{
    glBindVertexArray(VAO);
    glEnableVertexAttribArray(startAt);
    glVertexAttribPointer(startAt, 3, GL_FLOAT, GL_FALSE, dataSize, (void*) offset);
    glVertexAttribDivisor(startAt, 1);
}


void InstanceMeshRenderable::setVertexAttribPointersVec2(int startAt, long offset)
{
    glBindVertexArray(VAO);
    glEnableVertexAttribArray(startAt);
    glVertexAttribPointer(startAt, 2, GL_FLOAT, GL_FALSE, dataSize, (void*) offset);
    glVertexAttribDivisor(startAt, 1);
}

void InstanceMeshRenderable::setVertexAttribPointersMat4(int startAt, long offset)
{
    glBindVertexArray(VAO);
    for (int i = 0; i < 4; i ++) {
        glEnableVertexAttribArray(startAt+i);
        glVertexAttribPointer(startAt + i, 4, GL_FLOAT, GL_FALSE, 
                dataSize, (void*) (i * sizeof(glm::vec4) + offset));
    }
    for (int i = 0; i < 4; i ++) {
        glVertexAttribDivisor(startAt + i, 1);
    }
}

void InstanceMeshRenderable::draw(Shader& shader)
{
    int i = 0;
    for (auto id : textureIds) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, id);
        i++;
    }

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, bufId);
    glBufferData(GL_ARRAY_BUFFER, (curInstance) * dataSize, buf.data(), GL_DYNAMIC_DRAW);
    glDrawElementsInstanced(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0, curInstance);

    curInstance = 0;
    queued = false;
};

void InstanceMeshRenderable::queueDraw() {
    if (queued) {
        return; // Only queue the draw once
    }
    ShaderEnum stageList[] = {
        SHADER_TEXT
    };
    for (auto s : stageList) {
        if (s & stage) {
            queues[s][getDrawQueuesFunc()].push_back(this);
            queued = true;
        }
    }
}

