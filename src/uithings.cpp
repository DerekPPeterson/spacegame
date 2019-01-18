#include "uithings.h"

#include "spaceThings.h"
#include "util.h"
#include "event.h"

using namespace std;

shared_ptr<Model> WarpBeacon::m;

std::map<std::string, ResourceType> resourceStrings = {
    {"wb", RESOURCE_WARP_BEACONS},
    {"mat", RESOURCE_MATERIALS},
    {"ai", RESOURCE_AI},
    {"am", RESOURCE_ANTIMATTER},
    {"inf", RESOURCE_INFLUENCE},
};

std::shared_ptr<Renderable> createIcon(ResourceType type)
{
    Renderable * icon = nullptr;
    switch (type) {
        case RESOURCE_WARP_BEACONS:
            icon = new WarpBeacon(); break;
        case RESOURCE_MATERIALS:
            icon = new ResourceSphere(); break;
        case RESOURCE_AI:
            icon = new AiIcon(); break;
        case RESOURCE_ANTIMATTER:
            icon = new AntiMatterIcon(); break;
        case RESOURCE_INFLUENCE:
            icon = new InfluenceIcon(); break;
        // TODO new icon for this?
        case RESOURCE_ANY:
            icon = nullptr; break;
    }
    return shared_ptr<Renderable>(icon);
};


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
    tmpModel = glm::rotate(tmpModel, rotation * 0.7f, {1, 0, 0.75});
    shader.setCommon(UNIFORM_MODEL, tmpModel);
    m->draw(shader);
}

void ResourceSphere::update(UpdateInfo& info) 
{
    rotation = info.curTime * 2 * 3.14 * 0.1;
}

AiIcon::AiIcon() : Renderable(SHADER_LAMP), t(Fonts::console, "", {0, 2, 0}, 1, 0.25)
{
    position = {0, 0, 0};
}

void AiIcon::draw(Shader& shader)
{
    shader.setCommon(UNIFORM_USE_VIEW, false);
    glm::mat4 screenModel = getModel();
    screenModel = glm::scale(screenModel, glm::vec3(0.5));
    screenModel = glm::translate(screenModel, {0, 0, -0.01});
    shader.setCommon(UNIFORM_MODEL, screenModel);
    shader.setCommon(UNIFORM_COLOR, {0, 0, 0});
    Shapes::warpQuad->draw(shader);
    shader.setCommon(UNIFORM_USE_VIEW, true);
}

void AiIcon::queueDraw()
{
    Renderable::queueDraw();
    t.setModel(glm::translate(getModel(), {-0.5, +0.5, 0}));
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
        {.radius = 0.35, .phase = 0, .inclination = 3.14 / 2},
        {.radius = 0.35, .phase = 3.14, .inclination = 3.14 / 2},
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

    glm::mat4 warpModel = glm::scale(getModel(), glm::vec3(0.5));
    warpModel = glm::translate(warpModel, 0.2f * -glm::vec3(getModel() * glm::vec4(0, 0, 0, 1)));
    shader.setCommon(UNIFORM_MODEL, warpModel);
    Shapes::warpQuad->draw(shader);

    shader.setBool("useView", true);
}

void AntiMatterIcon::update(UpdateInfo& info) 
{
    rotation = info.curTime * 2 * 3.14;
}


InfluenceIcon::InfluenceIcon() : Renderable(SHADER_LAMP) 
{
    glm::vec3 followerPos = {-0.8, 0.2, -0.4};
    for (int i = 0; i < nFollowers; i++) {
        followers.push_back({
                followerPos, 
                rand_float_between(0, 2 * 3.14),
                rand_float_between(0.1, 1),
                });
        followerPos.x += 0.45 + rand_float_between(-0.2, 0.2);
        if (followerPos.x > 0.8) {
            followerPos.x = -0.8;
            followerPos.y += 0.1;
            followerPos.z += -0.2 + rand_float_between(-0.1, 0.1);
        }
    }
}

std::shared_ptr<Model> InfluenceIcon::m;
void InfluenceIcon::setup()
{
    m = shared_ptr<Model>(new Model("./res/models/person/person.obj"));
}

void InfluenceIcon::draw(Shader &shader)
{
    shader.setBool("useView", false);
    glm::mat4 personModel = glm::translate(getModel(), {0, 0.1, 0});
    personModel = glm::scale(getModel(), glm::vec3(0.8));

    shader.setCommon(UNIFORM_COLOR, {4, 0, 0});
    shader.setCommon(UNIFORM_MODEL, personModel);
    m->draw(shader);

    shader.setCommon(UNIFORM_COLOR, {0.6, 0, 0});
    for (auto f : followers) {
        glm::mat4 tmpModel = glm::translate(personModel, f.pos);
        tmpModel = glm::translate(tmpModel, {0, 0.2f * sin(f.speed * 2 * 3.14 * curTime + f.phase), 0});
        tmpModel = glm::scale(tmpModel, glm::vec3(0.8));
        shader.setCommon(UNIFORM_MODEL, tmpModel);
        m->draw(shader);
    }
    shader.setBool("useView", true);
}

void InfluenceIcon::update(UpdateInfo& info)
{
    curTime = info.curTime;
}

