#include "event.h"

using namespace std;

std::list<shared_ptr<Event>> Event::events;

void Event::triggerEvent(EventType type, std::shared_ptr<void> data)
{
    events.push_back(shared_ptr<Event>(new Event(type, data)));
}

std::vector<std::shared_ptr<void>> Event::getEvents(EventType type)
{
    vector<shared_ptr<void>> ret;
    for (auto it = events.begin(); it != events.end(); it++) {
        if (it->get()->type == type) {
            ret.push_back(it->get()->data);
            events.erase(it);
        }
    }

    return ret;
}
