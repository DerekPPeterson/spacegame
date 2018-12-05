#include "event.h"

#include <plog/Log.h>

using namespace std;

std::list<shared_ptr<Event>> Event::events;

void Event::triggerEvent(EventType type, std::shared_ptr<void> data)
{
    events.push_back(shared_ptr<Event>(new Event(type, data)));
}

template <class T>
std::vector<std::shared_ptr<T>> Event::getEvents(EventType type)
{
    vector<shared_ptr<T>> ret;
    for (auto it = events.begin(); it != events.end(); it++) {
        if (it->get()->type == type) {
            ret.push_back(static_pointer_cast<T>(it->get()->data));
            it = events.erase(it);
            it--;
        }
    }

    return ret;
}

template <class T>
std::shared_ptr<T> Event::_getLatestEvent(EventType type, bool clearOthers)
{
    std::shared_ptr<T> ret(nullptr);

    auto it = events.end();
    while (it != events.begin()) {
        it--;
        if (it->get()->type == type) {
            if (ret == nullptr) {
                ret = static_pointer_cast<T>(it->get()->data);
                it = events.erase(it);
                LOG_INFO << "Event (type=" << type << ") was processed";
            } else if (clearOthers) {
                it = events.erase(it);
            }
        }
    }
    return ret;
}

shared_ptr<pair<unsigned int, unsigned int>> Event::getLatestEvent(
        EventType type, bool clearOthers)
{
    return _getLatestEvent<std::pair<unsigned int, unsigned int>>(
                type, clearOthers);
}
