// ☕ Привет

#include "Game.h"

using namespace Bomberman;


int main() {
        srand((int)(time(0)));
        //Game game(17, 20);
        Game game(17, 50);

        std::thread _game_action_thread(&Game::Run, &game);

        char command;
        do
        {
            //game.Print(); 

            command = _getch();

            if (command == 27)
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
     //     case 'q': game.BlowAllBombsNow(); break;
            }
        } while (!game.IsGameOver());

        _game_action_thread.join();
}