#ifndef TESTUTIL_H
#define TESTUTIL_H

#include <subprocess.hpp>

class LocalServerStarter
{
    public:
        LocalServerStarter() {
            p = new subprocess::Popen({"./server"});
            usleep(1e5);
        }
        ~LocalServerStarter() {
            p->kill();
            delete p;
        }
    private:
        subprocess::Popen *p;

};

#endif
