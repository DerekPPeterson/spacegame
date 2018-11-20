#include "render.h"
#include <vector>

using namespace std;

void Renderer::setup()
{
    // compile shaders
    // load models/textures/create VBOS
    // Create framebuffers
    // Create projection matrix
}

struct DrawInfo
{
    Shader *shader;
    DrawableInstance *d;
};

void drawDrawables(const vector<Drawable>& drawables)
{
    unordered_set<void (*)()> drawQueueFunctions;
    for (auto d : drawables) {
        d.queueDraw();
        drawQueueFunctions.insert(d.drawQueuePtr);
    }
    for (auto f : drawQueueFunctions) {
        f();
    }
}

void loop()
{
    // clear warp framebuffer

    // render main scene to framebuffer

    // Get the drawables of every object
    const unordered_set<Object*>& objects = Object::getObjects();
    vector<Drawable> sceneDrawables;
    vector<Drawable> warpDrawables;
    for (auto object : objects) {
        if (not object->isVisible()) continue;
        for (Drawable d : object->getDrawables()) {
            switch(d.stage) {
                case RENDER_SCENE: 
                    sceneDrawables.push_back(d); break;
                case RENDER_WARP:
                    warpDrawables.push_back(d); break;
                default:
                    ;
            }
        }
    }

    drawDrawables(sceneDrawables);

        // all visible lighted objects
        // all lamp objects
        // all simply lighted objects
        
        // skybox

    // render warp effects to warp framebuffers

    // calculate bloom in bloom pingpong buffers

    // merge previous framebuffers into display buffer

};
