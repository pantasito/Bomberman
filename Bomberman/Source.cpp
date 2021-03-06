﻿// ☕ Привет

#include "Game.h"
#include <conio.h> 

using namespace Bomberman;

int main() {
    srand((int)(time(0)));
    Game game(7, 15);

    std::thread _game_action_thread(&Game::Run, &game);

    char command;
    do
    {
        command = _getch();

        if (command == 47)
        {
            game.Stop();
            break;
        }

        switch (command)
        {
        case 'w': game.MoveBoMan(Direction::Up); break;
        case 'd': game.MoveBoMan(Direction::Right); break;
        case 's': game.MoveBoMan(Direction::Down); break;
        case 'a': game.MoveBoMan(Direction::Left); break;
        case ' ': game.DropBomb(); break;
        case 'q': game.SetBombsTimerToBlowNow(); break;
        }
    } while (!game.IsGameOver());

    _game_action_thread.join();
}
