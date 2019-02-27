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
#include <optional>

struct CardInfo
{
    std::string name = "Long Card Name";
    std::string text = "Warp Beacon: {wb}\nMaterials: {mat}\nAI: {ai}\nAM: {am}\nInf: {inf}";
    std::string type = "Card Type";
    glm::vec3 color = {0.1, 2, 2};
    ResourceAmount cost = {
        {RESOURCE_MATERIALS, 1}, 
        {RESOURCE_AI, 1}, 
        {RESOURCE_ANTIMATTER, 1}, 
        {RESOURCE_WARP_BEACONS, 1}, 
        {RESOURCE_INFLUENCE, 1}
    };
    ResourceAmount provides = {};
    int ownerId = 0;
    int logicId = 0;
    std::optional<logic::Ship> creates;
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
    float k = 40;
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
    float damping = 2 * sqrt(40);
    std::vector<std::shared_ptr<SpringObject>> objects;
    std::vector<std::shared_ptr<Spring>> springs;
    void updatePositions(float deltaTime);
};

class StencilModel : public ModelWithModel
{
    public:
        StencilModel(int id, std::shared_ptr<Model> m) 
            : ModelWithModel(SHADER_STENCIL, m),
              id(id % 255 + 1)
        {};

        virtual void draw(Shader& shader) {
            glStencilFunc(GL_ALWAYS, id, 0xFF);
            ModelWithModel::draw(shader);
        }
    private:
        int id;
};

class ModelUnderStencil : public ModelWithModel
{
    public:
        ModelUnderStencil(int id, std::shared_ptr<Model> m)
            : ModelWithModel(SHADER_UI_LIGHTING_CARD_IMAGE, m),
              id(id % 255 + 1)
        {};

        virtual void draw(Shader& shader) {
            glStencilFunc(GL_EQUAL, id, 0xFF);
            ModelWithModel::draw(shader);
        }
    private:
        int id;
};

class TexturedQuadUnderStencil : public TexturedQuad
{
    public:
        TexturedQuadUnderStencil(int id, std::string path)
            : TexturedQuad(SHADER_CARD_BG, path),
              id(id % 255 + 1)
        {};

        virtual void draw(Shader& shader) {
            glStencilFunc(GL_EQUAL, id, 0xFF);
            TexturedQuad::draw(shader);
        }
    private:
        int id;
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

        void setFaceUp(bool faceUp);

        void play();

        static std::shared_ptr<Card> createFrom(logic::Card);

        bool enemyOwned = false;
    protected:
        void updateModel();
        void onClick() override;
        void onRelease() override;

        bool faceUp = true;
        static std::shared_ptr<LineModel> cardModel;
        static std::shared_ptr<LineModel> cardBackModel;

        Text titleText;
        Text cardText;
        Text costText;
        Text typeText;
        Text hullText;
        HullIcon hullIcon;
        AttackIcon attackIcon;

        float size = 0.2;
        glm::vec3 speed = {0, 0, 0};
        float phase = 0;
        Interpolated<float> angleY;

        CardInfo info;
        Zone zone;

        ModelUnderStencil displayShip;
        TexturedQuadUnderStencil cardBackground;
        static std::shared_ptr<Model> stencilQuad;
        StencilModel stencilQuadWithModel;

        float highlight = 1;

        friend class CardZone;
        friend class Hand;
        friend class Deck;
        friend class Discard;
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
        Hand(glm::vec2 screenPos);
        virtual void update(UpdateInfo& info) override;
};

class Deck : public CardZone
{
    public:
        Deck();
};

class Discard : public CardZone
{
    public:
        Discard(); 
        virtual void update(UpdateInfo& info) override;
        virtual void addCard(std::shared_ptr<Card> card) override;
};

#endif
