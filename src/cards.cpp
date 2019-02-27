#include "cards.h"

#include "timer.h"
#include "util.h"
#include "event.h"

#include <plog/Log.h>

#include <algorithm>
#include <random>
#include <sstream>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

using namespace std;

std::ostream & operator<<(std::ostream &os, const Zone& p) {
    switch (p) {
        case ZONE_NONE:
            os << "ZONE_NONE"; break;
        case ZONE_DECK:
            os << "ZONE_DECK"; break;
        case ZONE_HAND:
            os << "ZONE_HAND"; break;
        case ZONE_STACK:
            os << "ZONE_STACK"; break;
        case ZONE_DISCARD:
            os << "ZONE_DISCARD"; break;
    }
    return os;
}

void SpringSystem::updatePositions(float deltaTime)
{
    for (auto& object : objects) {
        object->acceleration = {0, 0, 0};
    }
    for (auto& spring : springs) {
        auto force = spring->calcForceAonB();
        spring->b->acceleration += force / spring->b->mass;
        spring->a->acceleration -= force / spring->a->mass;
    }
    for (auto& object : objects) {
        if (not object->fixed) {
            object->acceleration += -object->speed * damping;
            object->speed += object->acceleration * deltaTime;
            object->setPos(object->getPos() + object->speed * deltaTime);
        } else {
        }
    }
}

// TODO different card models?
shared_ptr<LineModel> Card::cardModel;
shared_ptr<LineModel> Card::cardBackModel;
shared_ptr<Model> Card::stencilQuad;

std::string createCostString(ResourceAmount amount)
{
    stringstream ss;
    for (auto [str, type] : resourceStrings) {

        if (type == RESOURCE_ANY) {
            if (amount[type] > 0) {
                ss << "{" << str << "|" << amount[type] << "}";
            }
        } else {
            for (int i = 0; i < amount[type]; i++) {
                ss << "{" << str << "}";
            }
        }

    }
    return ss.str();
}

void Card::setup()
{
    LOG_INFO << "Loading card models/fonts";
    // make sure to update the quad vertices if the model changes
    cardModel = shared_ptr<LineModel>(new LineModel("./res/models/card/card.obj"));
    cardBackModel = shared_ptr<LineModel>(new LineModel("./res/models/card_back/card_back.obj"));
    stencilQuad = make_shared<Model>("./res/models/card/card_stencil.obj");
    
    // This loads the texture here rather than on card creation
    TexturedQuad(SHADER_CARD_BG, "./res/textures/dark_space.jpg");
}

void Card::queueDraw() 
{
    Renderable::queueDraw();

    if (faceUp) {
        glm::mat4 titleModel = glm::translate(model, {-1.3, -1.6, 0});
        titleModel = glm::rotate(titleModel, 3.141f / 2, {0, 0, 1});
        titleText.setModel(titleModel);
        titleText.queueDraw();

        glm::mat4 typeModel = glm::translate(model, {+1.05, -1.6, 0});
        typeModel = glm::rotate(typeModel, 3.141f / 2, {0, 0, 1});
        typeText.setModel(typeModel);
        typeText.queueDraw();

        glm::mat4 textModel = glm::translate(model, {-0.8, -0.5, 0.03});
        cardText.setModel(textModel);
        cardText.queueDraw();

        glm::mat4 costModel = glm::translate(model, {-1, 2, 0});
        costText.setModel(costModel);
        costText.queueDraw();

        glm::mat4 displayShipModel = glm::translate(model, {0, 0.65, -1});
        displayShipModel = glm::rotate(displayShipModel,(float) (Timer::global.get() * 2.0f * 3.14 / 4), {0, 1, 0});
        displayShipModel = glm::scale(displayShipModel, glm::vec3(0.5));
        displayShip.setModel(displayShipModel);
        displayShip.queueDraw();

        if (info.creates) {
            glm::mat4 hullModel = glm::translate(model, {0.8, -1.8, 0});
            hullModel = glm::scale(hullModel, glm::vec3(0.15));
            hullIcon.setModel(hullModel);
            hullIcon.queueDraw();

            glm::mat4 hullTextModel = glm::translate(model, {0.6, -1.7, 0});
            hullText.setModel(hullTextModel);
            hullText.queueDraw();
        }

        glm::mat4 cardBackgroundModel = glm::translate(model, {0, 0, -5});
        cardBackgroundModel = glm::scale(cardBackgroundModel, glm::vec3(10));
        cardBackground.setModel(cardBackgroundModel);
        cardBackground.queueDraw();

        stencilQuadWithModel.setModel(model);
        stencilQuadWithModel.queueDraw();
    }
}

void Card::draw(Shader& shader)
{
    shader.setVec3("color", info.color * highlight);
    shader.setCommon(UNIFORM_MODEL, model);


    if (faceUp) {
        cardModel->draw(shader);
    } else {
        cardBackModel->draw(shader);
    }
    // Text is not drawn here, it is queued for later
}

void Card::setFaceUp(bool faceUp) 
{
    this->faceUp = faceUp;
}

