#include "gamelogic.h"
#include "spaceThings.h"
#include "cards.h"
#include "uithings.h"


#include "event.h"

using namespace std;

vector<shared_ptr<Object>> GameLogic::getObjects()
{
    return state.objects;
}

vector<shared_ptr<Renderable>> GameLogic::getRenderables()
{
    vector<shared_ptr<Renderable>> renderables;
    for (auto o : state.objects) {
        auto r = dynamic_pointer_cast<Renderable>(o);
        if (r) {
            renderables.push_back(r);
        }
    }
    return renderables;
}

// TODO add some options
GameState GameLogic::startGame() 
{
    GameState state;

    int curIndex = 0;

    // Create and insert systems
    shared_ptr<SpaceGrid> spacegrid(new SpaceGrid);
    state.objects.push_back(spacegrid);
    state.spacegrid = curIndex++;

    vector<std::shared_ptr<Object>> systems = spacegrid->getAllSystems();
    state.objects.insert(state.objects.end(), systems.begin(), systems.end());
    for (int i = 0; i < systems.size(); i++) {
        state.systems.push_back(curIndex++);
    }

    // Create and insert some ships
    // TODO only flagship at start
    for (int i = 0; i < 30; i++) {
        state.objects.emplace_back(new SpaceShip("SS1", spacegrid->getSystem(0, 0)));
        state.units.push_back(curIndex++);
    }

    shared_ptr<Hand> hand(new Hand());
    state.objects.push_back(hand);
    state.hand = curIndex++;

    // TODO get decklist from somewhere out of current game
    shared_ptr<Deck> deck(new Deck({}));
    state.objects.push_back(hand);
    state.deck = curIndex++;

    // For now just use temp cards
    CardInfo info;
    for (int i = 0; i < 40; i++) {
        shared_ptr<Card> card(new Card(info));
        card->setVisible(false);
        deck->insert(card, DECK_BOTTOM);
        state.objects.emplace_back(card);
        state.cards.push_back(curIndex++);
    }
    deck->shuffle();

    for (int i = 0; i < 7; i++) {
        hand->addCard(deck->draw());
    }

    // TODO this is just temporary
    shared_ptr<ResourceDisplay> display(new ResourceDisplay());
    state.objects.push_back(display);


    this->state = state;

    

    return state;
}

void GameLogic::updateState()
{
    GameState newState;
    switch (state.phase) {
        default:
            newState = main(state);
    }

    state = newState;
}

GameState GameLogic::startTurn(GameState state) const
{
    return state;
}

GameState GameLogic::main(GameState state) const
{
    shared_ptr<pair<unsigned int, unsigned int>> systemCoords = Event::getLatestEvent(EVENT_SYSTEM_CLICK, true);
    if (systemCoords != nullptr) {
        for (auto i : state.units) {
            auto spacegrid = static_pointer_cast<SpaceGrid>(state.objects[state.spacegrid]);
            static_pointer_cast<SpaceShip>(state.objects[i])->gotoSystem(
                    spacegrid->getSystem(systemCoords->first, systemCoords->second));
        }
    }

    return state;
}

GameState GameLogic::resolveMovement(GameState state) const
{
    return state;
}

GameState GameLogic::resoveCard(GameState state) const
{
    return state;
}
