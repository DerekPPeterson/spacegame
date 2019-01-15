#include "catch.hpp"

#include "logic.h"

using namespace logic;

TEST_CASE("Game start tests", "[GameState]") {
    GameState state;
    state.startGame();

    SECTION("Check counts of game object types") {
        REQUIRE(state.ships.size() == 2);
        REQUIRE(state.players.size() == 2);
        REQUIRE(state.players.front().hand.size() == 7);
        REQUIRE(state.players.front().deck.size() == 40 - 7);
    }
}

