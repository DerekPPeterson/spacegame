#ifndef LOGIC_H
#define LOGIC_H

#include <vector>
#include <string>
#include <map>
#include <memory>

#include <cereal/archives/json.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/complex.hpp>
#include <cereal/types/memory.hpp>

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

    struct System
    {
        int id;
        SERIALIZE(id);
    };

    struct Ship
    {
        string type;
        int attack;
        int shield;
        int armour;
        int movement;

        int owner;
        int controller;

        SERIALIZE(type, attack, shield, armour, movement, controller);
    };

    struct Resource
    {
        int amount;
        int max;
        int perTurn;
        SERIALIZE(amount, max, perTurn);
    };

    typedef map<ResourceType, int> ResourceAmount;
    
    struct Card
    {
        string name;
        ResourceAmount cost;
        SERIALIZE(name, cost);
    };

    struct Player
    {
        string name;
        map<ResourceType, Resource> resources;
        vector<Card> deck;
        vector<Card> hand;
        vector<Card> deleted;
        shared_ptr<Ship> flagship;
        SERIALIZE(name, resources, deck, hand, deleted, flagship);
    };

    enum TurnPhases {
        PHASE_UPKEEP,
        PHASE_MAIN,
        PHASE_MOVE,
        PHASE_END,
    };

    struct TurnInfo
    {
        int whoseTurn;
        int activePlayer;
        TurnPhases phase;
        SERIALIZE(whoseTurn, activePlayer, phase);
    };

    class GameState
    {
        public:
            void startGame();
            void writeStateToFile(string filename);
            SERIALIZE(turnInfo, players, ships, systems);
        protected:
            TurnInfo turnInfo;

            vector<Player> players;
            vector<Ship> ships;
            vector<System> systems;
    };
};

#endif
