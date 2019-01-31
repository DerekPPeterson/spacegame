#include "testutil.h"


using namespace subprocess;

int main()
{
    LocalServerStarter server;

    auto app1 = Popen({"./app", "-u", "player1"});
    auto app2 = Popen({"./app", "-u", "player2", "--joinuser", "player1"});

    app1.wait();
    app2.wait();

    return 0;
}
