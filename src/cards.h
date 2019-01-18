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
#include <iostream>

struct CardInfo
{
    std::string name = "Long Card Name";
    std::string text = "Warp Beacon: {wb}\nMaterials: {mat}\nAI: {ai}\nAM: {am}\nInf: {inf}";
    glm::vec3 color = {0.1, 2, 2};
    ResourceAmount cost = {{RESOURCE_MATERIALS, 1}, {RESOURCE_AI, 1}, {RESOURCE_ANTIMATTER, 1}, {RESOURCE_WARP_BEACONS, 1}, {RESOURCE_INFLUENCE, 1}};
};

std::string createCostString(ResourceAmount amount);

enum Zone
{
    ZONE_NONE,
    ZONE_DECK,
    ZONE_HAND,
    ZONE_STACK,
    ZONE_DISCARD
};

std::ostream & operator<<(std::ostream &os, const Zone& p);

struct SpringObject : virtual public has_position
{
    SpringObject() {};
    SpringObject(glm::vec3 position) {setPos(position);};
    float mass = 1;
    bool fixed = false;
    glm::vec3 speed = {0, 0, 0};
    glm::vec3 acceleration = {0, 0, 0};
};

struct Spring
{
    Spring() {};
    Spring(float l, std::shared_ptr<SpringObject> a, std::shared_ptr<SpringObject> b )
        : l(l), a(a), b(b) {};
    float k = 20;
    float l = 1;
    std::shared_ptr<SpringObject> a;
    glm::vec3 aOffset = {0, 0, 0};
    std::shared_ptr<SpringObject> b;
    glm::vec3 bOffset = {0, 0, 0};

    glm::vec3 calcForceAonB() {
        glm::vec3 springVec = a->getPos() + aOffset - b->getPos() + bOffset ;
        float length = glm::length(springVec);
        if (length == 0) {
            springVec = {0.001, 0, 0};
        }
        return (glm::length(springVec) - l) * k * glm::normalize(springVec);
    }

};

struct SpringSystem
{
    float damping = sqrt(20);
    std::vector<std::shared_ptr<SpringObject>> objects;
    std::vector<std::shared_ptr<Spring>> springs;
    void updatePositions(float deltaTime);
};

class Card : public Renderable , public Object, public Dragable,
             public needs_setup<Card>, public SpringObject
{
    public:
        Card(CardInfo info);
        Card(logic::Card logicCard);

        virtual void draw(Shader& shader) override;
        virtual void queueDraw() override;
        virtual void update(UpdateInfo& info) override;
        static void setup();

        void play();

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
        Zone zone;

        float highlight = 1;

        friend class CardZone;
        friend class Hand;
        friend class Deck;
};

class CardZone : public Object, public UIObject, virtual public has_position
{
    public:
        //CardZone();
        virtual void addCard(std::shared_ptr<Card> card);
        virtual void removeCard(std::shared_ptr<Card> card);
        //virtual void update(UpdateInfo& info) override;
        //std::vector<std::shared_ptr<Object>> getAllCards();
    protected:
        std::vector<std::shared_ptr<Card>> cards;
        bool cardsAreVisible = false;
        bool cardsAreDragable = false;
        Zone zone = ZONE_NONE;
};

class Stack : public CardZone
{
    public:
        Stack();
        virtual void update(UpdateInfo& info) override;
};

class Hand : public CardZone
{
    public:
        Hand();
        virtual void update(UpdateInfo& info) override;
};

class Deck : public CardZone
{
    public:
        Deck(std::vector<std::shared_ptr<Card>>);
};

#endif
