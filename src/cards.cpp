#include "cards.h"

#include "timer.h"
#include "util.h"

#include <plog/Log.h>

#include <algorithm>
#include <random>

using namespace std;

// TODO different card models?
shared_ptr<LineModel> Card::cardModel;
shared_ptr<Font> Card::titleFont;
shared_ptr<Font> Card::cardFont;

void Card::setup()
{
    LOG_INFO << "Loading card models/fonts";
    // make sure to update the quad vertices if the model changes
    cardModel = shared_ptr<LineModel>(new LineModel("./res/models/card/card.obj"));
    titleFont = shared_ptr<Font>(new Font("res/fonts/conthrax.fnt"));
    cardFont = shared_ptr<Font>(new Font("res/fonts/gravity.fnt"));
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
}

void Card::draw(Shader& shader)
{
    shader.setVec3("color", info.color);
    shader.setCommon(UNIFORM_MODEL, model);
    cardModel->draw(shader);
    // Text is not drawn here, it is queued for later
}

// TODO standerdize object setup methods
Card::Card(CardInfo info) : Renderable(SHADER_CARD),
    titleText(titleFont, info.name, info.color, 0, 0.2),
    cardText(cardFont, info.text, info.color, 1.6, 0.15)
{
    position = {-1, -1, -5};
    // For clickbox
    quadVertices = { 
        {-1, 1.6, 0},
        {1, 1.6, 0},
        {1, -1.6, 0},
        {-1, -1.6, 0},
    };
}

void Card::updateModel()
{
    glm::mat4 model(1.0f);
    glm::vec3 tmpPosition = position;
    //tmpPosition.y += 0.1 * sin(3.14 / 4 * Timer::get("start") + phase);
    model = glm::translate(model, tmpPosition);
    model = glm::scale(model, {size, size, size});
    float angleY = 3.1415f / 16 * sin(3.14159 / 2 * Timer::get("start") + phase);
    model = glm::rotate(model, angleY, {0, 1, 0});
    if (isHovered) {
        float angleX = 3.1415f / 16 * sin(3.14159 * Timer::get("start") + phase);
        model = glm::rotate(model, angleX, {1, 0, 0});
    }
    setModel(model);
}


void Card::update(UpdateInfo& info)
{
    updateModel();

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
}

void Card::onClick()
{
    LOG_INFO << "Card: " << info.name << " clicked";
};

Hand::Hand()
{
    handPos = calcWorldSpaceCoords({screenWidth, screenHeight * 0.9}, 5);
}

void Hand::addCard(shared_ptr<Card> card)
{
    card->position = handPos;
    card->setVisible(true);
    cards.push_back(card);
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

Deck::Deck(std::vector<std::shared_ptr<Card>> cards) :
    cards(cards)
{
    shuffle();
}

void Deck::shuffle()
{
    random_shuffle(cards.begin(), cards.end());
}

std::shared_ptr<Card> Deck::draw()
{
    auto card = cards.back();
    cards.pop_back();
    return card;
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
