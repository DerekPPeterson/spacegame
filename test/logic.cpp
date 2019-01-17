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

TEST_CASE("Basic Game Logic Tests", "[GameState]") {
    GameState state;
    state.startGame();

    REQUIRE(state.ships.size() == 2);
    REQUIRE(state.players.size() == 2);
    REQUIRE(state.players.front().hand.size() == 7);
    REQUIRE(state.players.front().deck.size() == 40 - 7);

    SECTION("Changes array tests") {
        // Play a card
        auto actions = state.getPossibleActions();
        auto playCard = findActionType(actions, ACTION_PLAY_CARD);
        REQUIRE(playCard); 
        state.performAction(*playCard);
        auto changes = state.getChangesAfter();
        REQUIRE(changes[0].type == CHANGE_PLAY_CARD);
        REQUIRE(get<int>(changes[0].data) == playCard->id);

        // Select targets for card
        actions = state.getPossibleActions();
        auto selectSystem = findActionType(actions, ACTION_SELECT_SYSTEM);
        REQUIRE(selectSystem);
        state.performAction(*selectSystem);
        changes = state.getChangesAfter(changes[0].changeNo);
        REQUIRE(changes.size() == 0);
        REQUIRE(state.turnInfo.phase.back() == PHASE_RESOLVE_STACK);

        // Let the card Resolve
        actions = state.getPossibleActions();
        auto doNothing = findActionType(actions, ACTION_NONE);
        REQUIRE(doNothing);
        state.performAction(*doNothing);
        changes = state.getChangesAfter(1);
        REQUIRE(changes.size() == 2);
        REQUIRE(changes[0].type == CHANGE_RESOLVE_CARD);
        REQUIRE(get<int>(changes[0].data) == playCard->id);
        REQUIRE(changes[1].type == CHANGE_ADD_SHIP);
    }
}
