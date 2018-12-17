#include "uithings.h"

#include "spaceThings.h"
#include "util.h"

using namespace std;

shared_ptr<Model> WarpBeacon::m;

void WarpBeacon::setup()
{
    m = shared_ptr<Model>(new Model("./res/models/warp_diamond/warp_diamond.obj"));
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
    Shapes::warpQuad->draw(shader);

    shader.setBool("useView", true);
}

void WarpBeacon::update(UpdateInfo& info) 
{
    rotation = info.curTime * 2 * 3.14 * 0.1;
    yshift = 0.1 * sin(2 * 3.14 * 0.5 * info.curTime);
    warpPulseScale = info.curTime * 2 * 3.14;
}

shared_ptr<Model> ResourceSphere::m;

void ResourceSphere::setup()
{
    m = shared_ptr<Model>(new Model("./res/models/resource_sphere/resource_sphere.obj"));
}

void ResourceSphere::draw(Shader& shader)
{
    glm::mat4 tmpModel = glm::translate(model, position);
    tmpModel = glm::rotate(tmpModel, rotation, {0, 1, 0});
    shader.setCommon(UNIFORM_MODEL, tmpModel);
    m->draw(shader);
}

void ResourceSphere::update(UpdateInfo& info) 
{
    rotation = info.curTime * 2 * 3.14 * 0.1;
}

AiIcon::AiIcon() : Renderable(SHADER_NONE), t(Fonts::console, "", {0, 2, 0}, 0, 0.4)
{
    position = {0, 0, 0};
}

void AiIcon::queueDraw()
{
    t.setModel(glm::translate(getModel(), {-0.7, 1, 0}));
    t.queueDraw();
}

void AiIcon::update(UpdateInfo &info)
{
    if (curLineLength > maxLineLength) {
        curText.push_back('\n');
        curLineLength = 0;
        curLines++;
        if (curLines >= maxLines)
        {
            curText = curText.substr(curText.find('\n') + 1, curText.length());
            curLines--;
        }
    }
    if (info.curTime - lastUpdate > updateInterval) {
        lastUpdate = info.curTime;
        curText.push_back(rand() % 48 + '0');
        t.setText(curText);
        curLineLength++;
    }
}


void AntiMatterIcon::draw(Shader& shader)
{
    shader.setCommon(UNIFORM_USE_VIEW, false);
    Orbit orbits[]  = {
        {.radius = 0.5, .phase = 0, .inclination = 3.14 / 2},
        {.radius = 0.5, .phase = 3.14, .inclination = 3.14 / 2},
    };

    glm::mat4 tmpModel = glm::rotate(getModel(), rotation, {1, 1, 0});

    shader.setCommon(UNIFORM_COLOR, {15, 15, 0});
    for (int i = 0; i < 2; i++) {
        glm::mat4 electronModel = glm::translate(tmpModel, 
                calcOrbitPosition({0, 0, 0}, orbits[i]));
        electronModel = glm::scale(electronModel, glm::vec3(0.05));
        shader.setCommon(UNIFORM_MODEL, electronModel);
        Shapes::sphere->draw(shader);
    }

    shader.setCommon(UNIFORM_COLOR, {0, 1.2, 0});
    for (int i = 0; i < 3; i++) {
        glm::mat4 protonModel = glm::scale(tmpModel, glm::vec3(0.06));
        protonModel = glm::rotate(protonModel, rotation * 1.2f + i * 3.14f * 2 / 3, {0, 0, 1});
        protonModel = glm::translate(protonModel, {0, 1.15, 0});
        shader.setCommon(UNIFORM_MODEL, protonModel);
        Shapes::sphere->draw(shader);

        if (i == 0) {
            shader.setCommon(UNIFORM_COLOR, {1.2, 0, 0});
        }
    }

    shader.setCommon(UNIFORM_USE_VIEW, true);
};

void AntiMatterIcon::drawWarp(Shader& shader) 
{
    shader.setBool("useView", false);

    glm::mat4 warpModel = glm::scale(getModel(), glm::vec3(0.7));
    warpModel = glm::translate(warpModel, 0.2f * -glm::vec3(getModel() * glm::vec4(0, 0, 0, 1)));
    shader.setCommon(UNIFORM_MODEL, warpModel);
    Shapes::warpQuad->draw(shader);

    shader.setBool("useView", true);
}

void AntiMatterIcon::update(UpdateInfo& info) 
{
    rotation = info.curTime * 2 * 3.14;
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

    shared_ptr<ResourceSphere> resourceSphere(new ResourceSphere());
    auto resCounter = shared_ptr<IconNum>(new IconNum(resourceSphere));
    resCounter->setPos(warpCounter->getPos() + glm::vec3(0.4, 0, 0));
    displays.push_back(resCounter);

    shared_ptr<AiIcon> aiIcon(new AiIcon());
    auto aiCounter = shared_ptr<IconNum>(new IconNum(aiIcon));
    aiCounter->setPos(resCounter->getPos() + glm::vec3(0.4, 0, 0));
    displays.push_back(aiCounter);

    shared_ptr<AntiMatterIcon> antimatterIcon(new AntiMatterIcon());
    auto antimatterCounter = shared_ptr<IconNum>(new IconNum(antimatterIcon));
    antimatterCounter->setPos(aiCounter->getPos() + glm::vec3(0.4, 0, 0));
    displays.push_back(antimatterCounter);
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
