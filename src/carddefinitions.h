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

tuple<int, int, vector<Target>> singleSystemControlledByActivePlayer(GameState& state)
{
    return {1, 1, getSystemsControlledByActivePlayer(state)};
}

void createShipIn(GameState& state, logic::Ship ship)
{
    auto card = state.stack.back();
    ship.newId();
    ship.owner = card.playedBy;
    ship.owner = card.playedBy;
    ship.controller = card.playedBy;
    ship.curSystemId = card.targets.front();
    state.ships.push_back(ship);
    state.changes.push_back({.type = CHANGE_ADD_SHIP, .data = ship});
}

void resourcesToController(GameState& state, Ship& ship, ResourceAmount amount)
{
    auto player = state.getPlayerById(ship.controller);
    player->resources = player->resources + amount;
    state.changes.push_back({
            .type = CHANGE_PLAYER_RESOURCES, 
            .data=pair<int, ResourceAmount>(player->id, player->resources)
            });
}

namespace ShipDefinitions {
    Ship sampleShip = {
        .type = "SS1",
        .attack = 1,
        .shield = 0,
        .armour = 1,
        .movement = 1,
    };

    Ship miningShip = {
        .type = "Mining base",
        .attack = 0,
        .shield = 0,
        .armour = 1,
        .movement = 0,
        .upkeep = [](GameState& state, Ship& ship) {
            resourcesToController(state, ship, {{RESOURCE_MATERIALS, 1}});
        },
    };
};

namespace CardDefinitions {
    Card sample_ship = {
        .name = "Sample Ship",
        .cardText = "Construct a sample ship in a system you control",
        .cost = {{RESOURCE_MATERIALS, 1}},
        .getValidTargets = singleSystemControlledByActivePlayer,
        .resolve = [](GameState& state) {
            createShipIn(state, ShipDefinitions::sampleShip);
        },
    };

    Card resource_ship = {
        .name = "Mining Station",
        .cardText = "Construct in system you control.\nProvides 1 {mat} per turn",
        .cost = {},
        .getValidTargets = singleSystemControlledByActivePlayer,
        .resolve = [](GameState& state) {
            createShipIn(state, ShipDefinitions::miningShip);
        },
    };
}
