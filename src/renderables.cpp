#include "renderables.h"

#include <glad/glad.h>

using namespace std;

RenderableQueues Renderable::queues;

void Renderable::queueDraw() {
    queues[stage][drawQueue].push_back(this);
}

void Renderable::drawStage(ShaderEnum stage, Shader& shader)
{
    for (auto p : queues[stage]) {
        p.first(shader, p.second);
    }
    queues[stage].clear();
};

void Renderable::drawQueue(Shader& shader, vector<Renderable*> renderables)
{
    for (auto renderable : renderables) {
        // TODO probably want to not print this all the time
        cout << "Rendering a " << typeid(*renderable).name() << endl;
        renderable->draw(shader);
    }
}

void MeshRenderable::draw(Shader& shader) 
{
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}
