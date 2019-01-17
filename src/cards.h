#ifndef CARDS_H
#define CARDS_H

#include "drawable.h"
#include "renderables.h"
#include "model.h"
#include "text.h"
#include "uithings.h"
#include "logic.h"

#include <memory>
#include <map>

struct CardInfo
{
    std::string name = "Long Card Name";
    std::string text = "Warp Beacon: {wb}\nMaterials: {mat}\nAI: {ai}\nAM: {am}\nInf: {inf}";
    glm::vec3 color = {0.1, 2, 2};
    ResourceAmount cost = {{RESOURCE_MATERIALS, 1}, {RESOURCE_AI, 1}, {RESOURCE_ANTIMATTER, 1}, {RESOURCE_WARP_BEACONS, 1}, {RESOURCE_INFLUENCE, 1}};
};

std::string createCostString(ResourceAmount amount);

class Card : public Renderable , public Object, public Dragable,
             public needs_setup<Card>
{
    public:
        Card(CardInfo info);
        Card(logic::Card logicCard);
        virtual void draw(Shader& shader) override;
        virtual void queueDraw() override;
        virtual void update(UpdateInfo& info) override;
        static void setup();
        static std::shared_ptr<Card> createFrom(logic::Card);
    protected:
        void updateModel();
        void onClick() override;
        void onRelease() override;

        static std::shared_ptr<LineModel> cardModel;

        Text titleText;
        Text cardText;
        Text costText;

        float size = 0.2;
        glm::vec3 speed = {0, 0, 0};
        float phase = 0;

        CardInfo info;

        float highlight = 1;

        friend class Hand;
};

class Hand : public Object, public UIObject
{
    public:
        Hand();
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

class Deck : public Object
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
