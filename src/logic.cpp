#include "logic.h"

#include <fstream>

using namespace logic;
using namespace std;


// TODO dynamically load deck from somewhere
vector<logic::Card> SAMPLE_DECK;

void GameState::writeStateToFile(string filename)
{
    ofstream ofs(filename);
    cereal::JSONOutputArchive oarchive(ofs);
    oarchive(*this);
}

void GameState::startGame()
{
    // TODO load decks from somewhere
    for (int i = 0; i < 40; i++) {
        SAMPLE_DECK.push_back({.name="Card", .cost={{RESOURCE_AI, 1}}});
    }

    for (int i = 0; i < 2; i++) {
        Player p = {
            .name = "Player",
            .deck = SAMPLE_DECK,
            .flagship = nullptr,
            .resources = {{RESOURCE_WARP_BEACONS, {.amount = 0, .max=1, .perTurn=1}}},
        };
        players.push_back(p);
    }

    turnInfo = {
        .whoseTurn = 0,
        .activePlayer = 0,
        .phase = PHASE_UPKEEP,
    };
};

int main()
{
    GameState state;
    state.startGame();
    state.writeStateToFile("testState.json");
}
