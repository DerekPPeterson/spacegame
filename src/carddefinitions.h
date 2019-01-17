#include "logic.h"

#include <string>

using namespace logic;

vector<Target> getSystemsControlledByActivePlayer(GameState& state)
{
    vector<Target> systemIds;
    for (auto &s : state.systems) {
        if (s.controllerId == state.turnInfo.activePlayer) {
            Target target = {.id = s.id, .type = TARGET_SYSTEM};
            systemIds.push_back(target);
        }
    }
    return systemIds;
}

void createSampleShip(GameState& state)
{
    auto card = state.stack.back();
    Ship sampleShip = {
        .type = "SS1",
        .attack = 1,
        .shield = 0,
        .armour = 1,
        .movement = 1,
        .owner = card.playedBy,
        .controller = card.playedBy,
        .curSystemId = card.targets[0]
    };
    state.ships.push_back(sampleShip);
    state.changes.push_back({.type = CHANGE_ADD_SHIP, .data = sampleShip});
}

namespace CardDefinitions {
    Card sample_ship = {
        .name = "Sample Ship",
        .cardText = "Construct a sample ship in a system you control",
        .cost = {{RESOURCE_MATERIALS, 1}},
        .getValidTargets = [] (GameState& state) -> pair<int, vector<Target>> 
        {
            return {1, getSystemsControlledByActivePlayer(state)};
        },
        .resolve = createSampleShip,
    };
}
