#ifndef LOGIC_H
#define LOGIC_H

#include <vector>
#include <list>
#include <string>
#include <map>
#include <memory>
#include <functional>

#include <cereal/archives/json.hpp>
#include <cereal/archives/portable_binary.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/list.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/complex.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/functional.hpp>

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

// A type used to keep track of all resource types
typedef std::map<ResourceType, int> ResourceAmount;

// The size of the spacegrid to create
#define SPACEGRID_SIZE 3

namespace logic {

    using namespace std;

    struct GameState;

    /* Used to assign a unique id number to each game object when it is 
     * created
     * */
    struct GameObject {
        GameObject() {
            id = curId++;
        };
        
        /* Call after copying game object if the copy needs to be a new object
         */
        void newId() {
            id = curId++;
        };
        int id;
        static int curId;
    };

    /* System object, representing a valid location for ship, structures
     * etc to be placed */
    struct System : public GameObject
    {
        int controllerId = 0;
        bool home = false;
        int i, j;
        vector<int> adjacent;
        SERIALIZE(id, controllerId, adjacent, home, i, j);
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
            out << "(Ship: " << c.type << " in system " << c.curSystemId
                << " owned by " << c.owner << ")";
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

    inline void DEFAULT_CARD_RESOLVE(GameState& state) {
        LOG_ERROR << "Resolving a card with a default resolve function";
    }

    enum TargetType {
        TARGET_SYSTEM,
        TARGET_SHIP,
    };
    struct Target
    {
        TargetType type;
        int id;
        SERIALIZE(type, id);
    };

    struct Card : public GameObject
    {
        string name;
        string cardText;
        ResourceAmount cost;
        int playedBy = 0;
        vector<int> targets;

        function<void(GameState&)> resolve = DEFAULT_CARD_RESOLVE;
        function<pair<int, vector<Target>>(GameState&)> getValidTargets;
        friend ostream & operator << (ostream &out, const Card &c) {
            out << "(Card: " << c.name << " id " << c.id << ")";
            return out;
        }
        SERIALIZE(id, name, cardText, cost, playedBy, targets);
    };

    struct Player : public GameObject
    {
        string name;
        map<ResourceType, Resource> resources;
        list<Card> deck;
        list<Card> hand;
        list<Card> discard;
        int flagshipId;
        SERIALIZE(id, name, resources, deck, hand, discard, flagshipId);

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

    enum ChangeType
    {
        CHANGE_ADD_SHIP,
        CHANGE_REMOVE_SHIP,
        CHANGE_PLAY_CARD,
        CHANGE_DRAW_CARD,
    };

    struct Change
    {
        int changeNo;
        ChangeType type;
        vector<int> ids;
        SERIALIZE(changeNo, type, ids);
    };

    struct GameState
    {
        void startGame();
        vector<Action> getPossibleActions();
        void performAction(Action);

        void writeStateToFile(string filename);
        void print();
        SERIALIZE(turnInfo, players, ships, systems, beacons, stack, changes);

        friend ostream & operator << (ostream &out, const GameState &c);

        TurnInfo turnInfo;

        Player* getPlayerById(int id);
        Ship* getShipById(int id);
        System* getSystemById(int id);
        System* getSystemByPos(int i, int j);
        Card* getCardById(int id);
        WarpBeacon* getBeaconById(int id);
        
        vector<Change> getChangesAfter(int changeNo);

        void playCard(int cardId, int playerId);
        void resolveStackTop();
        void placeBeacon(int systemId, int ownerId);
        void endTurn();

        vector<Action> getValidCardActions();
        vector<Action> getValidBeaconActions();
        vector<Action> getTargetActions();

        list<Player> players;
        list<Ship> ships;
        list<System> systems;
        list<WarpBeacon> beacons;

        list<Card> stack;

        vector<Change> changes;
    };
};

#endif