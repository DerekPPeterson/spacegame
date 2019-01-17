
#include "event.h"

std::map<EventType, std::queue<EventData>> Event::events;

std::vector<EventData> Event::getEvents(EventType type)
{
	std::vector<EventData> ret;
    for (int i = 0; i < events[type].size(); i++) {
        ret.push_back(events[type].front());
        events[type].pop();
    }

    return ret;
}

std::optional<EventData> Event::getNextEvent(EventType type, bool clearOthers)
{
	std::optional<EventData> ret;
    if (events[type].size()) {
        ret = events[type].front();
        events[type].pop();
    }

    if (clearOthers) {
        events[type] = {};
    }

    return ret;
}
