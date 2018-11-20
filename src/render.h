#ifndef RENDER_H
#define RENDER_H

#include <vector>

#include "framebuffer.h"
#include "drawable.cpp"


class Renderer
{
    public:
        Renderer();
    private:
        vector<Object*>& objects;
};

#endif
