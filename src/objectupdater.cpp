#include "objectupdater.h"
#include <iostream>
#include <stdio.h>

#include <plog/Log.h>

using namespace std;

void ObjectUpdater::updateFunc(int tid)
{
    while(go) {
        LOG_VERBOSE << "Update thread " << tid <<  " waiting to start";
        {
            unique_lock<mutex> lk(m);
            cstart.wait(lk, [this, tid]{return not hasRun[tid];});
            lk.unlock();
        }
        if (not go) return;

        int batch_size = objects.size() / nThreads;
        int start = tid * batch_size;
        int end = start + batch_size;
        if (tid == nThreads - 1) {
            end += objects.size() % nThreads;
        }

        LOG_VERBOSE << "Update thread " << tid << " processing elements " << 
            start << " -> " << end;
        for (int i = start; i < end; i++) {
            if (objects[i]) {
                objects[i]->update(info);
            }
        }

        {
            lock_guard<mutex> lk(m);
            hasRun[tid] = true;
            cfinish.notify_all();
        }
    }
};

ObjectUpdater::ObjectUpdater(int nThreads) :
    nThreads(nThreads), threads(nThreads)
{
    hasRun = {true, true, true};
    go = true;
    for (int i = 0; i < nThreads; i++) {
        threads[i] = thread([this, i] {updateFunc(i);});
    }
};

ObjectUpdater::~ObjectUpdater()
{
    go = false;
    updateObjects(info, objects);
    for (int i = 0; i < nThreads; i++) {
        threads[i].join();
    }
}

void ObjectUpdater::updateObjects(UpdateInfo info, vector<shared_ptr<Object>> objects)
{
    waitForUpdates();
    
    this->info = info;
    this->objects = objects;

    LOG_VERBOSE << "Telling update threads to start";
    lock_guard<mutex> lk(m);
    for (int i = 0; i < nThreads; i++) {
        hasRun[i] = false;
    }
    m.unlock();
    cstart.notify_all();
}

void ObjectUpdater::waitForUpdates()
{
    LOG_VERBOSE << "Waiting for update threads to be done";
    unique_lock<mutex> lk(m);
    cfinish.wait(lk, [this]{
            bool done = true;
            for (int i = 0; i < nThreads; i++) {
                done = done and hasRun[i];
            }
            return done;
            });
    lk.unlock();
}
