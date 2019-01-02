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
    LOG_INFO << "Setting up game";

    // TODO load decks from somewhere
    LOG_INFO << "Loading sample deck";
    for (int i = 0; i < 40; i++) {
        SAMPLE_DECK.push_back({.name="Card", .cost={{RESOURCE_AI, 1}}});
    }

    LOG_INFO << "Creating player objects";
    for (int i = 0; i < 2; i++) {
        // Create player objects
        Player p = {
            .name = "Player" + to_string(i),
            .deck = SAMPLE_DECK,
            .flagship = nullptr,
            .resources = {{RESOURCE_WARP_BEACONS, {.amount = 0, .max=1, .perTurn=1}}},
        };
        p.draw(7);
        players.push_back(p);
        LOG_INFO << "Adding player " << p.name;

        // Give them flagships TODO load chosen flagshipss
        LOG_INFO << "Added flagship for " << p.name;
        logic::Ship sampleFlagship = {
            .type = "Sample Flagship",
            .attack = 1,
            .shield = 10,
            .armour = 10,
            .movement = 1,
            .owner = i,
            .controller = i
        };
        ships.push_back(sampleFlagship);
    }

    turnInfo = {
        .whoseTurn = 0,
        .activePlayer = 0,
        .phase = PHASE_UPKEEP,
    };
};

void Player::draw(int n)
{
    for (int i = 0; i < n; i++) {
        if (not deck.size()) {
            // TODO handle drawing from empty deck
            break;
        };
        LOG_INFO << "Player " << name << " drew a card";
        auto card = deck.back();
        deck.pop_back();
        hand.push_back(card);
    }
};

int main()
{
    plog::init(plog::verbose, "./logic_test.log");

    GameState state;
    state.startGame();
    state.writeStateToFile("testState.json");
}
