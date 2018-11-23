#include "renderables.h"

#include <glad/glad.h>

using namespace std;

std::unordered_map<void (*)(std::vector<Renderable>), 
    std::vector<Renderable>> Renderable::drawQueues;

void Renderable::queueDraw()
{
    drawQueues[drawQueue].push_back(*this);
}

void Renderable::drawAllQueues()
{
    for (auto p : drawQueues) {
        p.first(p.second);
    }
};

void Renderable::drawQueue(vector<Renderable> renderables)
{
    for (auto renderable : renderables) {
        renderable.draw();
    }
}

void MeshRenderable::draw() 
{
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}
