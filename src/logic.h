#ifndef LOGIC_H
#define LOGIC_H

#include <vector>
#include <list>
#include <string>
#include <map>
#include <memory>
#include <functional>
#include <variant>
#include <tuple>

#include <cereal/archives/json.hpp>
#include <cereal/archives/portable_binary.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/variant.hpp>
#include <cereal/types/list.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/complex.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/functional.hpp>
#include <cereal/types/common.hpp>
#include <cereal/types/utility.hpp>

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

bool operator == (const ResourceAmount a, const ResourceAmount b);
ResourceAmount operator + (ResourceAmount a, const ResourceAmount& b);
ResourceAmount operator - (ResourceAmount a, const ResourceAmount& b);
bool operator >= (const ResourceAmount& a, const ResourceAmount& b);
bool operator <= (const ResourceAmount& a, const ResourceAmount& b);

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

    struct Ship;
    inline void DEFAULT_SHIP_NOTHING(GameState& state, Ship& ship) {};

    enum ShipKind {
        SHIP_NORMAL,
        SHIP_RESOURCE,
        SHIP_FLAGSHIP,
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

        ShipKind kind = SHIP_NORMAL;

        void applyDamage(int damage);
        bool isDestroyed() {return armour <= 0;};

        function<void(GameState&, Ship&)> upkeep = DEFAULT_SHIP_NOTHING;

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
        ResourceAmount provides; // Used to determine color of card if cost is not present
        int playedBy = 0;
        int ownerId = 0;
        vector<int> targets;

        function<void(GameState&)> resolve = DEFAULT_CARD_RESOLVE;
        function<tuple<int, int, vector<Target>>(GameState&)> getValidTargets;
        friend ostream & operator << (ostream &out, const Card &c) {
            out << "(Card: " << c.name << " id " << c.id << ")";
            return out;
        }
        SERIALIZE(id, name, cardText, cost, provides, playedBy, targets, ownerId);
    };

    struct Player : public GameObject
    {
        string name;
        ResourceAmount resources;
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

        pair<int, Card> draw();
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
        ACTION_SELECT_SHIPS,
        ACTION_SELECT_SYSTEM,
    };

    struct Action
    {
        ActionType type;
        int playerId;

        int id; // used if there is a single relevant id

        // Used if there are multiple possible targets
        vector<int> targets;
        int minTargets;
        int maxTargets;

        string description;
        friend ostream & operator << (ostream &out, const Action &c);
        SERIALIZE(type, playerId, id, targets, minTargets, maxTargets, description);
    };

    enum ChangeType
    {
        CHANGE_ADD_SHIP,    // data will be ship that was added
        CHANGE_SHIP_CHANGE, // data will be ship changed (id of ship already exists)
        CHANGE_REMOVE_SHIP, // data will be ship that was removed
        CHANGE_PLAY_CARD,   // data will be cardId of card that was played
        CHANGE_RESOLVE_CARD,   // data will be cardId of card that was resolved
        CHANGE_DRAW_CARD,   // data will be pair of playerid, Card
        CHANGE_PHASE_CHANGE, // data will be turnInfo
        CHANGE_PLACE_BEACON, // data will be WarpBeacon object
        CHANGE_PLAYER_RESOURCES, // data will be pair of playerId, ResourceAmount
        CHANGE_MOVE_SHIP,  // data will be pair of shipId, new systemId
        CHANGE_COMBAT_START,  // data will be systemId of system that started combat
    };

    struct Change
    {
        int changeNo = 0;
        ChangeType type;
        variant<
            Ship,
            int,
            Card,
            Player,
            TurnInfo,
            pair<int, Card>,
            WarpBeacon,
            pair<int, ResourceAmount>,
            pair<int, int>
            > data;
        SERIALIZE(changeNo, type, data);
        friend ostream & operator << (ostream &out, const Change &c) {
            out << "(Change: no " << c.changeNo << " type: " << c.type << ")";
            return out;
        }
    };

    struct GameState
    {
        void startGame();
        vector<Action> getPossibleActions(int playerId = 0);
        void performAction(Action);

        void writeStateToFile(string filename);
        void print();
        SERIALIZE(turnInfo, players, ships, systems, beacons, stack, changes);

        friend ostream & operator << (ostream &out, const GameState &c);

        TurnInfo turnInfo;

        Player* getPlayerById(int id);
        Ship* getShipById(int id);
        void deleteShipById(int id);
        vector<Ship*> getShipsBySystem(int id);
        System* getSystemById(int id);
        System* getSystemByPos(int i, int j);
        Card* getCardById(int id);
        WarpBeacon* getBeaconById(int id);
        
        vector<Change> getChangesAfter(int changeNo = 0);

        list<Player> players;
        list<Ship> ships;
        list<System> systems;
        list<WarpBeacon> beacons;

        list<Card> stack;

        vector<Change> changes;

        private:
            void playCard(int cardId, int playerId);
            void resolveStackTop();
            void placeBeacon(int systemId, int ownerId);
            void moveShipsToBeacon(int beaconId, vector<int> ships);
            void endTurn();
            void upkeep(bool firstTurn=false);
            void updateSystemControllers();
            void resolveCombats();

            vector<Action> getValidCardActions();
            vector<Action> getValidBeaconActions();
            vector<Action> getTargetActions();
            vector<Action> getValidShipsForBeacon(int beaconId);

            set<int> shipCanReach(int shipId);
    };
};

#endif
