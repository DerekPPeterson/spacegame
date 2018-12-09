#ifndef CARDS_H
#define CARDS_H

#include "drawable.h"
#include "renderables.h"
#include "model.h"
#include "text.h"

#include <memory>

struct CardInfo
{
    std::string name = "Long Card Name";
    std::string text = "This is some card box text\nMore text";
    glm::vec3 color = {0.1, 2, 2};
};

class Card : public Renderable , public Object, public Dragable,
             public needs_setup<Card> {
    public:
        Card(CardInfo info);
        virtual void draw(Shader& shader) override;
        virtual void queueDraw() override;
        virtual void update(UpdateInfo& info) override;
        static void setup();
    protected:
        void updateModel();
        void onClick() override;

        static std::shared_ptr<LineModel> cardModel;
        static std::shared_ptr<Font> cardFont;
        static std::shared_ptr<Font> titleFont;

        Text titleText;
        Text cardText;

        float size = 0.2;
        glm::vec3 speed = {0, 0, 0};
        float phase = 0;

        CardInfo info;

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

// TODO render deck?
enum DeckLocation
{
    DECK_TOP,
    DECK_BOTTOM,
    DECK_RANDOM
};

class Deck : Object
{
    public:
        Deck(std::vector<std::shared_ptr<Card>>);
        void shuffle();
        std::shared_ptr<Card> draw();
        void insert(std::shared_ptr<Card> card, DeckLocation location);
    protected:
        std::vector<std::shared_ptr<Card>> cards;
};

#endif
