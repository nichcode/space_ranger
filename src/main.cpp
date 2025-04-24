
#include "game.h"

// src files
#include "game.cpp"
#include "player.cpp"
#include "bullet.cpp"
#include "group.cpp"
#include "meteor.cpp"
#include "lives.cpp"
#include "state_machine.cpp"
#include "title_state.cpp"
#include "game_state.cpp"

int main(int argc, char** argv)
{
    Game game;
    game.Init();

    game.Run();
    game.Shutdown();
    return 0;
}