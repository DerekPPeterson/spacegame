#include "uithings.h"

#include "spaceThings.h"
#include "util.h"
#include "event.h"

using namespace std;

shared_ptr<Model> WarpBeacon::m;
shared_ptr<Model> HullIcon::m;

std::map<std::string, ResourceType> resourceStrings = {
    {"wb", RESOURCE_WARP_BEACONS},
    {"mat", RESOURCE_MATERIALS},
    {"ai", RESOURCE_AI},
    {"am", RESOURCE_ANTIMATTER},
    {"inf", RESOURCE_INFLUENCE},
    {"any", RESOURCE_ANY},
};

std::shared_ptr<Renderable> createIcon(ResourceType type, int n)
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
        case RESOURCE_ANY:
            icon = new AnyResIcon(n); break;
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

void HullIcon::setup()
{
    m = make_shared<Model>("./res/models/icons/hull.obj");
}

void HullIcon::draw(Shader& shader)
{
    glm::mat4 tmpModel = glm::translate(getModel(), position);
    tmpModel = glm::rotate(tmpModel, rotation, {0, 1, 0});
    shader.setCommon(UNIFORM_MODEL, tmpModel);
    m->draw(shader);
}

void HullIcon::update(UpdateInfo& info) 
{
    rotation = 3.14 / 10 * sin(info.curTime * 2 * 3.14 / 4);
}

AnyResIcon::AnyResIcon(int n) :
    Renderable(SHADER_NONE), t(Fonts::title, to_string(n), {2, 2, 2}, 0, 0.6)
{
}

