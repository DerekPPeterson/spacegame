#include "cards.h"

#include "timer.h"

#include "util.h"

using namespace std;

LineModel Card::cardModel;
Font Card::titleFont;

void Card::setup()
{
    static bool isSetup = false;
    if (not isSetup) {
        // make sure to update the quad vertices if the model changes
        cardModel = LineModel("./res/models/card/card.obj");
        isSetup = true;
        titleFont = Font("res/fonts/conthrax.fnt");
    }

}

void Card::queueDraw() 
{
    Renderable::queueDraw();

    glm::mat4 textModel = glm::translate(model, {-1, 1.9, 0});
    textModel = glm::scale(textModel, glm::vec3(0.2));
    titleText.setModel(textModel);
    titleText.queueDraw();
}

void Card::draw(Shader& shader)
{
    shader.setVec3("color", color);
    shader.setCommon(UNIFORM_MODEL, model);
    cardModel.draw(shader);


    //shader.setVec3("color", {10, 0, 0});
    //LineMesh clickbox(quadVertices, {0, 1, 1, 2, 2, 3, 3, 0});
    //clickbox.draw(shader);

}

// TODO standerdize object setup methods
Card::Card() 
{
    setup();
    stage = SHADER_CARD;
    position = {-1, -1, -5};
    quadVertices = {
        {-1, 1.6, 0},
        {1, 1.6, 0},
        {1, -1.6, 0},
        {-1, -1.6, 0},
    };
    titleText = Text(&titleFont, name, color);
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
    checkSetHoverQuad(info.projection, glm::mat4(1.0f), 
            info.mouse.position.x, info.mouse.position.y, 
            info.screenWidth, info.screenHeight);
    checkSetDrag(info.projection, glm::mat4(1.0f), info.mouse,
            info.screenWidth, info.screenHeight);
    
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

Hand::Hand(int screenWidth, int screenHeight, glm::mat4 projection)
{
    handPos = calcWorldSpaceCoords({screenWidth, screenHeight * 0.9}, 5,
        projection, glm::mat4(1.0f), screenWidth, screenHeight);
}

void Hand::addCard(shared_ptr<Card> card)
{
    card->position = handPos;
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

        if (cards[i]->isHovered) {
            acceleration.x = 0;
        } else {
            acceleration.y = -springiness * (cards[i]->getPos().y - handPos.y);
        }

        acceleration -= cards[i]->speed * damping;
        acceleration *= info.deltaTime;
        cards[i]->speed += acceleration;
        newPositions[i] = cards[i]->position + cards[i]->speed * info.deltaTime;
    }

    for (int i = 0; i < cards.size(); i++) {
        cards[i]->position = newPositions[i];
    }
}


