#include "logic.h"

using namespace std;
using namespace logic;

int main()
{

    GameState state;
    state.startGame();
    cerr << state;
    state.writeStateToFile("testState.json");
    cerr << state;

    while (1) {
        auto actions = state.getPossibleActions();
        int i = 0;
        for (auto a : actions) {
            cout << i++ << ": " << a << endl;
        }
        int selection;
        cin >> selection;
        state.performAction(actions[selection]);
        cerr << state;
    }
}
