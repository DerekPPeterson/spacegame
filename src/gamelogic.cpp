#include "gamelogic.h"
#include "spaceThings.h"

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
