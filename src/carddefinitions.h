#include "logic.h"

#include <string>

using namespace logic;

vector<Target> getAllNonFlagships(GameState& state)
{
    vector<Target> targets;
    for (auto ship : state.ships) {
        if (ship.kind != SHIP_FLAGSHIP) {
            targets.push_back({.id = ship.id, .type = TARGET_SHIP});
        }
    }
    return targets;
}

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

tuple<int, int, vector<Target>> noTargets(GameState& state)
{
    return {0, 0, {}};
}

void subtle_hack(GameState& state) {
    auto cardIt = prev(state.stack.end(), 2);
    auto card = *cardIt;
    state.stack.erase(cardIt);
    auto haltedPlayer = state.getPlayerById(card.playedBy);
    haltedPlayer->hand.push_back(card);
    state.changes.push_back({.type=CHANGE_RETURN_CARD_STACK_TO_HAND, .data=card.id});

    state.drawCard(state.stack.back().playedBy);
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

    if (ship.kind == SHIP_RESOURCE) {
        auto player = state.getPlayerById(card.playedBy);
        player->playedResourceShipThisTurn = true;
    }
}

void destroyTargetShips(GameState& state)
{
    auto card = state.stack.back();
    for (auto shipId : card.targets) {
        state.changes.push_back({.type = CHANGE_REMOVE_SHIP, .data = shipId});
        state.deleteShipById(shipId);
        LOG_INFO << "Ship id: " << shipId << " destroyed";
    }
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

    Ship defaultFlagship = {
        .type = "Default Flagship",
        .attack = 2,
        .shield = 5,
        .armour = 10,
        .movement = 1,
        .kind = SHIP_FLAGSHIP,
        .upkeep = [](GameState& state, Ship& ship) {
            resourcesToController(state, ship, {{RESOURCE_WARP_BEACONS, 1}});
        },
    };

    Ship miningShip = {
        .type = "Mining Platform",
        .attack = 0,
        .shield = 0,
        .armour = 1,
        .movement = 1,
        .kind = SHIP_RESOURCE,
        .upkeep = [](GameState& state, Ship& ship) {
            resourcesToController(state, ship, {{RESOURCE_MATERIALS, 1}});
        },
    };

    Ship aiCore = {
        .type = "AI Coreship",
        .attack = 0,
        .shield = 0,
        .armour = 1,
        .movement = 1,
        .kind = SHIP_RESOURCE,
        .upkeep = [](GameState& state, Ship& ship) {
            resourcesToController(state, ship, {{RESOURCE_AI, 1}});
        },
    };

    Ship amGatherer = {
        .type = "AM Gatherer",
        .attack = 0,
        .shield = 0,
        .armour = 1,
        .movement = 0,
        .kind = SHIP_RESOURCE,
        .upkeep = [](GameState& state, Ship& ship) {
            resourcesToController(state, ship, {{RESOURCE_ANTIMATTER, 1}});
        },
    };

    Ship diplomaticVessal = {
        .type = "Diplomatic Vessel",
        .attack = 0,
        .shield = 0,
        .armour = 1,
        .movement = 0,
        .kind = SHIP_RESOURCE,
        .upkeep = [](GameState& state, Ship& ship) {
            resourcesToController(state, ship, {{RESOURCE_INFLUENCE, 1}});
        },
    };
};

namespace CardDefinitions {
    Card sample_ship = {
        .name = "Sample Ship",
        .cardText = "Construct a sample ship in a system you control",
        .cost = {{RESOURCE_MATERIALS, 1}},
        .type = CARD_SHIP,
        .getValidTargets = singleSystemControlledByActivePlayer,
        .creates = ShipDefinitions::sampleShip,
        .resolve = [](GameState& state) {
            createShipIn(state, ShipDefinitions::sampleShip);
        },
    };

    Card resource_ship = {
        .name = "Mining Platform",
        .cardText = "Construct in system you control.\nProvides 1 {mat} per turn",
        .cost = {},
        .provides = {{RESOURCE_MATERIALS, 1}},
        .type = CARD_RESOURCE_SHIP,
        .getValidTargets = singleSystemControlledByActivePlayer,
        .creates = ShipDefinitions::miningShip,
        .resolve = [](GameState& state) {
            createShipIn(state, ShipDefinitions::miningShip);
        },
    };

    Card ai_coreship = {
        .name = "AI Coreship",
        .cardText = "Construct in system you control.\nProvides 1 {ai} per turn",
        .cost = {},
        .provides = {{RESOURCE_AI, 1}},
        .type = CARD_RESOURCE_SHIP,
        .getValidTargets = singleSystemControlledByActivePlayer,
        .resolve = [](GameState& state) {
            createShipIn(state, ShipDefinitions::aiCore);
        },
    };

    Card subtle_hack = {
        .name = "Subtle hack",
        .cardText = "Halt the last card play. Return that card to it's owners hand. Draw a card.",
        .cost = {{RESOURCE_AI, 1}},
        .type = CARD_INSTANT_ACTION,
        //.getValidTargets = noTargets,
        .resolve = ::subtle_hack,
        .canPlay = [](GameState& state){return state.stack.size() > 0;},
    };

    Card am_gatherer = {
        .name = "Antimatter Collector",
        .cardText = "Construct in system you control.\nProvides 1 {am} per turn",
        .cost = {},
        .provides = {{RESOURCE_ANTIMATTER, 1}},
        .type = CARD_RESOURCE_SHIP,
        .getValidTargets = singleSystemControlledByActivePlayer,
        .resolve = [](GameState& state) {
            createShipIn(state, ShipDefinitions::amGatherer);
        },
    };

    Card diplomaticVessal = {
        .name = "Diplomatic Vessel",
        .cardText = "Construct in system you control.\nProvides 1 {inf} per turn",
        .cost = {},
        .provides = {{RESOURCE_INFLUENCE, 1}},
        .type = CARD_RESOURCE_SHIP,
        .getValidTargets = singleSystemControlledByActivePlayer,
        .resolve = [](GameState& state) {
            createShipIn(state, ShipDefinitions::diplomaticVessal);
        },
    };

    Card am_laser = {
        .name = "Anti-matter laser",
        .cardText = "Destroy target ship",
        .cost = {{RESOURCE_ANTIMATTER, 2}},
        .type = CARD_INSTANT_ACTION,
        .getValidTargets = [](GameState& state) {
            return tuple<int, int, vector<Target>>(1, 1, getAllNonFlagships(state));},
        .resolve = [](GameState& state) {
            destroyTargetShips(state);
        },
    };
}
