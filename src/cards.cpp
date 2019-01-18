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

std::string createCostString(ResourceAmount amount)
{
    stringstream ss;
    for (auto p : resourceStrings) {
        string str = p.first;
        ResourceType type = p.second;

        for (int i = 0; i < amount[type]; i++) {
            ss << "{" << str << "}";
        }
    }
    return ss.str();
}

void Card::setup()
{
    LOG_INFO << "Loading card models/fonts";
    // make sure to update the quad vertices if the model changes
    cardModel = shared_ptr<LineModel>(new LineModel("./res/models/card/card.obj"));
}

void Card::queueDraw() 
{
    Renderable::queueDraw();

    glm::mat4 titleModel = glm::translate(model, {-1.3, -1.6, 0});
    titleModel = glm::rotate(titleModel, 3.141f / 2, {0, 0, 1});
    titleText.setModel(titleModel);
    titleText.queueDraw();

    glm::mat4 textModel = glm::translate(model, {-0.8, -0.5, 0.03});
    cardText.setModel(textModel);
    cardText.queueDraw();

    glm::mat4 costModel = glm::translate(model, {-1, 2, 0});
    //costModel = glm::scale(costModel, glm::vec3(0.1));
    costText.setModel(costModel);
    costText.queueDraw();

}

void Card::draw(Shader& shader)
{
    shader.setVec3("color", info.color * highlight);
    shader.setCommon(UNIFORM_MODEL, model);
    cardModel->draw(shader);
    // Text is not drawn here, it is queued for later
}

map<ResourceType, glm::vec3> CARD_COLORS = {
    {RESOURCE_WARP_BEACONS, {0, 2, 0}},
    {RESOURCE_MATERIALS, {1 * 2, 2 * 0.560,  2 * 0.239}},
    {RESOURCE_AI, {0, 0, 0}},
    {RESOURCE_ANTIMATTER, {2 * 0.745, 2 * 0.654, 2 * 0.843}},
    {RESOURCE_INFLUENCE, {2, 0, 0}},
};

Card::Card(CardInfo info) : Renderable(SHADER_CARD),
    titleText(Fonts::title, info.name, info.color, 0, 0.2),
    cardText(Fonts::regular, info.text, info.color, 1.6, 0.15),
    costText(Fonts::regular, createCostString(info.cost), info.color, 1.6, 0.25)
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
    for (auto p : info.cost) {
        Card::info.color += CARD_COLORS[p.first] * (float) p.second;
        totalCost += p.second;
    }
    Card::info.color /= totalCost;

    titleText.setColor(Card::info.color);
    cardText.setColor(Card::info.color);
    cardText.setColor(Card::info.color);
}

std::shared_ptr<Card> Card::createFrom(logic::Card logicCard)
{
    CardInfo info = {
        .name = logicCard.name,
        .text = logicCard.cardText,
        .cost = logicCard.cost,
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
    float angleY = 3.1415f / 16 * sin(3.14159 / 2 * Timer::global.get() + phase);
    model = glm::rotate(model, angleY, {0, 1, 0});
    if (isHovered) {
        float angleX = 3.1415f / 16 * sin(3.14159 * Timer::global.get() + phase);
        model = glm::rotate(model, angleX, {1, 0, 0});
    }
    setModel(model);
}


void Card::update(UpdateInfo& info)
{
    updateModel();
    costText.update(info);
    cardText.update(info);

    checkSetHoverQuad(info, true);
    checkSetDrag(info, true);
    
    float sizeChangeRate = 3;
    float targetSize;

    if (isHovered) {
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

    if (position.y > -0.45) {
        highlight = 3;
    } else {
        highlight = 1;
    }
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

Hand::Hand()
{
    position = calcWorldSpaceCoords({screenWidth * 0.9, screenHeight * 0.9}, 5);
    cardsAreVisible = true;
    cardsAreDragable = true;
    zone = ZONE_HAND;
}

void CardZone::addCard(shared_ptr<Card> card)
{
    if (card) {
        card->setVisible(cardsAreVisible);
        card->dragEnabled = cardsAreDragable;
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
        cards.push_back(card);
        LOG_INFO << "Added card: '" << card->info.name << "' to " << zone;
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

Deck::Deck(std::vector<std::shared_ptr<Card>> cards)
{
    this->cards = cards;
    for (auto card : cards) {
        card->zone = ZONE_DECK;
    }
}
