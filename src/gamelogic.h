#ifndef GAMELOGIC_H
#define GAMELOGIC_H

#include <memory>
#include <vector>
#include "drawable.h"
#include "renderer.h"

enum Phase {
    PHASE_NONE,
    PHASES_START_TURN,
    PHASES_MAIN,
    PHASES_RESOLVE_MOVEMENT,
    PHASES_RESOLVE_CARD,
};

struct GameState
{
    Phase phase;
    std::vector<std::shared_ptr<Object>> objects;

    // Indexes into the above vector to keep track of particular types of 
    // objects
    int spacegrid;
    std::vector<int> systems;
    std::vector<int> units;
    int deck;
    int hand;
    std::vector<int> cards;
};

class GameLogic
{
    public:
        GameLogic() {};

        /* Initialize and return a game state
         */
        GameState startGame();

        /* Based on the current state, call the appropriate state function and
         * update the state with the new state returned
         */
        void updateState();

        /* Get all the currently active game objects
         */
        std::vector<std::shared_ptr<Object>> getObjects();

        /* Get the game objects that are also renderables
         */
        std::vector<std::shared_ptr<Renderable>> getRenderables();

    private:
        /* Current game state
         */
        GameState state;

        /* State transition functions all take the current state and return a
         * new state. This could result in new game objects being created
         * or destroyed. Can check and clear events to determine what state
         * changes to make
         */
        GameState startTurn(GameState state) const;
        GameState main(GameState state) const;
        GameState resolveMovement(GameState state) const;
        GameState resoveCard(GameState state) const;
        
};

#endif

