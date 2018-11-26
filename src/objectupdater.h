#ifndef OBJECTUPDATER_H
#define OBJECTUPDATER_H

#include <vector>
#include <thread>
#include <memory>

#include "drawable.h"
#include "nocopy.h"

#include <mutex>
#include <condition_variable>

class ObjectUpdater : public non_copyable
{
    public:
        ObjectUpdater(int nThreads);
        ~ObjectUpdater();
        /** Run the update function for every object in the given array.
         * objects mush not be modified until waitForUpdates is called
         */
        void updateObjects(UpdateInfo info, std::vector<std::shared_ptr<Object>> objects);
        void waitForUpdates();
    private:
        std::vector<std::shared_ptr<Object>> objects;
        UpdateInfo info;

        int nThreads;
        std::vector<std::thread> threads;
        std::vector<bool> hasRun;
        std::mutex m;
        std::condition_variable cstart, cfinish;
        bool go;

        void updateFunc(int tid);
};


#endif

