#include "catch.hpp"

#include <optional>
#include <prettyprint.hpp>

#include "logic.h"

using namespace logic;
using namespace std;

optional<Action> findActionType(vector<Action> actions, ActionType type)
{
    optional<Action> found;
    for (auto a : actions) {
        if (a.type == type) {
            found = a;
            break;
        }
    }
    return found;
}

optional<Change> getChangeOfType(ChangeType type, vector<Change> changes)
{
    auto it = find_if(changes.begin(), changes.end(), 
            [type](Change c) {return c.type == type;});
    if (it == changes.end()) {
        return {};
    } else {
        return *it;
    }
}

TEST_CASE("Basic Game Logic Tests", "[GameState]") {
    GameState state;
    state.startGame();

    REQUIRE(state.ships.size() == 2);
    REQUIRE(state.players.size() == 2);
    REQUIRE(state.players.front().hand.size() == 7);
    REQUIRE(state.players.front().deck.size() == 40 - 7);

    SECTION("Changes array tests") {
        // Play a card
        auto currentPlayer = state.turnInfo.whoseTurn;
        auto actions = state.getPossibleActions(currentPlayer);
        auto playCard = findActionType(actions, ACTION_PLAY_CARD);
        REQUIRE(playCard); 
        state.performAction(*playCard);
        auto changes = state.getChangesAfter();
        auto playCardChange = getChangeOfType(CHANGE_PLAY_CARD, changes);
        REQUIRE(playCardChange);
        REQUIRE(get<int>(playCardChange->data) == playCard->id);

        // Select targets for card
        actions = state.getPossibleActions(currentPlayer);
        auto selectSystem = findActionType(actions, ACTION_SELECT_SYSTEM);
        REQUIRE(selectSystem);
        state.performAction(*selectSystem);
        REQUIRE(state.turnInfo.phase.back() == PHASE_RESOLVE_STACK);

        // Let the card Resolve
        actions = state.getPossibleActions(currentPlayer);
        auto doNothing = findActionType(actions, ACTION_NONE);
        REQUIRE(doNothing);
        state.performAction(*doNothing);
        changes = state.getChangesAfter(1);

        auto resolveCardChange = getChangeOfType(CHANGE_RESOLVE_CARD, changes);
        REQUIRE(resolveCardChange);
        REQUIRE(get<int>(resolveCardChange->data) == playCard->id);
        auto addShipChange = getChangeOfType(CHANGE_ADD_SHIP, changes);
        REQUIRE(addShipChange);
    }
}

TEST_CASE("ResourceAmount arithmatic", "[ResourceAmount]")
{
    SECTION("Simple tests") {
        ResourceAmount a = {
            {RESOURCE_MATERIALS, 2},
            {RESOURCE_INFLUENCE, 1}
        };

        ResourceAmount b = {
            {RESOURCE_MATERIALS, 1},
            {RESOURCE_INFLUENCE, 1}
        };
        
        REQUIRE(a == a);
        REQUIRE(a != b);
        REQUIRE(b < a);

        ResourceAmount aPlusB = {
            {RESOURCE_MATERIALS, 3},
            {RESOURCE_INFLUENCE, 2}
        };
        REQUIRE(a + b == aPlusB);
    }

    SECTION("RESOURCE_ANY tests") {
        ResourceAmount a = {
            {RESOURCE_ANY, 5}
        };

        ResourceAmount b = {
            {RESOURCE_MATERIALS, 1},
            {RESOURCE_INFLUENCE, 1}
        };

        ResourceAmount c = {
            {RESOURCE_MATERIALS, 9},
            {RESOURCE_INFLUENCE, 1}
        };

        ResourceAmount d = {
            {RESOURCE_MATERIALS, 4},
            {RESOURCE_INFLUENCE, 2}
        };

        ResourceAmount aMinusB = {
            {RESOURCE_ANY, 3}
        };

        //REQUIRE(a - b == aMinusB);
        //REQUIRE((a <= c) == true);
        //REQUIRE((a >= b) == true);
        //REQUIRE((a <= d) == true);

        ResourceAmount amount = {
            {RESOURCE_WARP_BEACONS, 1},
            {RESOURCE_MATERIALS, 0},
        };
        ResourceAmount cost = {
            {RESOURCE_MATERIALS, 1},
        };

        REQUIRE((cost <= amount) == false);

        amount = {
            {RESOURCE_WARP_BEACONS, 1},
            {RESOURCE_MATERIALS, 1},
        };

        REQUIRE((cost <= amount) == true);

    }
}
