#include "testutil.h"


using namespace subprocess;

int main()
{
    LocalServerStarter server;

    char width[] = "710";
    char height[] = "400";

    auto app1 = Popen({"./app", 
            "-u", "player1", 
            "-l", "spacegame1.log",
            "-w", width, "-h", height,
            });
    usleep(2e6);
    auto app2 = Popen({"./app", 
            "-u", "player2", 
            "--joinuser", "player1",
            "-l", "spacegame2.log",
            "-w", width, "-h", height,
            });

    app1.wait();
    app2.wait();

    return 0;
}
