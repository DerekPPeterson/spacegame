#include "gamelogic.h"
#include "spaceThings.h"
#include "cards.h"
#include "uithings.h"

#include <exception>


#include "event.h"

using namespace std;

vector<shared_ptr<Object>> GraphicsObjectHandler::getObjects()
{
    return objects;
}

vector<shared_ptr<Renderable>> GraphicsObjectHandler::getRenderables()
{
    vector<shared_ptr<Renderable>> renderables;
    for (auto o : objects) {
        auto r = dynamic_pointer_cast<Renderable>(o);
        if (r) {
            renderables.push_back(r);
        }
    }
    return renderables;
}

void GraphicsObjectHandler::addObject(shared_ptr<Object> object)
{
    if (object->logicId) {
        index[object->logicId] = objects.size();
    }
    objects.push_back(object);
}

shared_ptr<Object> GraphicsObjectHandler::getObject(int logicId)
{
    if (index.find(logicId) != index.end()) {
        return objects[index[logicId]];
    } else {
        throw runtime_error("Could not find logicId in graphics object index");
    }
}

void GraphicsObjectHandler::initializePlayer(logic::Player player)
{
    shared_ptr<Hand> hand(new Hand());
    addObject(hand);
    shared_ptr<Deck> deck(new Deck({}));
    addObject(deck);

    for (auto logicCard : player.hand) {
        auto card = Card::createFrom(logicCard);
        hand->addCard(card);
        addObject(card);
    }
    for (auto logicCard : player.deck) {
        auto card = Card::createFrom(logicCard);
        card->setVisible(false);
        addObject(card);
    }
}

void GraphicsObjectHandler::startGame(logic::GameState initialState) 
{
    shared_ptr<SpaceGrid> spacegrid(new SpaceGrid);
    addObject(spacegrid);

    vector<std::shared_ptr<Object>> systems = spacegrid->getAllSystems();
    objects.insert(objects.end(), systems.begin(), systems.end());

    for (auto o: systems) {
        auto s = dynamic_pointer_cast<System>(o);
        s->logicId = initialState.getSystemByPos(s->gridx, s->gridy)->id;
    }

    initializePlayer(initialState.players.front());
}
