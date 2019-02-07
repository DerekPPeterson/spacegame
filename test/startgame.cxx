#include "testutil.h"


using namespace subprocess;

int main()
{
    LocalServerStarter server;

    auto app1 = Popen({"./app", 
            "-u", "player1", 
            "-l", "spacegame1.log"
            });
    usleep(4e6);
    auto app2 = Popen({"./app", 
            "-u", "player2", 
            "--joinuser", "player1",
            "-l", "spacegame2.log"
            });

    app1.wait();
    app2.wait();

    return 0;
}
