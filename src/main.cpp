
#include "game.h"

// src files
#include "game.cpp"
#include "player.cpp"

int main(int argc, char** argv)
{
    Game game;
    game.Init();

    game.Run();
    game.Shutdown();
    return 0;
}