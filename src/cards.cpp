#include "cards.h"

#include "timer.h"

LineModel Card::cardModel;

void Card::setup()
{
    static bool isSetup = false;
    if (not isSetup) {
        cardModel = LineModel("./res/models/card/card.obj");
        isSetup = true;
    }
}

void Card::draw(Shader& shader)
{
    shader.setVec3("color", color);
    glm::mat4 model(1.0f);
    glm::vec3 tmpPosition = position;
    tmpPosition.y += 0.1 * sin(3.14 / 4 * Timer::get("start"));
    model = glm::translate(model, tmpPosition);
    model = glm::scale(model, {size, size, size});
    float angleY = 3.1415f / 16 * sin(3.14159 / 2 * Timer::get("start"));
    model = glm::rotate(model, angleY, {0, 1, 0});
    if (isHovered) {
        float angleX = 3.1415f / 16 * sin(3.14159 * Timer::get("start"));
        model = glm::rotate(model, angleX, {1, 0, 0});
    }
    shader.setCommon(UNIFORM_MODEL, model);
    cardModel.draw(shader);
}

// TODO standerdize object setup methods
Card::Card() 
{
    setup();
    stage = SHADER_CARD;
    position = {-1, -1, -5};
}

void Card::update(UpdateInfo& info)
{
    checkSetHover(info.projection, glm::mat4(1.0f), 
            info.mousePos.x, info.mousePos.y, 
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
    } else if (size < targetSize) {
        size *= (1 + sizeChangeRate * info.deltaTime);
        if (size > targetSize) {
            size = targetSize;
        }
    }
}

