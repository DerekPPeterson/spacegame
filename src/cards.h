#ifndef CARDS_H
#define CARDS_H

#include "drawable.h"
#include "renderables.h"
#include "model.h"

#include <memory>


class Card : public Renderable , public Object, public Dragable {
    public:
        Card();
        virtual void draw(Shader& shader) override;
        virtual void update(UpdateInfo& info) override;
        static void setup();
    private:
        void updateModel();
        static LineModel cardModel;
        glm::vec3 color = {0.5, 5, 5.5};
        float size = 0.2;
        float xspeed = 0;
        float phase = 0;

        friend class Hand;
};

class Hand : public Object
{
    public:
        Hand() {};
        void addCard(std::shared_ptr<Card> card);
        virtual void update(UpdateInfo& info) override;
        std::vector<std::shared_ptr<Object>> getAllCards();
    private:
        std::vector<std::shared_ptr<Card>> cards;
        std::vector<float> calculateSpringCardAccelerations(std::vector<Card*> cards);
        float right = 2.5;
};

#endif
