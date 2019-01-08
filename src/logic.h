#ifndef LOGIC_H
#define LOGIC_H

#include <vector>
#include <list>
#include <string>
#include <map>
#include <memory>
#include <functional>

#include <cereal/archives/json.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/list.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/complex.hpp>
#include <cereal/types/memory.hpp>

#include <prettyprint.hpp>
#include <plog/Log.h>
#include <plog/Appenders/ColorConsoleAppender.h>

#include <stdarg.h>

#include <backward.hpp>

#define SERIALIZE(...) \
template<class Archive> \
void serialize(Archive & archive) \
{\
    archive(__VA_ARGS__); \
}

enum ResourceType
{
    RESOURCE_ANY,
    RESOURCE_WARP_BEACONS,
    RESOURCE_MATERIALS,
    RESOURCE_AI,
    RESOURCE_ANTIMATTER,
    RESOURCE_INFLUENCE,
};

namespace logic {

    using namespace std;

    class GameState;

    /* Used to assign a unique id number to each game object when it is 
     * created
     * */
    struct GameObject {
        GameObject() {
            static int curId = 1;
            id = curId++;
        };
        int id;
    };

    /* System object, representing a valid location for ship, structures
     * etc to be placed */
    struct System : public GameObject
    {
        SERIALIZE(id);
        vector<int> adjacent;
    };

    /* Ship object, representing a unit that can move and attack other ship
     * units */
    struct Ship : public GameObject
    {
        string type;     // TODO this will be a key into some table
        int attack;      // damage per round
        int shield;      // regenerating hp
        int armour;      // static hp
        int movement;    // Number of systems can move in a single turn

        int owner;       // playerId of owner
        int controller;  // playerId of controller

        int curSystemId; // Current location

        friend ostream & operator << (ostream &out, const Ship &c)
        {
            out << "(Ship: " << c.type << " in system " << c.curSystemId << ")";
            return out;
        }

        SERIALIZE(id, type, attack, shield, armour, movement, controller, 
                curSystemId);
    };

    struct WarpBeacon : public GameObject
    {
        int ownerId;    // Player who owns beacon
        int systemId;   // System id where the beacon is currently placed

        friend ostream & operator << (ostream &out, const WarpBeacon &c)
        {
            out << "(WarpBeacon: in system" << c.systemId << ")";
            return out;
        }
        SERIALIZE(id, ownerId, systemId);
    };

    /* A resource amount, that could be a per turn changing amount
     */
    struct Resource
    {
        int amount;     // Current amount of resource present
        int max;        // Current maximum resource capacity
        int perTurn;    // Resource generation per turn
        SERIALIZE(amount, max, perTurn);
    };

    // A type used to keep track of all resource types
    typedef map<ResourceType, int> ResourceAmount;

    void DEFAULT_CARD_RESOLVE(GameState& state) {
        LOG_ERROR << "Resolving a card with a default resolve function";
    }
    
    struct Card : public GameObject
    {
        string name;
        ResourceAmount cost;
        int playedBy = 0;
        function<void(GameState&)> resolve = DEFAULT_CARD_RESOLVE;
        function<vector<int>(GameState&)> getValidTargets;
        friend ostream & operator << (ostream &out, const Card &c) {
            out << "(Card: " << c.name << " id " << c.id << ")";
            return out;
        }
        SERIALIZE(name, cost);
    };

    struct Player : public GameObject
    {
        string name;
        map<ResourceType, Resource> resources;
        list<Card> deck;
        list<Card> hand;
        list<Card> discard;
        int flagshipId;
        SERIALIZE(name, resources, deck, hand, discard, flagshipId);

        friend ostream & operator << (ostream &out, const Player &c)
        {
            out << "(Player: " << c.name << " id " << c.id << ")";
            return out;
        }

        void draw(int n=1);
    };

    enum TurnPhases {
        PHASE_UPKEEP,
        PHASE_MAIN,
        PHASE_MOVE,
        PHASE_END,
        PHASE_SELECT_CARD_TARGETS,
        PHASE_SELECT_BEACON_TARGETS,
        PHASE_RESOLVE_STACK,
    };

    ostream & operator<< (ostream &out, const TurnPhases &c);

    struct TurnInfo
    {
        int whoseTurn;
        int activePlayer;
        vector<TurnPhases> phase;
        bool selectTargets;
        SERIALIZE(whoseTurn, activePlayer, phase);
    };

    enum ActionType {
        ACTION_NONE,
        ACTION_PLACE_BEACON,
        ACTION_PLAY_CARD,
        ACTION_END_TURN,
        ACTION_SELECT_SHIPS,
        ACTION_SELECT_SYSTEM,
    };

    struct Action
    {
        ActionType type;
        int playerId;
        int id;
        vector<int> targets;
        int nTargets;
        string description;
        friend ostream & operator << (ostream &out, const Action &c);
        SERIALIZE(type, playerId, id, targets, nTargets, description);
    };

    class GameState
    {
        public:
            void startGame();
            vector<Action> getPossibleActions();
            void performAction(Action);

            void writeStateToFile(string filename);
            void print();
            SERIALIZE(turnInfo, players, ships, systems);

            friend ostream & operator << (ostream &out, const GameState &c);

            TurnInfo turnInfo;

            Player* getPlayerById(int id);
            Ship* getShipById(int id);
            System* getSystemById(int id);
            Card* getCardById(int id);
            WarpBeacon* getBeaconById(int id);

            void playCard(int cardId, int playerId);
            void resolveStackTop();
            void placeBeacon(int systemId, int ownerId);
            void endTurn();

            vector<Action> getValidCardActions();
            vector<Action> getValidBeaconActions();

            list<Player> players;
            list<Ship> ships;
            list<System> systems;
            list<WarpBeacon> beacons;

            list<Card> stack;
    };
};

#endif
