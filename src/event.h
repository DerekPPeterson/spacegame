#ifndef EVENT_H
#define EVENT_H

#include <memory>
#include <vector>
#include <list>

#include "nocopy.h"

enum EventType
{
    EVENT_NORMAL,
    EVENT_SYSTEM_CLICK, // data will be a std::pair representing i,j coords of 
                        // clicked system
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
        template <class T>
        static std::vector<std::shared_ptr<T>> getEvents(EventType type);

        /* Get the data only for the latest event of the given type
         * If clearOthers it will delete the other events of the given type
         * as well
         * Will return a shared_ptr to nullptr if no event
         */
        static std::shared_ptr<std::pair<unsigned int, unsigned int>> 
            getLatestEvent(EventType type, bool clearOthers=false);
    protected:
        /* Private constructor, event type is only used internally */
        Event(EventType type, std::shared_ptr<void> data) :
            type(type), data(data) {};

        EventType type; 
        std::shared_ptr<void> data;

        static std::list<std::shared_ptr<Event>> events;
        template <class T>
        static std::shared_ptr<T> _getLatestEvent(EventType type, bool clearOthers=false);
};

#endif
