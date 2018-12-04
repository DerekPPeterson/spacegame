#ifndef EVENT_H
#define EVENT_H

#include <memory>
#include <vector>
#include <list>

#include "nocopy.h"

enum EventType
{
    EVENT_NORMAL,
    EVENT_SYSTEM_CLICK,
};

class Event : public non_copyable
{
    public:
        /* Register a new event
         * @type  enum defined above, should indicate the kind of data
         *        expected by whatever checks that kind of event
         * @data  data of some kind related to the event
         */
        static void triggerEvent(EventType type, std::shared_ptr<void> data);
        /* Get the data associated with all events of the provided type 
         */
        static std::vector<std::shared_ptr<void>> getEvents(EventType type);
    protected:
        /* Private constructor, event type is only used internally */
        Event(EventType type, std::shared_ptr<void> data) :
            type(type), data(data) {};

        EventType type; 
        std::shared_ptr<void> data;

        static std::list<std::shared_ptr<Event>> events;
};

#endif
