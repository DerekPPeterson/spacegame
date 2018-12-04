#ifndef CARDS_H
#define CARDS_H

#include "drawable.h"
#include "renderables.h"
#include "model.h"
#include "text.h"

#include <memory>


class Card : public Renderable , public Object, public Dragable {
    public:
        Card();
        virtual void draw(Shader& shader) override;
        virtual void queueDraw() override;
        virtual void update(UpdateInfo& info) override;
        static void setup();
    private:
        void updateModel();

        static LineModel cardModel;
        static Font cardFont;
        static Font titleFont;

        Text titleText;
        Text cardText;

        glm::vec3 color = {0.5, 5, 5.5};
        float size = 0.2;
        glm::vec3 speed = {0, 0, 0};
        float phase = 0;

        std::string name = "Long Card Name";
        std::string text = "This is some card box text\nWith a newline. wooo";

        friend class Hand;
};

class Hand : public Object
{
    public:
        Hand();
        Hand(int screenWidth, int screenHeight, glm::mat4 projection);
        void addCard(std::shared_ptr<Card> card);
        virtual void update(UpdateInfo& info) override;
        std::vector<std::shared_ptr<Object>> getAllCards();
    private:
        std::vector<std::shared_ptr<Card>> cards;
        glm::vec3(handPos);
};

#endif
