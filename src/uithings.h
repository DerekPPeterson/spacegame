#ifndef UITHINGS_H
#define UITHINGS_H

#include "renderables.h"
#include "model.h"
#include "drawable.h"
#include "text.h"
#include "logic.h"
#include "util.h"
#include "spaceThings.h"
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
};

class ResourceSphere : public Renderable, public Selectable, 
    public needs_setup<ResourceSphere>, public Object
{
    public:
        ResourceSphere() : Renderable(SHADER_UI_LIGHTING) 
            {position = glm::vec3(0);};
        void draw(Shader& shader) override;
        void update(UpdateInfo& info) override;
        static void setup();

    protected:
        float rotation = 0;

        static std::shared_ptr<Model> m;
};

class AiIcon : public Renderable, public Selectable, public Object
{
    public:
        AiIcon();
        void draw(Shader& shader) override;
        void queueDraw() override;
        void update(UpdateInfo& info) override;
    protected:
        std::string curText = "";
        float updateInterval = 0.1;
        float lastUpdate = 0;
        int curLines = 0;
        int maxLines = 4;
        int curLineLength = 0;
        int maxLineLength = 4;
        Text t;
};

class AntiMatterIcon : public Renderable, public Selectable, public Object
{
    public:
        AntiMatterIcon() : Renderable(SHADER_LAMP | SHADER_ANTI) {position = {0, 0, 0};};
        void draw(Shader& shader) override;
        void drawWarp(Shader& shader) override;
        void update(UpdateInfo& info) override;
    protected:
        float rotation;
};

class InfluenceIcon : public Renderable, public Selectable, public Object,
    public needs_setup<InfluenceIcon>
{
    public:
        InfluenceIcon();
        void draw(Shader& shader) override;
        void update(UpdateInfo& info) override;
        static void setup();
    protected:
        struct follower {
            glm::vec3 pos;
            float phase;
            float speed;
        };
        std::vector<follower> followers;
        int nFollowers = 12;
        float curTime = 0;
        static std::shared_ptr<Model> m;
};

class IconNum : public Renderable, public Selectable, 
     public Object, public UIObject
{
    public:
        IconNum(std::shared_ptr<Renderable> icon);
        void queueDraw() override;
        void setVal(int n) {val = n;};
        int getVal() {return val;};
        virtual void update(UpdateInfo& info) override;
        void updateModel();
    private:
        int val = 0;
        Text t;
        std::shared_ptr<Renderable> icon;
        // TODO might not want to use a shared_ptr to a Renderable here
};

extern std::map<std::string, ResourceType> resourceStrings;
std::shared_ptr<Renderable> createIcon(ResourceType type);

class ResourceDisplay : public Renderable, public Object, 
    public UIObject, public has_model_mat
{
    public:
        ResourceDisplay(glm::vec3 position, float iconSize = 0.1);
        virtual void update(UpdateInfo& info) override;
        void queueDraw() override;
        void set(ResourceAmount amount); 
    protected:
        float iconSize = 0.1;
        Text text;
};

class Button : public Object, public Renderable, public UIObject,
    public Selectable
{
    public:
        Button(std::string text, glm::vec3 position, glm::vec3 color, float size, std::string clickEventLabel = "");
        virtual void queueDraw() override;
        virtual void draw(Shader& shader) override;
        virtual void update(UpdateInfo& info) override;
        virtual void onClick() override;
        void setActive(bool active) {this->active = active;};
        void setSelected(bool selected) {this->selected = selected;};
        void setColors(glm::vec3 normal, glm::vec3 selected) {
            color = normal;
            selectedColor = selected;
        }
    protected:
        glm::vec3 color;
        glm::vec3 selectedColor;
        std::unique_ptr<LineMesh> lineMesh;
        std::string label;
        Text text;
        float size;
        float padding = 0.1;
        bool active = false;
        bool selected = false;
        std::string clickEventLabel;

};

class TurnIndicator : public Object, public Renderable, public UIObject,
    public has_model_mat
{
    public:
        TurnIndicator(glm::vec3 screenPos, int localPlayer, logic::TurnInfo turnInfo);
        void changeTurn(logic::TurnInfo newTurnInfo);

        virtual void queueDraw() override;
        //virtual void draw(Shader& shader) override;
    private:
        logic::TurnInfo turnInfo;
        Text turnText;
        Text phaseText;
        glm::vec3 color;
        int localPlayer;
        float size = 0.1;
};

class DebugInfo : public Object, public Renderable, public UIObject
{
    public:
        DebugInfo();
        void addInfo(std::string info);
        virtual void queueDraw() override;
    private:
        std::list<std::string> lines;
        Text text;

};


class SystemInfo : public Object, public Renderable, public UIObject,
    public has_model_mat
{
    public:
        SystemInfo(std::shared_ptr<System> sys, int localPlayer);
        void addShip(logic::Ship ship);
        void removeShip(int id);

        void draw(Shader& shader) override;
        void queueDraw() override;
        void update(UpdateInfo& info) override;
        void setShipButtonActive(int shipId, bool active);
        void setShipButtonSelected(int shipId, bool selected);

    private:
        float size = 0.1;
        int localPlayer = 0;
        std::shared_ptr<System> sys;

        std::map<int, std::shared_ptr<Button>> buttons;
};

#endif
