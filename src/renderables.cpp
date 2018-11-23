#include "renderables.h"

#include <glad/glad.h>

using namespace std;

std::unordered_map<void (*)(Shader&, std::vector<Renderable>), 
    std::vector<Renderable>> Renderable::drawQueues;

void Renderable::queueDraw()
{
    drawQueues[drawQueue].push_back(*this);
}

void Renderable::drawAllQueues(Shader& shader)
{
    for (auto p : drawQueues) {
        p.first(shader, p.second);
    }
};

void Renderable::drawQueue(Shader& shader, vector<Renderable> renderables)
{
    for (auto renderable : renderables) {
        renderable.draw(shader);
    }
}

void MeshRenderable::draw(Shader& shader) 
{
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}
