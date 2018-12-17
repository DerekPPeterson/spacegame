#ifndef UITHINGS_H
#define UITHINGS_H

#include "renderables.h"
#include "model.h"
#include "drawable.h"
#include "text.h"
#include <memory>

// Class to handle screen space info and transformations

class WarpBeacon : public Renderable, public Selectable, 
    public needs_setup<WarpBeacon>, public Object
{
    public:
        WarpBeacon() : Renderable(SHADER_UI_LIGHTING | SHADER_WARP_STEP1) 
            {position = glm::vec3(0);};
        void draw(Shader& shader) override;
        void drawWarp(Shader& shader) override;
        void update(UpdateInfo& info) override;
        static void setup();
    protected:
        float size = 1;
        float rotation = 0;
        float yshift = 0;
        float warpPulseScale = 1;

    static std::shared_ptr<Model> m;
    static std::shared_ptr<Model> warpQuad;
};

class IconNum : public Renderable, public Selectable, 
     public Object, public UIObject
{
    public:
        IconNum(std::shared_ptr<Renderable> icon, float size = 0.1);
        void queueDraw() override;
        void setVal(int n) {val = n;};
        virtual void update(UpdateInfo& info) override;
        void updateModel();
    private:
        int val = 0;
        Text t;
        float size = 0.2;
        std::shared_ptr<Renderable> icon;
        // TODO might not want to use a shared_ptr to a Renderable here
};

class ResourceDisplay : public Renderable, public Object, 
    public UIObject
{
    public:
        ResourceDisplay();
        virtual void update(UpdateInfo& info) override;
        void queueDraw() override;
    protected:
        std::vector<std::shared_ptr<IconNum>> displays;
};

#endif

