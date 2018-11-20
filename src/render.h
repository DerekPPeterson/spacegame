#ifndef RENDER_H
#define RENDER_H

#include <unordered_set>


enum RenderStage {
    RENDER_NONE,
    RENDER_SCENE,
    RENDER_WARP
};

#ifndef JUST_RENDER_STAGES
#define JUST_RENDER_STAGES

#include "drawable.h"
#include "framebuffer.h"

class Renderer
{
    public:
        Renderer();
        void setup();
        void loop();
};
#endif
#endif
