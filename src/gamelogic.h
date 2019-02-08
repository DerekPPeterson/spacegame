#ifndef GAMELOGIC_H
#define GAMELOGIC_H

#include <memory>
#include <vector>
#include <queue>
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

        void updateState(std::vector<logic::Change>, UpdateInfo info);

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

        float timeToProcessNextChange = 0;
        std::queue<logic::Change> pendingChanges;

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
        // All accept change of a particular type and return the number of seconds
        // to act before processing the next change
        float resolveCard(logic::Change change);
        float addShip(logic::Change change);
        float drawCard(logic::Change change);
        float phaseChange(logic::Change change);
        float playCard(logic::Change change);
        float changePlayerResources(logic::Change change);
        float moveShip(logic::Change change);
        float removeShip(logic::Change change);
        float shipChange(logic::Change change);
        float combatStart(logic::Change change);
        float combatRound(logic::Change change);
        float combatEnd(logic::Change change);
};

#endif

