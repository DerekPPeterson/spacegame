#include "catch.hpp"

#include "logic.h"

using namespace logic;

TEST_CASE("Basic Game Logic Tests", "[GameState]") {
    GameState state;
    state.startGame();

    REQUIRE(state.ships.size() == 2);
    REQUIRE(state.players.size() == 2);
    REQUIRE(state.players.front().hand.size() == 7);
    REQUIRE(state.players.front().deck.size() == 40 - 7);

    SECTION("Changes array tests") {
        auto actions = state.getPossibleActions();
        Action playCard;
        for (auto a : actions) {
            if (a.type == ACTION_PLAY_CARD) {
                playCard = a;
                break;
            }
        }
        state.performAction(playCard);
        auto changes = state.getChangesAfter(0);
        REQUIRE(changes[0].type == CHANGE_PLAY_CARD);
        REQUIRE(get<int>(changes[0].data) == playCard.id);
    }
}
