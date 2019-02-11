#include "testutil.h"


using namespace subprocess;

int main(int argc, char **argv)
{
    LocalServerStarter server;

    std::string width = "710";
    std::string height = "400";

    if (argc == 3) {
        width = argv[1];
        height = argv[2];
    }

    auto app1 = Popen({ 
            "./app",
            "-u", "player1", 
            "-l", "spacegame1.log",
            "-w", width.c_str(), "-h", height.c_str(),
            });
    usleep(1e6);
    auto app2 = Popen({
            "./app",
            "-u", "player2", 
            "--joinuser", "player1",
            "-l", "spacegame2.log",
            "-w", width.c_str(), "-h", height.c_str(),
            });

    app1.wait();
    app2.wait();

    return 0;
}