IconNum::IconNum(std::shared_ptr<Renderable> icon) : 
    Renderable(SHADER_UI_LIGHTING), t(Fonts::title, "", {1.1, 1.1, 1.1}), 
    icon(icon)
{
    position = {0, 0, 0};
}

void IconNum::update(UpdateInfo& info)
{
    dynamic_pointer_cast<Object>(icon)->update(info);
}

void IconNum::updateModel()
{
}

void IconNum::queueDraw()
{
    t.setText(to_string(val));
    t.setModel(glm::translate(model, glm::vec3(1, 0.75, 0)));
    t.queueDraw();

    dynamic_pointer_cast<has_model_mat>(icon)->setModel(model);
    icon->queueDraw();
}

ResourceDisplay::ResourceDisplay(float iconSize)
    : Renderable(SHADER_NONE), iconSize(iconSize)
{
    //setPosScreenspace({screenWidth * 0.05, screenHeight * 0.05});
    position = {0, 0, 0};
    shared_ptr<WarpBeacon> beacon(new WarpBeacon());
    auto warpCounter = shared_ptr<IconNum>(new IconNum(beacon));
    displays.push_back(warpCounter);

    shared_ptr<ResourceSphere> resourceSphere(new ResourceSphere());
    auto resCounter = shared_ptr<IconNum>(new IconNum(resourceSphere));
    displays.push_back(resCounter);

    shared_ptr<AiIcon> aiIcon(new AiIcon());
    auto aiCounter = shared_ptr<IconNum>(new IconNum(aiIcon));
    displays.push_back(aiCounter);

    shared_ptr<AntiMatterIcon> antimatterIcon(new AntiMatterIcon());
    auto antimatterCounter = shared_ptr<IconNum>(new IconNum(antimatterIcon));
    displays.push_back(antimatterCounter);

    shared_ptr<InfluenceIcon> influenceIcon(new InfluenceIcon());
    auto influenceCounter = shared_ptr<IconNum>(new IconNum(influenceIcon));
    displays.push_back(influenceCounter);
}

void ResourceDisplay::update(UpdateInfo& info)
{
    for (auto d : displays) {
        d->update(info);
    }
}

void ResourceDisplay::queueDraw()
{
    glm::mat4 curModel = glm::translate(getModel(), getPos());
    curModel = glm::scale(curModel, glm::vec3(iconSize));

    for (auto d : displays) {
        d->setModel(curModel);
        d->queueDraw();
        curModel = glm::translate(curModel, {4, 0, 0});
    }
}

void ResourceDisplay::set(ResourceAmount amount)
{
    for (auto p : amount) {
        displays[p.first]->setVal(p.second);
    }
}

void ResourceDisplay::set(ResourceType type, int val)
{
    displays[type]->setVal(val);
}

int ResourceDisplay::get(ResourceType type)
{
    return displays[type]->getVal();
}

Button::Button(std::string label, glm::vec3 position, glm::vec3 color, float size)
    : Renderable(SHADER_CARD), color(color), label(label), 
      text(Fonts::title, label, color, 0, 5.0/8), size(size)
{
    setPos(calcWorldSpaceCoords({position.x * screenWidth, position.y * screenHeight}, -position.z));

    model = glm::translate(model, getPos());
    model = glm::scale(model, glm::vec3(size));

    setVisible(true);

    float padding = 0.1;
    text.setModel(glm::translate(model, {padding, -padding, 0}));

    float textwidth = text.calcWidth();

    /* Create the LineMesh like so:
     *(0, 0)
     *     +-------------------+
     *     |                   |
     *     |Text (with padding)|
     *     |                   |
     *     +-------------------+
     *                     (textwidth + padding * 2, -1 - padding * 2)
     */
     vector<glm::vec3> vertices = {
         {0, 0, 0},
         {textwidth + 2 * padding, 0, 0},
         {textwidth + 2 * padding, -1 - padding * 2, 0},
         {0, -1 - padding * 2, 0},
         {0, 0, -0.2},
         {textwidth + 2 * padding, 0, -0.2},
         {textwidth + 2 * padding, -1 - padding * 2, -0.2},
         {0, -1 - padding * 2, -0.2},
     };
     vector<unsigned int> indices = {
        0, 1, 1, 2, 2, 3, 3, 0,
        4, 5, 5, 6, 6, 7, 7, 4,
        0, 4, 1, 5, 2, 6, 3, 7,
     };
     lineMesh = make_unique<LineMesh>(vertices, indices);

     // create clickbox vertices
     quadVertices.insert(quadVertices.begin(), vertices.begin(), vertices.begin() + 4);
}

void Button::queueDraw() 
{
    Renderable::queueDraw();
    text.queueDraw();
}

void Button::draw(Shader& shader) 
{
    shader.setCommon(UNIFORM_MODEL, model);
    shader.setCommon(UNIFORM_COLOR, color);
    lineMesh->draw(shader);
}

void Button::update(UpdateInfo& info)
{
    checkSetHoverQuad(info, true);
}

void Button::onClick()
{
    Event::triggerEvent<string>(EVENT_BUTTON_PRESS, label);
}
