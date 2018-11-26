#include "objectupdater.h"
#include <iostream>
#include <stdio.h>

using namespace std;

void ObjectUpdater::updateFunc(int tid)
{
    while(go) {
        printf("Update thread %d waiting to start\n", tid);
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

        printf("Update thread %d processing elements %d -> %d\n", tid, start, end);
        for (int i = start; i < end; i++) {
            objects[i]->update(info);
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
    for (int i = 0; i < nThreads; i++) {
        hasRun = {true, true, true};
        go = true;
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
    this->info = info;
    this->objects = objects;

    cout << "Telling update threads to start" << endl;
    lock_guard<mutex> lk(m);
    for (int i = 0; i < nThreads; i++) {
        hasRun[i] = false;
    }
    m.unlock();
    cstart.notify_all();
}

void ObjectUpdater::waitForUpdates()
{
    cout << "Waiting for update threads to be done" << endl;
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
