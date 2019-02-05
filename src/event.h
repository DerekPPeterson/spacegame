#ifndef EVENT_H
#define EVENT_H

#include <memory>
#include <vector>
#include <map>
#include <queue>
#include <list>
#include <variant>
#include <string>

#include <plog/Log.h>

#include "nocopy.h"

enum EventType
{
    EVENT_SYSTEM_CLICK, // data will be logicId of the system
    EVENT_DRAW,         // data will be a pair representing playernum/number of cards
    EVENT_PLAY_CARD,    // data will be the logicId of the card
    EVENT_BUTTON_PRESS, // data will be the string of the pressed button
};

typedef std::variant<int, std::pair<int, int>, std::string> EventData;

class Event : public non_copyable
{
    public:
        /* Register a new event
         * @type  enum defined above, should indicate the kind of data
         *        expected by whatever checks that kind of event
         * @data  data of some kind related to the event
         */
        template <class T>
        static void triggerEvent(EventType type, T data)
        {
            EventData d = data;
            events[type].push(d);
        }


        /* Get the data associated with all events of the provided type 
         */
        static std::vector<EventData> getEvents(EventType type);

        /* Get the data only for the latest event of the given type
         * If clearOthers it will delete the other events of the given type
         * as well
         */
        static std::optional<EventData> getNextEvent(EventType type, bool clearOthers=false);
    protected:
        /* Private constructor, event type is only used internally */
        template <class T>
        Event(EventType type, T data) : data(data) {};

        EventData data;

        static std::map<EventType, std::queue<EventData>> events;
};

#endif
