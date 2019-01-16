#ifndef GAMELOGIC_H
#define GAMELOGIC_H

#include <memory>
#include <vector>
#include <optional>
#include "drawable.h"
#include "renderer.h"
#include "logic.h"

class GraphicsObjectHandler
{
    public:
        GraphicsObjectHandler() {};

        void startGame(logic::GameState initialState);

        void setPossibleActions(std::vector<logic::Action> actions) {};
        std::optional<logic::Action> getSelectedAction() {return {};};

        void updateState(std::vector<logic::Change>) {};;

        /* Get all the currently active game objects
         */
        std::vector<std::shared_ptr<Object>> getObjects();

        /* Get the game objects that are also renderables
         */
        std::vector<std::shared_ptr<Renderable>> getRenderables();

    private:
        void initializePlayer(logic::Player);
        void addObject(std::shared_ptr<Object> object);
        std::shared_ptr<Object> getObject(int logicId);
        std::vector<std::shared_ptr<Object>> objects;
        std::map<int, int> index;
};

#endif