map<ResourceType, glm::vec3> CARD_COLORS = {
    {RESOURCE_WARP_BEACONS, {0, 2, 0}},
    {RESOURCE_MATERIALS, {1 * 2, 2 * 0.560,  2 * 0.239}},
    {RESOURCE_AI, {0, 1.5, 0}},
    {RESOURCE_ANTIMATTER, {2 * 0.745, 2 * 0.654, 2 * 0.843}},
    {RESOURCE_INFLUENCE, {2, 0, 0}},
};

Card::Card(CardInfo info) : Renderable(SHADER_CARD),
    titleText(Fonts::title, info.name, info.color, 0, 0.2),
    cardText(Fonts::regular, info.text, info.color, 1.6, 0.15),
    costText(Fonts::regular, createCostString(info.cost), info.color, 1.6, 0.25),
    typeText(Fonts::title, info.type, info.color, 9, 0.15),
    hullText(Fonts::title, info.creates ? to_string(info.creates->armour) : "", info.color, 0, 0.15),
    angleY(0, 3.14),
    info(info),
    displayShip(info.logicId, info.creates ? (shipModels[info.creates->type] ? shipModels[info.creates->type] : shipModels["FALLBACK"]) : nullptr),
    cardBackground(info.logicId, "./res/textures/dark_space.jpg"),
    stencilQuadWithModel(info.logicId, stencilQuad)
{
    position = {-1, -1, -5};
    // For clickbox
    quadVertices = { 
        {-1, 1.6, 0},
        {1, 1.6, 0},
        {1, -1.6, 0},
        {-1, -1.6, 0},
    };

    Card::info.color = {0, 0, 0};
    int totalCost = 0;
    ResourceAmount nothing = {};
    ResourceAmount resourceAmountForColors;
    if (info.cost == nothing) {
        resourceAmountForColors = info.provides;
    } else {
        resourceAmountForColors = info.cost;
    }
    for (auto [resourceType, amount] : resourceAmountForColors) {
        Card::info.color += CARD_COLORS[resourceType] * (float) amount;
        totalCost += amount;
    }
    if (totalCost) {
        Card::info.color /= totalCost;
    } else {
        Card::info.color = {0.5, 0.5, 0.5};
    }

    titleText.setColor(Card::info.color);
    cardText.setColor(Card::info.color);
    cardText.setColor(Card::info.color);
    typeText.setColor(Card::info.color);
    hullText.setColor(Card::info.color);
}

std::shared_ptr<Card> Card::createFrom(logic::Card logicCard)
{
    stringstream type;
    type << logicCard.type;
    CardInfo info = {
        .name = logicCard.name,
        .text = logicCard.cardText,
        .cost = logicCard.cost,
        .provides = logicCard.provides,
        .ownerId = logicCard.ownerId,
        .type = type.str(),
        .creates = logicCard.creates,
        .logicId = logicCard.id,
    };
    auto card = shared_ptr<Card>(new Card(info));
    card->logicId = logicCard.id;
    return card;
}

void Card::updateModel()
{
    glm::mat4 model(1.0f);
    glm::vec3 tmpPosition = position;
    //tmpPosition.y += 0.1 * sin(3.14 / 4 * Timer::get("start") + phase);
    model = glm::translate(model, tmpPosition);
    model = glm::scale(model, {size, size, size});

    if (isHovered) {
        angleY.target = 0;
    } else {
        angleY.target = 3.14 / 12;
    }
    model = glm::rotate(model, angleY.curVal, {0, 1, 0});
    setModel(model);
}


void Card::update(UpdateInfo& info)
{
    updateModel();
    costText.update(info);
    cardText.update(info);
    hullText.update(info);

    checkSetHoverQuad(info, true);
    checkSetDrag(info, true);
    
    float sizeChangeRate = 3;
    float targetSize;

    if (isHovered or zone == ZONE_STACK) {
        targetSize = 0.4;
    } else {
        targetSize = 0.2;
    }

    if (size > targetSize) {
        size /= (1 + sizeChangeRate * info.deltaTime);
        if (size < targetSize) {
            size = targetSize;
        }
        position.y -= size * 2 * info.deltaTime;
    } else if (size < targetSize) {
        size *= (1 + sizeChangeRate * info.deltaTime);
        position.y += size * 3 * info.deltaTime;
        if (size > targetSize) {
            size = targetSize;
        }
    }

    angleY.update(info.deltaTime);
}

void Card::onClick()
{
    LOG_INFO << "Card: " << info.name << " clicked";
};

void Card::onRelease()
{
    LOG_INFO << "Card: " << info.name << " released at: " << position.x << position.y;
    if (position.y > -0.45) {
        Event::triggerEvent(EVENT_PLAY_CARD, logicId);
    }
}

Hand::Hand(glm::vec2 screenPos)
{
    position = calcWorldSpaceCoords(
            {screenWidth * screenPos.x, screenHeight * screenPos.y}, 5);
    cardsAreVisible = true;
    cardsAreDragable = true;
    zone = ZONE_HAND;
}

