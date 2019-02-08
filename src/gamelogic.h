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
        GraphicsObjectHandler(Camera& camera) : camera(camera) {};

        void startGame(logic::GameState initialState, int myPlayerId);

        void setPossibleActions(std::vector<logic::Action> actions);

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
        Camera& camera;

        int playerId;

        void initializePlayer(logic::Player);
        void addObject(std::shared_ptr<Object> object);
        void removeObjects();

        std::shared_ptr<Object> getObject(int logicId);
        std::vector<std::shared_ptr<Object>> objects;
        std::map<int, int> index;
        
        std::vector<logic::Action> actions;
        std::optional<logic::Action> selectedAction;

        void updateSysInfoActiveButtons();
    
        // common objects that need to be referred to:
        std::shared_ptr<Stack> stack;
        std::shared_ptr<Discard> discard;
        std::shared_ptr<Hand> hand;

        std::shared_ptr<Discard> enemyDiscard;
        std::shared_ptr<Hand> enemyHand;

        std::shared_ptr<TurnIndicator> turnIndicator;

        std::shared_ptr<Button> passButton;
        std::shared_ptr<Button> confirmButton;

        std::shared_ptr<DebugInfo> debugInfo;

        std::shared_ptr<ResourceDisplay> myResources;
        std::shared_ptr<ResourceDisplay> enemyResources;

        std::map<int, std::shared_ptr<SystemInfo>> sysInfos;

        std::set<int> shipIdsSelected;

        // Change response functions
        void resolveCard(logic::Change change);
        void addShip(logic::Change change);
        void drawCard(logic::Change change);
        void phaseChange(logic::Change change);
        void playCard(logic::Change change);
        void changePlayerResources(logic::Change change);
        void moveShip(logic::Change change);
        void removeShip(logic::Change change);
        void shipChange(logic::Change change);
        void combatStart(logic::Change change);
};

#endif

