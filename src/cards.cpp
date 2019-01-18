#include "cards.h"

#include "timer.h"
#include "util.h"
#include "event.h"

#include <plog/Log.h>

#include <algorithm>
#include <random>
#include <sstream>

using namespace std;

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
    handPos = calcWorldSpaceCoords({screenWidth, screenHeight * 0.9}, 5);
}

void Hand::addCard(shared_ptr<Card> card)
{
    if (card) {
        card->position = handPos;
        card->setVisible(true);
        card->dragEnabled = true;
        cards.push_back(card);
    }
}

void Hand::update(UpdateInfo& info)
{
    sort(cards.begin(), cards.end(), [](const shared_ptr<Card> a, const shared_ptr<Card> b) 
            {return a->getPos().x < b->getPos().x;});

    float cardSpacing = 0.2; // fraction of card size
    float springiness = 20; // spring constant (ish)
    float width = 2;        // width of card model
    float damping = sqrt(springiness);      // force to slow down cards
    float right = handPos.x;
    float left = right;     
    for (int i = 0; i < cards.size(); i++) {
        left -= width * cards[i]->size * (1 + cardSpacing);
    }
    vector<glm::vec3> newPositions(cards.size());
    for (int i = 0; i < cards.size(); i++) {
        glm::vec3 acceleration = {0, 0, 0};

        // Calculate horizontal acceleration of cards due to adjacent cards
        if (i > 0) {
            acceleration.x -= springiness * (cards[i]->position.x - cards[i-1]->position.x - width * cards[i]->size * (1 + cardSpacing));
        } else {
            acceleration.x += springiness * (-cards[i]->position.x + left - width * cards[i]->size * (1 + cardSpacing));
        }
        if (i < cards.size() - 1) {
            acceleration.x += springiness * (-cards[i]->position.x + cards[i+1]->position.x - width * cards[i]->size * (1 + cardSpacing));
        } else {
            acceleration.x += springiness * (-cards[i]->position.x + right - width * cards[i]->size * (1 + cardSpacing));
        }

        // if a card is hovered it shouldn't move horizontally
        if (cards[i]->isHovered) {
            acceleration.x = 0;
        }
        
        // Pull cards back to the hand if they are not activly being dragged
        if (not cards[i]->beingDragged) {
            acceleration.y = -springiness * (cards[i]->getPos().y - handPos.y);
        }

        // Calculate new card positions
        acceleration -= cards[i]->speed * damping;
        acceleration *= info.deltaTime;
        cards[i]->speed += acceleration;
        newPositions[i] = cards[i]->position + cards[i]->speed * info.deltaTime;
    }

    // Update card positions
    for (int i = 0; i < cards.size(); i++) {
        cards[i]->position = newPositions[i];
    }
}

Stack::Stack()
{
    stackPos = calcWorldSpaceCoords({screenWidth * 0.8, screenHeight * 0.5}, 5);
}

void Stack::addCard(shared_ptr<Card> card)
{
    if (card) {
        card->setVisible(true);
        card->dragEnabled = false;
        cards.push_back(card);
    }
}


Deck::Deck(std::vector<std::shared_ptr<Card>> cards) :
    cards(cards)
{
}

void Deck::shuffle()
{
    random_shuffle(cards.begin(), cards.end());
}

std::shared_ptr<Card> Deck::draw()
{
    if (cards.size()) {
        auto card = cards.back();
        cards.pop_back();
        return card;
    } else {
        return shared_ptr<Card>(nullptr);
    }
}

void Deck::insert(std::shared_ptr<Card> card, DeckLocation location)
{
    vector<shared_ptr<Card>> toInsert = {card};
    switch (location) {
        case DECK_TOP:
            cards.insert(cards.begin(), toInsert.begin(), toInsert.end());
            break;
        case DECK_BOTTOM:
            cards.push_back(card);
            break;
        case DECK_RANDOM:
            auto insertIt = cards.begin() + rand() % cards.size();
            cards.insert(insertIt, toInsert.begin(), toInsert.end());
    }
}
