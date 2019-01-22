#ifndef GAMELOGIC_H
#define GAMELOGIC_H

#include <memory>
#include <vector>
#include <optional>
#include "drawable.h"
#include "renderer.h"
#include "logic.h"

#include "cards.h"

class GraphicsObjectHandler
{
    public:
        GraphicsObjectHandler() {};

        void startGame(logic::GameState initialState);

        void setPossibleActions(std::vector<logic::Action> actions) 
        {
            this->actions = actions;
            if (actions.size() == 1) {
                selectedAction = actions[0];
            }
        };
        std::optional<logic::Action> getSelectedAction();

        void updateState(std::vector<logic::Change>);

        /* Get all the currently active game objects
         */
        std::vector<std::shared_ptr<Object>> getObjects();

        /* Get the game objects that are also renderables
         */
        std::vector<std::shared_ptr<Renderable>> getRenderables();

        /* This is called every frame to check for click events and such
         */
        void checkEvents();

    private:
        int playerId;

        void initializePlayer(logic::Player);
        void addObject(std::shared_ptr<Object> object);

        std::shared_ptr<Object> getObject(int logicId);
        std::vector<std::shared_ptr<Object>> objects;
        std::map<int, int> index;

        std::vector<logic::Action> actions;
        std::optional<logic::Action> selectedAction;
    
        // common objects that need to be referred to:
        std::shared_ptr<Stack> stack;
        std::shared_ptr<Discard> discard;
        std::shared_ptr<Hand> hand;

        std::shared_ptr<Discard> enemyDiscard;
        std::shared_ptr<Hand> enemyHand;
};

#endif

