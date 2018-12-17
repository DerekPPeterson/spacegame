#include "uithings.h"

using namespace std;

shared_ptr<Model> WarpBeacon::m;
shared_ptr<Model> WarpBeacon::warpQuad;

void WarpBeacon::setup()
{
    m = shared_ptr<Model>(new Model("./res/models/warp_diamond/warp_diamond.obj"));
    warpQuad = shared_ptr<Model>(new Model("./res/models/quad/quad.obj"));
}

void WarpBeacon::draw(Shader& shader) 
{
    glm::mat4 tmpModel = glm::translate(model, position + glm::vec3(0, yshift, 0));
    tmpModel = glm::rotate(tmpModel, rotation, {0, 1, 0});
    tmpModel = glm::scale(tmpModel, glm::vec3(size));
    shader.setCommon(UNIFORM_MODEL, tmpModel);
    m->draw(shader);
}

void WarpBeacon::drawWarp(Shader& shader) 
{
    shader.setBool("useView", false);

    glm::mat4 tmpModel = glm::translate(model, position + glm::vec3(0, yshift, 0));
    shader.setCommon(UNIFORM_MODEL, tmpModel);
    shader.setFloat("frequency", 5);
    shader.setFloat("phase", warpPulseScale);
    warpQuad->draw(shader);

    shader.setBool("useView", true);
}

void WarpBeacon::update(UpdateInfo& info) 
{
    rotation = info.curTime * 2 * 3.14 * 0.1;
    yshift = 0.1 * sin(2 * 3.14 * 0.5 * info.curTime);
    warpPulseScale = info.curTime * 2 * 3.14;
}

IconNum::IconNum(std::shared_ptr<Renderable> icon, float size) : 
    Renderable(SHADER_UI_LIGHTING), t(Fonts::title, "", {1.1, 1.1, 1.1}), 
    size(size), icon(icon)
{
    position = calcWorldSpaceCoords({screenWidth / 2, screenHeight / 2}, 5);
}

void IconNum::update(UpdateInfo& info)
{
    dynamic_pointer_cast<Object>(icon)->update(info);
}

void IconNum::updateModel()
{
    model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::scale(model, glm::vec3(size));
}

void IconNum::queueDraw()
{
    updateModel();
    t.setText(to_string(val));
    t.setModel(glm::translate(model, glm::vec3(1, 0.75, 0)));
    t.queueDraw();

    dynamic_pointer_cast<has_model_mat>(icon)->setModel(model);
    icon->queueDraw();
}

ResourceDisplay::ResourceDisplay()
    : Renderable(SHADER_NONE)
{
    shared_ptr<WarpBeacon> beacon(new WarpBeacon());
    auto warpCounter = shared_ptr<IconNum>(new IconNum(beacon));
    warpCounter->setPosScreenspace({screenWidth * 0.05, screenHeight * 0.05});
    displays.push_back(warpCounter);

    shared_ptr<WarpBeacon> beacon1(new WarpBeacon());
    auto warpCounter1 = shared_ptr<IconNum>(new IconNum(beacon1));
    warpCounter1->setPosScreenspace({screenWidth * 0.05, screenHeight * 0.05});
    warpCounter1->setPos(warpCounter1->getPos() + glm::vec3(0.4, 0, 0));
    displays.push_back(warpCounter1);
}

void ResourceDisplay::update(UpdateInfo& info)
{
    for (auto d : displays) {
        d->update(info);
    }
}

void ResourceDisplay::queueDraw()
{
    for (auto d : displays) {
        d->queueDraw();
    }
}