void CardZone::addCard(shared_ptr<Card> card)
{
    if (card) {
        card->setVisible(cardsAreVisible);
        if (card->enemyOwned) {
            card->dragEnabled = false; }
        else {
            card->dragEnabled = cardsAreDragable;
        }
        card->zone = zone;
        cards.push_back(card);
        LOG_INFO << "Added card: '" << card->info.name << "' to " << zone;
    } else {
        LOG_ERROR << "Tried to add a nullptr to a card zone";
    }
}

void CardZone::removeCard(shared_ptr<Card> card)
{
    if (card) {
        card->setVisible(false);
        card->dragEnabled = false;
        card->zone = ZONE_NONE;
        cards.erase(remove(cards.begin(), cards.end(), card), cards.end());
        LOG_INFO << "Removed card: '" << card->info.name << "' from " << zone;
    } else {
        LOG_ERROR << "Tried to add a nullptr to a card zone";
    }
}


void Hand::update(UpdateInfo& info)
{
    sort(cards.begin(), cards.end(), [](const shared_ptr<Card> a, const shared_ptr<Card> b) 
            {return a->getPos().x > b->getPos().x;});

    float cardSpacing = (2.0 + 0.2) * 0.2;
        
    SpringSystem springSystem;

    for (int i = 0; i < cards.size(); i++) {
        // Cast card to a spring object for use in the springsystem
        auto card = cards[i];
        card->fixed = card->dragging; // some glitching without this
        auto cardSpringObject = dynamic_pointer_cast<SpringObject>(card);

        // Create a fixed position to attach the card to
        auto fixedPos = position - glm::vec3(i * cardSpacing, 0, 0);
        if (card->isHovered) {
            fixedPos += glm::normalize( -card->getPos()) * 0.5f + glm::vec3(0, 0.5, 0);
        }
        auto fixed = shared_ptr<SpringObject>(new SpringObject(fixedPos));
        fixed->fixed = true;

        // connect position and card with spring
        auto spring = shared_ptr<Spring>(new Spring(0, card, fixed));

        springSystem.objects.push_back(fixed);
        springSystem.objects.push_back(cardSpringObject);
        springSystem.springs.push_back(spring);
    }

    springSystem.updatePositions(info.deltaTime);
}

void Stack::update(UpdateInfo& info)
{
    float cardSpacing = 0.5;
        
    SpringSystem springSystem;

    for (int i = 0; i < cards.size(); i++) {
        // Cast card to a spring object for use in the springsystem
        auto card = cards[i];
        auto cardSpringObject = dynamic_pointer_cast<SpringObject>(card);
        cardSpringObject->fixed = false;

        // Create a fixed position to attach the card to
        auto fixedPos = position - glm::vec3(-cardSpacing * 0.2 * i, 0, cardSpacing * (cards.size() - i - 1));
        auto fixed = shared_ptr<SpringObject>(new SpringObject(fixedPos));
        fixed->fixed = true;

        // connect position and card with spring
        auto spring = shared_ptr<Spring>(new Spring(0, card, fixed));

        springSystem.objects.push_back(fixed);
        springSystem.objects.push_back(cardSpringObject);
        springSystem.springs.push_back(spring);
    }

    springSystem.updatePositions(info.deltaTime);
}

void Discard::update(UpdateInfo& info)
{
    float cardSpacing = 0.2;
        
    SpringSystem springSystem;

    for (int i = 0; i < cards.size(); i++) {
        // Cast card to a spring object for use in the springsystem
        auto card = cards[i];
        auto cardSpringObject = dynamic_pointer_cast<SpringObject>(card);
        cardSpringObject->fixed = false;

        // Create a fixed position to attach the card to
        auto fixedPos = position - glm::vec3(0, 0, cardSpacing * (cards.size() - i - 1));
        auto fixed = shared_ptr<SpringObject>(new SpringObject(fixedPos));
        fixed->fixed = true;

        // connect position and card with spring
        auto spring = shared_ptr<Spring>(new Spring(0, card, fixed));

        springSystem.objects.push_back(fixed);
        springSystem.objects.push_back(cardSpringObject);
        springSystem.springs.push_back(spring);
    }

    springSystem.updatePositions(info.deltaTime);
}


Stack::Stack()
{
    setPos(calcWorldSpaceCoords({screenWidth * 0.8, screenHeight * 0.5}, 5));
    cardsAreVisible = true;
    cardsAreDragable = false;
    zone = ZONE_STACK;
}

Deck::Deck()
{
    cardsAreVisible = true;
    cardsAreDragable = false;
    zone = ZONE_DECK;
}

Discard::Discard()
{
    position = calcWorldSpaceCoords({screenWidth * 0.1, screenHeight * 0.9}, 5);
    cardsAreVisible = true;
    cardsAreDragable = false;
    zone = ZONE_DISCARD;
}

void Discard::addCard(shared_ptr<Card> card)
{
    CardZone::addCard(card);
    card->highlight = 0.3;
}
