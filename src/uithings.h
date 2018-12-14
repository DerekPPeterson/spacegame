#ifndef UITHINGS_H
#define UITHINGS_H

#include "drawable.h"
#include "renderables.h"
#include "model.h"
#include <memory>

class WarpBeacon : public Renderable, public Selectable, 
    public needs_setup<WarpBeacon>, public Object
{
    public:
        WarpBeacon() : Renderable(SHADER_CARD) {};
        void draw(Shader& shader) override;
        void update(UpdateInfo& info) override;
        static void setup();
    protected:
        float size = 0.2;
        float rotation = 0;

    static std::shared_ptr<Model> model;
};

#endif