void AnyResIcon::queueDraw()
{
    t.setModel(glm::translate(getModel(), {-0.5, 0.5, 0}));
    t.queueDraw();
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

ResourceDisplay::ResourceDisplay(glm::vec3 position, float iconSize)
    : Renderable(SHADER_NONE), iconSize(iconSize)
{
    setVisible(true);
    setPos(calcWorldSpaceCoords({position.x * screenWidth, position.y * screenHeight}, -position.z));
}

void ResourceDisplay::update(UpdateInfo& info)
{
    text.update(info);
}

void ResourceDisplay::set(ResourceAmount amount)
{
    stringstream ss;
    for (auto pair : resourceStrings) {
        auto str = pair.first;
        auto type = pair.second;

        if (amount.find(type) != amount.end())
        {
            ss << "{" << str << "}: " << amount[type] << endl;
        }
    }

    text.setText(ss.str());
}

void ResourceDisplay::queueDraw()
{
    glm::mat4 curModel = glm::translate(getModel(), getPos());
    curModel = glm::scale(curModel, glm::vec3(iconSize));
    text.setModel(curModel);
    text.queueDraw();
}

unique_ptr<LineMesh> create2DBox(glm::vec2 ul, glm::vec2 lr, float depth = 0.2)
{
     vector<glm::vec3> vertices = {
         {ul.x, ul.y, 0},
         {ul.x, lr.y, 0},
         {lr.x, lr.y, 0},
         {lr.x, ul.y, 0},

         {ul.x, ul.y, -depth},
         {ul.x, lr.y, -depth},
         {lr.x, lr.y, -depth},
         {lr.x, ul.y, -depth},
     };
     vector<unsigned int> indices = {
        0, 1, 1, 2, 2, 3, 3, 0,
        4, 5, 5, 6, 6, 7, 7, 4,
        0, 4, 1, 5, 2, 6, 3, 7,
     };
     return make_unique<LineMesh>(vertices, indices);
}


Button::Button(std::string label, glm::vec3 position, glm::vec3 color, 
        float size, string clickEventLabel, std::shared_ptr<Font> font)
    : Renderable(SHADER_CARD), color(color), selectedColor(color), label(label), 
      text(font, label, color, 0, 5.0/8), size(size), 
      clickEventLabel(clickEventLabel)
{
    setPos(calcWorldSpaceCoords({position.x * screenWidth, position.y * screenHeight}, -position.z));

    model = glm::translate(model, getPos());
    model = glm::scale(model, glm::vec3(size));

    setVisible(true);

    float textwidth = text.calcWidth();

     lineMesh = create2DBox({0, 0}, {textwidth + padding * 2, -1 - padding * 2});
     // create clickbox vertices
     quadVertices.insert(quadVertices.begin(), lineMesh->vertices.begin(), 
             lineMesh->vertices.begin() + 4);
}

void Button::queueDraw() 
{
    Renderable::queueDraw();
    auto drawColor = selected ? selectedColor : color;
    text.setColor(drawColor * (active ? 1.0f : 0.5f));
    float padding = 0.1;
    text.setModel(glm::translate(getModel(), {padding, -padding, 0}));

    text.queueDraw();
}

void Button::draw(Shader& shader) 
{
    shader.setCommon(UNIFORM_MODEL, model);
    auto drawColor = selected ? selectedColor : color;
    shader.setCommon(UNIFORM_COLOR, drawColor * (active ? 1.0f : 0.5f));
    lineMesh->draw(shader);
}

void Button::update(UpdateInfo& info)
{
    checkSetHoverQuad(info, true);
}

void Button::onClick()
{
    if (active) {
        if (clickEventLabel.size()) {
            Event::triggerEvent<string>(EVENT_BUTTON_PRESS, clickEventLabel);
            LOG_INFO << "Clicked button: " << clickEventLabel;
        } else {
            Event::triggerEvent<string>(EVENT_BUTTON_PRESS, label);
            LOG_INFO << "Clicked button: " << label;
        }
    }
}

void TurnIndicator::changeTurn(logic::TurnInfo newTurnInfo)
{
    turnInfo = newTurnInfo;

    if (newTurnInfo.whoseTurn == localPlayer) {
        color = {0, 2, 2};
        turnText.setText("Your Turn");
    } else {
        color = {2, 0, 0};
        turnText.setText("Enemy Turn");
    }

    switch (turnInfo.phase.front()) {
        case logic::PHASE_UPKEEP:
            phaseText.setText("Upkeep Phase"); break;
        case logic::PHASE_MAIN:
            phaseText.setText("Main Phase"); break;
        case logic::PHASE_MOVE:
            phaseText.setText("Movement Phase"); break;
        case logic::PHASE_END:
            phaseText.setText("End Phase"); break;
        default:
            LOG_ERROR << "Incorrect phase given";
    }

    turnText.setColor(color);
    phaseText.setColor(color);
}

TurnIndicator::TurnIndicator(glm::vec3 screenPos, int localPlayer, logic::TurnInfo turnInfo)
    : Renderable(SHADER_CARD), turnText(Fonts::title), phaseText(Fonts::title), localPlayer(localPlayer)
{
    setPos(calcWorldSpaceCoords({screenPos.x * screenWidth, screenPos.y * screenHeight}, -screenPos.z));
    setModel(glm::translate(model, getPos()));
    setModel(glm::scale(model, glm::vec3(size)));
    changeTurn(turnInfo);
    setVisible(true);
    turnText.setModel(glm::translate(getModel(), {0, 0, 0}));
    phaseText.setModel(glm::translate(getModel(), {0, -1, 0}));
}

void TurnIndicator::queueDraw()
{
    Renderable::queueDraw();
    turnText.queueDraw();
    phaseText.queueDraw();
}

DebugInfo::DebugInfo() : Renderable(SHADER_NONE), text(Fonts::regular, "", {1, 1, 1}, 0, 0.05)
{
    setPos(calcWorldSpaceCoords({0, 0}, 3));
    text.setModel(glm::translate(glm::mat4(1.0f), getPos()));
    setVisible(true);
};

void DebugInfo::queueDraw()
{
    text.queueDraw();
}

void DebugInfo::addInfo(string info)
{
    lines.push_back(info);
    if (lines.size() >= 15) {
        lines.pop_front();
    }
    stringstream display;
    for (auto& line : lines) {
        display << line << endl;
    }
    text.setText(display.str());
}

SystemInfo::SystemInfo(shared_ptr<System> sys, int localPlayer)
    : Renderable(SHADER_CARD), localPlayer(localPlayer), sys(sys)
{
    setVisible(true);
}

void SystemInfo::addShip(logic::Ship ship)
{
    char label[80];
    sprintf(label, "%-10s %2d %2d %2d", ship.type.c_str(), 
            ship.attack, ship.shield, ship.armour);
    glm::vec3 color, selectedColor;
    if (ship.controller == localPlayer) {
        color = {0, 2, 2};
        selectedColor = {1, 2, 2};
    } else {
        color = {4, 0, 0};
        selectedColor = {4, 1, 1};
    }
    auto button = make_shared<Button>(label, glm::vec3(0, 0, 0), color, 1, 
            "shipid" + to_string(ship.id), Fonts::mono);
    button->setColors(color, selectedColor);

    buttons[ship.id] = button;

}

void SystemInfo::removeShip(int id)
{
    buttons.erase(id);
}

void SystemInfo::draw(Shader& shader)
{
    // TODO finish this
    //for (auto& p : lineMeshes) {
    //    p.second->draw(shader);
    //}
}

void SystemInfo::queueDraw()
{
    auto sysScreenPos = calcScreenSpaceCoords(sys->getPos());
    auto worldPos = calcWorldSpaceCoords({sysScreenPos.x, sysScreenPos.y}, 8);
    setPos(worldPos);
    setModel(glm::translate(glm::mat4(1.0f), getPos() + glm::vec3(1, -1, 0) * size));
    setModel(glm::scale(getModel(), glm::vec3(size)));

    int curLine = 0;
    for (auto& [id, button] : buttons) {
        button->setModel(glm::translate(getModel(), {0, -1.2 * curLine, 0}));
        button->queueDraw();
        curLine++;
    }
}

void SystemInfo::update(UpdateInfo& info) 
{
    for (auto& [id, button] : buttons) {
        button->update(info);
    }
}

void SystemInfo::setShipButtonActive(int shipId, bool active)
{
    if (shipId == 0) {
        for (auto& [id, button] : buttons) {
            button->setActive(active);
        }
    } else if (buttons.count(shipId)) {
        buttons[shipId]->setActive(active);
    }
}

void SystemInfo::setShipButtonSelected(int shipId, bool selected)
{
    if (shipId == 0) {
        for (auto& [id, button] : buttons) {
            button->setSelected(selected);
        }
    } else if (buttons.count(shipId)) {
        buttons[shipId]->setSelected(selected);
    }
}
