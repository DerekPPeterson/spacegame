#ifndef RENDERERABLES_H
#define RENDERERABLES_H

#include <vector>
#include <unordered_map>

class Renderable
{
    public:
        virtual void queueDraw();
        virtual void draw() {};
    private:
        static std::unordered_map<void (*)(std::vector<Renderable>), 
            std::vector<Renderable>> drawQueues;

        static void drawQueue(std::vector<Renderable> queue);
        static void drawAllQueues();
};

class MeshRenderable : public Renderable
{
    public:
        MeshRenderable(unsigned int VAO, unsigned int nIndices) : 
            VAO(VAO), nIndices(nIndices) {};
        virtual void draw() override;
    private:
        unsigned int VAO;
        unsigned int nIndices;
};

#endif 
