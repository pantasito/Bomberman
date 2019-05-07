#include <iostream> 

#include <conio.h> 

#include <ctime> 

#include <string>
#include <vector> 
#include <queue>

#include <chrono> 

#include <thread> 
#include <mutex> 

#include <memory> 

#include "Field.h" 
#include "Object/Bomb.h"
#include "Object/Enemy.h"

namespace Bomberman
{
    static const int kTimeFromPlantingBombUntilItsExplosion = 3;

    static const int kProbabilityBonusOfOneType = 40;

    static const int kNumberOfLivesAtTheStart = 3;

    static const Point kBombermanPositionAtTheBeginningOfTheGame(0, 0);  // ANTODO start_point

    static const std::vector<Point> kMoveDeltas = { Point(0,-1), Point(-1,0), Point(0,1), Point(1,0) };


    enum class Direction : int {
        Left = 0,
        Up = 1,
        Right = 2,
        Down = 3
    };

    // ANTODO move to perivate
    bool AreAllCellsInThisFieldAvailable(const Field& field, Point start, int number_of_indestructible_walls) {
        //std::queue<Point> current_cells_to_check; //ANTODO
        std::queue<Point> i_use_this_to_mark_all_available_cells;
        
        std::vector<std::vector<bool>> marked_cells(field.ColsCount(), std::vector<bool>(field.RowsCount(), false));

        i_use_this_to_mark_all_available_cells.push(start);
        int num_of_tested_cells = 1;

        while (!i_use_this_to_mark_all_available_cells.empty()) {
            Point available_cell = i_use_this_to_mark_all_available_cells.front();
            i_use_this_to_mark_all_available_cells.pop();
           // field.Add(FieldObject::Tested, available_cell);
            marked_cells[available_cell._row_num][available_cell._col_num] = true;

            for (int i = 0; i < kMoveDeltas.size(); ++i) {
                const Point cell = available_cell + kMoveDeltas[i];

                if (!field.IsOnField(cell)) {
                    continue;
                }

                //if (field.IsIn(FieldObject::Tested, cell)) {
                if (marked_cells[cell._row_num][cell._col_num]) {
                    continue;
                }

                if (field.IsIn(FieldObject::IndestructibleWall, cell)) {
                    continue;
                }

                //field.Add(FieldObject::Tested, cell);
                marked_cells[cell._row_num][cell._col_num] = true;

                i_use_this_to_mark_all_available_cells.push(cell);
                ++num_of_tested_cells;
            }
        }

        if (num_of_tested_cells + number_of_indestructible_walls != field.ColsCount() * field.RowsCount()) {
            return false;
        }

        return true;
    }


    class Game {
        Field _field;

        // ANTODO - разбить на структурки
        // ANTODO - все булы начинай с 
        bool _game_over = false;
        bool _you_won = false;

        int _cur_lives;

        bool _ability_to_pass_through_walls = false;
        bool _immunity_to_explosions = false;
        bool _detonate_bomb_at_touch_of_button = false;

        int _bomb_blast_radius = 1;
        int _max_bomb_num = 1;
        //int _cur_bomb_count = 0; // ANTODO выплить
        //std::vector<Bomb> _bombs;
        //std::vector<Enemies> _enemies;


        Point _bo_man_coords = kBombermanPositionAtTheBeginningOfTheGame;

        // ANTODO сделать класс Enemy
        std::vector<Point> _enemies_coords;
        std::vector<Point> _direction_of_movement_of_enemy; // ANTODO _direction_delta

        // ANTODO убрать по возможности

        //std::vector<Bomb> _bombs;
        //std::vector<PlantedBomb> _planted_bombs;

        void GenerateIndestructibleWalls(int IndestructibleWallsCount) { // ANTODO code style
            int indestructible_walls_generated = 0;

            while (indestructible_walls_generated < IndestructibleWallsCount) {
                const Point point(rand() % _field.RowsCount(), rand() % _field.ColsCount());

                if (_field.IsOnField(point)) {
                    _field.Set(FieldObject::IndestructibleWall, point);
                    ++indestructible_walls_generated;
                }
            }
            _field.Set(FieldObject::Empty, Point(1, 1)); // ANTODO
        }

        std::vector<Point> GenerateWalls(int WallsCount) { // ANTODO code stylew
            std::vector<Point> walls;

            while (walls.size() < WallsCount) {
                const Point point(rand() % _field.RowsCount(), rand() % _field.ColsCount());
                if (_field.IsEmpty(point)) {
                    _field.Set(FieldObject::Wall, point);
                    walls.push_back(point);
                }
            }
            return walls;
        }

        void GenerateMagicDoor(std::vector<Point> walls) { // ANTODO
            if (!walls.empty()) {
                _field.Add(FieldObject::MagicDoor, walls.back());
                walls.pop_back();
                return;
            }

            // Карта без стен, выход на новый уровенть нужно поставить куда угодно, кроме 
            // неразрушаемой стены и бомбермена. Т.е. ставим на бонус или врага или пустую клетку
            while (true) {
                const Point point(rand() % _field.RowsCount(), rand() % _field.ColsCount());
                if (_field.IsIn(FieldObject::IndestructibleWall, point) || _field.IsIn(FieldObject::BoMan, point)) {
                    continue;
                }
                _field.Add(FieldObject::MagicDoor, point);
                return;
            }
        }

        void GenerateBonuses(std::vector<Point> walls) {
            const int num_of_bonuses_of_one_type = (int)(walls.size() / kProbabilityBonusOfOneType);
            
            assert(walls.size() >= 2 * num_of_bonuses_of_one_type + 3);

            // ANTODO переделать систему бонусов, чтоб не было мноо ифов (ПОТОМ)
            for (int i = 0; i < num_of_bonuses_of_one_type; ++i) {
                _field.Add(FieldObject::IncreasingNumberOfBombsDeliveredAtTime, walls.back());
                walls.pop_back();
                _field.Add(FieldObject::IncreaseBombBlastRadius, walls.back());
                walls.pop_back();
            }

            //по одному крутому бонусу
            if (walls.size() > 3) {
                _field.Add(FieldObject::AbilityToPassThroughWalls, walls.back());
                walls.pop_back();
                _field.Add(FieldObject::ImmunityToExplosion, walls.back());
                walls.pop_back();
                _field.Add(FieldObject::DetonateBombAtTouchOfButton, walls.back());
                walls.pop_back();
            }
            //со скоростью движения бомбермена не справился
        }

        /*
        void GenerateEnemies(int enemies_num) {
            int generated_enemies_counter = 0;
            while (generated_enemies_counter < enemies_num) {
                const Point point(rand() % _field.RowsCount(), rand() % _field.ColsCount());

                if (_field.IsEmpty(point)) {
                    _field.Add(Enemy, point);
                    _enemies_coords.push_back(point);
                    ++generated_enemies_counter;
                }
            }
        }
        */
        /*
        void GenerateDirectionOfEnemyMovement() { // ANTODO
            for (int i = 0; i < _enemies_coords.size(); ++i) {
                for (int j = 0; j < 4; ++j) {
                    const Point possible_direction = _enemies_coords[i] + kMoveDeltas[j];
                    if (IsIsolated(possible_direction)) {
                        _direction_of_movement_of_enemy.emplace_back(0, 0);
                        break;
                    }
                    if (!_field.IsOnField(possible_direction)) {
                        continue;
                    }

                    _direction_of_movement_of_enemy.push_back(kMoveDeltas[j]);
                    break;
                }
            }
        }
        */

        bool IsIsolated(Point enemy) const {
            for (int i = 0; i < 4; ++i) {
                const Point point = enemy + kMoveDeltas[i];
                if (!_field.IsOnField(point)) {
                    continue;
                }

                // ANTODO вынести в функцию 
                if (!(_field.IsIn(FieldObject::Wall, point) || _field.IsIn(FieldObject::IndestructibleWall, point) || _field.IsIn(FieldObject::Enemy, point) || _field.IsIn(FieldObject::Bomb, point))) {
                    return false;
                }
            }
            return true;
        }

        Point RefreshNewPosAndDirection(Point point, int num_in_enemy_coords_vector) {

            static const Direction kPosibleDirection[4][3] =
            {
                {Direction::Up, Direction::Right, Direction::Down},
                {Direction::Left, Direction::Right, Direction::Down},
                {Direction::Up, Direction::Left, Direction::Down},
                {Direction::Up, Direction::Right, Direction::Left},
            };

            //	const auto prev_dir = _direction_of_movement_of_enemy[num_in_enemy_coords_vector];

                // ANTODO сгенерить перестановку {1, 2, 3}, пробежать по ней фором 

            while (true) {
                //int dir = (int)kPosibleDirection[prev_dir][rand() % 3];
                int dir = rand() % 4;

                const Point new_pos = point + kMoveDeltas[dir];
                if (!_field.IsOnField(new_pos)) {
                    continue;
                }
                if (IsIsolated(new_pos)) {
                    return point;
                }
                if (_field.IsIn(FieldObject::Wall, new_pos) || _field.IsIn(FieldObject::IndestructibleWall, new_pos) || _field.IsIn(FieldObject::Bomb, new_pos) || _field.IsIn(FieldObject::Enemy, new_pos)) {
                    continue;
                }
                //_direction_of_movement_of_enemy[num_in_enemy_coords_vector] = kMoveDeltas[dir];
                return point + kMoveDeltas[dir];
            }
        }

        void MoveEnemies() {
            // Хочу чтоб враг двигался по прямой до упора,
            // с шансом 20% менял направление
            // двигался до упора

            for (int i = 0; i < _enemies.size(); ++i) {
                //_enemy.PossibleChangeDirection(); 
                // ANTODO -
                // 0) Вызовем функцию, которая поменяет или не поменяет направление 
                // 1) пусть сначала проверяется что враг может подвуниться и если может двигай
                // 2) GetNewDirection - вернет новый дирешн, если вернет тот же самый который был, то это зачит, что двигаться некуда
                // 2.1) Если вернулась то же направление то continue
                // 2.2) Если вренулось другое, то пересетоваем дельту и хоим безусловно
                /*

                if (IsIsolated(_enemies_coords[i])) {
                    continue;
                }

                Point new_pos = _enemies_coords[i] + _direction_of_movement_of_enemy[i];
                if (!_field.IsOnField(new_pos)) {
                    new_pos = RefreshNewPosAndDirection(_enemies_coords[i], i);
                }

                if (_field.IsIn(Wall, new_pos) || _field.IsIn(IndestructibleWall, new_pos) || _field.IsIn(Enemy, new_pos) || _field.IsIn(Bomb, new_pos)) {
                    new_pos = RefreshNewPosAndDirection(_enemies_coords[i], i);
                }

                if (_field.IsIn(BoMan, new_pos)) {
                    ReduceCurLifeByOneAndMoveBoManToStart();
                }

                _field.Remove(Enemy, _enemies_coords[i]);
                _field.Add(Enemy, new_pos);
                _enemies_coords[i] = new_pos;
            }
            */
            /*
        //добиваюсь того, чтоб враг не стоял на месте

        for (auto& enemy_coords : _enemies_coords) {
            while (true) {
                if (IsIsolated(enemy_coords)) {
                    break;
                }
                int dir = rand() % 4;
                const Point new_pos = kMoveDeltas[dir] + enemy_coords;

                if (!_field.IsOnField(new_pos)) {
                    continue;
                }

                if (_field.IsIn(Wall, new_pos) || _field.IsIn(IndestructibleWall, new_pos)
                    || _field.IsIn(Bomb, new_pos) || _field.IsIn(Enemy, new_pos)) {
                    continue;
                }

                if (_field.IsIn(BoMan, new_pos)) {
                    MinusOneLife();
                }

                // Представь что враг всего 1. Даже так, я покажу 2 строчки, а ты сам подумай что может быть

                _field.Remove(Enemy, enemy_coords); // представб что первый поток выполнил эту операцию, но не приступил к следующей
                _field.Add(Enemy, new_pos);
                enemy_coords = new_pos;
                break;
            }
        }
        */
            }
        }

        void ReduceCurLifeByOneAndMoveBoManToStart() {
            _field.Remove(FieldObject::BoMan, _bo_man_coords);
            --_cur_lives;

            if (_cur_lives == 0) {
                _game_over = true;
                return;
            }

            _bo_man_coords = kBombermanPositionAtTheBeginningOfTheGame;
            _field.Set(FieldObject::BoMan, _bo_man_coords);
        }
        /*
        void ExplosionTimeController() {

            if (_bombs.empty()) {
                return;
            }
            const auto cur_time = time(0);
            // ANTODO std::partition
            for (auto& bomb : _bombs) {
                if (cur_time - bomb._time >= time_from_planting_bomb_until_its_explosion) {
                    BombBlowUp(bomb);
                }
            }
        }
        */
    public:
        Game(int rows_count, int cols_count)
            : _field(rows_count, cols_count) {
            const int number_of_objects = (int)(_field.RowsCount() * _field.ColsCount() * 0.16); //ANTODO
            //const int number_of_objects = (int)(_field.RowsCount() * _field.ColsCount() * 0.16); //ANTODO

            do
            {
                _field.Clear();
                GenerateIndestructibleWalls(number_of_objects);
            } while (!AreAllCellsInThisFieldAvailable(_field, kBombermanPositionAtTheBeginningOfTheGame, number_of_objects));

            _field.Set(FieldObject::BoMan, kBombermanPositionAtTheBeginningOfTheGame);

            std::vector<Point> walls = GenerateWalls(number_of_objects);
            GenerateMagicDoor(walls);
            GenerateBonuses(walls);

            //GenerateEnemies(_field.RowsCount());
            //GenerateDirectionOfEnemyMovement();
            _cur_lives = kNumberOfLivesAtTheStart;
        }
        /*
        void BlowAllBombsNow() {
            if (_detonate_bomb_at_touch_of_button == false) {
                return;
            }

            for (auto bomb : _planted_bombs) {
                BombBlowUp(bomb);
            }
        }
        */
        void GetInfoFromThisPoint(Point point) { // ANTODO плохое название
            if (_field.IsIn(FieldObject::Empty, point)) { // ANTODO всегда возвращает false
                return;
            }

            if (_field.IsIn(FieldObject::Wall, point)) {
                return;
            }

            if (_field.IsIn(FieldObject::MagicDoor, point)) {
                if (!_enemies_coords.empty()) {
                    return;
                }
                _game_over = true;
                _you_won = true;
                return;
            }

            if (_field.IsIn(FieldObject::IncreaseBombBlastRadius, point)) {
                ++_bomb_blast_radius;
                _field.Remove(FieldObject::IncreaseBombBlastRadius, point);
                return;
            }

            if (_field.IsIn(FieldObject::IncreasingNumberOfBombsDeliveredAtTime, point)) {
                ++_max_bomb_num;
                _field.Remove(FieldObject::IncreasingNumberOfBombsDeliveredAtTime, point);
                return;
            }

            if (_field.IsIn(FieldObject::AbilityToPassThroughWalls, point)) {
                _ability_to_pass_through_walls = true;
                _field.Remove(FieldObject::AbilityToPassThroughWalls, point);
                return;
            }

            if (_field.IsIn(FieldObject::ImmunityToExplosion, point)) {
                _immunity_to_explosions = true;
                _field.Remove(FieldObject::ImmunityToExplosion, point);
                return;
            }

            if (_field.IsIn(FieldObject::DetonateBombAtTouchOfButton, point)) {
                _detonate_bomb_at_touch_of_button = true;
                _field.Remove(FieldObject::DetonateBombAtTouchOfButton, point);
                return;
            }
        }

        void MoveBoMan(Direction direction) {
            const auto new_pos = _bo_man_coords + kMoveDeltas[static_cast<int>(direction)];

            if (!_field.IsOnField(new_pos)) {
                return;
            }

            if (_field.IsIn(FieldObject::IndestructibleWall, new_pos)) {
                return;
            }

            // ANTODO сначала легче проверить флаг
            if ((_field.IsIn(FieldObject::Wall, new_pos) || _field.IsIn(FieldObject::Bomb, new_pos)) && _ability_to_pass_through_walls == false) {
                return;
            }

            // ..понтяно и пиши понял в чем лажа частично. Поятно, что нужно сделать так что б там обе функции срабатывали оследовательно
            // все. Да, тут можно на разных уронях синхронизировать. МОжно на уровне целых функций
            // Т.е. пока работает MoveBoMan, не могут работать функции из Run.
            // Можно чуть более локально синхронизацию сделать, чтоб операция движения
            // параллельно с этим второй поток выполняет этот иф. Первый поток убрал врага с поля, но на новое место его еще не поставил
             // вот вторая	Допустим враг один. В этой строчке может сщлучиться так, что 
                            // врагов на поле вообще не будет и это как бы не очень правильно


            if (_field.IsIn(FieldObject::Enemy, new_pos)) {
                ReduceCurLifeByOneAndMoveBoManToStart();
                return;
            }

            // Т.е. чтоб эти 3 строчки выполнялись атомарно
            //что-то типа lock()
            _field.Add(FieldObject::BoMan, new_pos);
            _field.Remove(FieldObject::BoMan, _bo_man_coords);
            _bo_man_coords = new_pos;
            //unlock(); ?..
        }
        /*
        void DropBomb() {
            if (_cur_bomb_count >= _max_bomb_num) {
                return;
            }

            _planted_bombs.emplace_back(_bo_man_coords, time(0));
            _field.Add(Bomb, _bo_man_coords);
            ++_cur_bomb_count;
        }

        void BombBlowUp(PlantedBomb bomb_explodes_at_the_moment) {
            bool _bo_man_exploded = false;
            if (_field.IsIn(BoMan, bomb_explodes_at_the_moment._point) && _immunity_to_explosions == false) {
                _bo_man_exploded = true;
            }

            if (_field.IsIn(MagicDoor, bomb_explodes_at_the_moment._point)) {
                _field.Add(Enemy, bomb_explodes_at_the_moment._point);
                _enemies_coords.push_back(bomb_explodes_at_the_moment._point);
                _direction_of_movement_of_enemy.push_back(kMoveDeltas[rand() % 4]);
            }


            for (int i = 0; i < 4; ++i) {
                for (int j = 1; j <= _bomb_blast_radius; ++j) {
                    const auto exploded_cell = bomb_explodes_at_the_moment._point + kMoveDeltas[i] * j;

                    if (!_field.IsOnField(exploded_cell) || _field.IsIn(IndestructibleWall, exploded_cell)) {
                        break;
                    }

                    if (_field.IsIn(BoMan, exploded_cell) && _immunity_to_explosions == false) {
                        _bo_man_exploded = true;
                    }

                    if (_field.IsIn(Wall, exploded_cell)) {
                        _field.Remove(Wall, exploded_cell);
                        break;
                    }

                    if (_field.IsIn(Enemy, exploded_cell)) {
                        _field.Remove(Enemy, exploded_cell);

                        for (int i = 0; i < _enemies_coords.size(); ++i) { // ANTODO задуматься о более быстром поиске
                            if (_enemies_coords[i] == exploded_cell) {
                                _direction_of_movement_of_enemy[i] = _direction_of_movement_of_enemy.back();
                                _enemies_coords[i] = _enemies_coords.back();

                                _enemies_coords.pop_back();
                                _direction_of_movement_of_enemy.pop_back();
                                break;
                            }
                        }
                    }

                    /*
                        IncreaseBombBlastRadius = 64,					//r
                        IncreasingNumberOfBombsDeliveredAtTime = 128,	//n
                        AbilityToPassThroughWalls = 256,				//a
                        ImmunityToExplosion = 512,						//g
                        RunningSpeed = 1024,							//s
                        DetonateBombAtTouchOfButton = 2048,				//q
                    */
                    /*
                                if (_field.IsIn(IncreaseBombBlastRadius, exploded_cell) || _field.IsIn(IncreasingNumberOfBombsDeliveredAtTime, exploded_cell) ||
                                    _field.IsIn(AbilityToPassThroughWalls, exploded_cell) || _field.IsIn(ImmunityToExplosion, exploded_cell) ||
                                    _field.IsIn(DetonateBombAtTouchOfButton, exploded_cell)) {
                                    _field.Set(Enemy, exploded_cell);
                                    _enemies_coords.push_back(exploded_cell);
                                    _direction_of_movement_of_enemy.push_back(kMoveDeltas[rand() % 4]);
                                }

                                if (_field.IsIn(MagicDoor, exploded_cell)) {
                                    _field.Add(Enemy, exploded_cell);
                                    _enemies_coords.push_back(exploded_cell);
                                }

                                if (_field.IsIn(Bomb, exploded_cell)) {
                                    for (int i = 0; i < _planted_bombs.size(); ++i) {
                                        if (_planted_bombs[i]._point == exploded_cell) {
                                            _planted_bombs[i]._time = time(0) - time_from_planting_bomb_until_its_explosion;
                                            break;
                                        }
                                    }
                                }
                            }
                        }
                        _field.Remove(Bomb, bomb_explodes_at_the_moment._point);

                        for (auto& bomb : _planted_bombs) {
                            if (bomb == bomb_explodes_at_the_moment) {
                                bomb = _planted_bombs.back();
                                _planted_bombs.pop_back();
                            }
                        }

                        --_cur_bomb_count;
                        if (_bo_man_exploded) {
                            MinusOneLife();
                        }
                    }
                    */

        void Print() {
            std::system("cls");
            _field.Print();
        }

        void Run() {
            auto enemy_move_time = time(0);

            while (!_game_over) {
                //GetInfoFromThisPoint(_bo_man_coords);
                //if (time(0) - enemy_move_time > 1) {
                //    MoveEnemies();
                 //   enemy_move_time = time(0);
                //}

    //            ExplosionTimeController();
               // std::system("cls");

            //    std::cout << "lives: " << _lives;
            //    std::cout << " max bomb num: " << _max_bomb_num;
            //    std::cout << " bomb blast radius: " << _bomb_blast_radius << "  ";
      //          if (_planted_bombs.size() == 1) {
      //              std::cout << "timer: " << time(0) - _planted_bombs[0]._time << "/" << 3;
       //         }
                //std::cout << std::endl;

                _field.Print();

                std::system("cls");
            }
            std::system("cls");
            std::cout << (_you_won == true ? "YOU WON!" : "YOU LOST!") << std::endl;
            std::system("pause");
        }

        bool IsGameOver() {
            return _game_over;
        }

        /*

        auto enemy_move_time = time(0);

        while (!_game_over) {
            // ANTODO при нажатии на was d не двигать бомермена, а помнить только клавишу и двигать тут
            GetInfoFromThisPoint(_bo_man_coords);
            //CheckBomberManPosi();
            if (time(0) - enemy_move_time > 1) {
                MoveEnemies();
                enemy_move_time = time(0);
            }

            //ExplosionTimeController();
            std::system("cls");

            std::cout << "lives: " << _lives;
            std::cout << " max bomb num: " << _max_bomb_num;
            std::cout << " bomb blast radius: " << _bomb_blast_radius << "  ";
            /*
                if (_planted_bombs.size() == 1) {
                    std::cout << "timer: " << time(0) - _planted_bombs[0]._time << "/" << 3;
                }
                std::cout << std::endl;

                _field.Print();
            }
            */
            /*
                std::system("cls");
                std::cout << (_you_won == true ? "YOU WON!" : "YOU LOST!") << std::endl;
                std::system("pause");
            }
            */

        void Stop() {
            _game_over = true;
        }
    };

    /*
    std::mutex cout_mutex;

    void f(char symbol)
    {
        for (int i = 0; i < 100; i++)
        {
            cout_mutex.lock();
            std::cout << symbol << " - " << i << std::endl;
            cout_mutex.unlock();
        }
    }

    void g()
    {
        for (int i = 0; i < 100; i++)
        {
            std::cout << "g - " << i << std::endl;
        }
    }
    */
}

#include <ios>

int main() {
        /*
        std::cout << "Helloooooooooooooooooooooooooooooooooooo";
        std::cout.seekp(5, std::ios_base::beg);
        std::cout << '1';
        std::cout.flush();
        */

        /*
        std::thread _f_thread(f, 'f');
        f('g');
        _f_thread.join(); // ��������� ���������� _f_thread
        std::system("pause");
        return 0;
        */
        /*
        for (int i = 0; i < 256; i++)
        {
        std::cout << i << "-" << (char)i << std::endl;
        }
        system("pause");
        */

       // Bomberman::Object::Bomb b{ {0.3,0.5}, 14 };

        srand((int)(time(0)));
        //Game game(17, 20);
        Bomberman::Game game(17, 67);

        std::thread _game_action_thread(&Bomberman::Game::Run, &game);

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
            case 'w': game.MoveBoMan(Bomberman::Direction::Up); break;

            case 'd': game.MoveBoMan(Bomberman::Direction::Right); break;

            case 's': game.MoveBoMan(Bomberman::Direction::Down); break;

            case 'a': game.MoveBoMan(Bomberman::Direction::Left); break;

     //     case ' ': game.DropBomb(); break;
     //     case 'q': game.BlowAllBombsNow(); break;
            }
        } while (!game.IsGameOver());

        _game_action_thread.join();


	/*
	std::cout << "Helloooooooooooooooooooooooooooooooooooo";
	std::cout.seekp(5, std::ios_base::beg);
	std::cout << '1';
	std::cout.flush();
	*/

	/*
	std::thread _f_thread(f, 'f');
	f('g');
	_f_thread.join(); // Подождать завершения _f_thread
	std::system("pause");
	return 0;
	*/
	/*
	for (int i = 0; i < 256; i++)
	{
	std::cout << i << "-" << (char)i << std::endl;
	}
	system("pause");
	*/
    /*
	srand((int)(time(0)));
	//Game game(17, 20);
	Game game(17, 35);

	std::thread _game_action_thread(&Game::Run, &game);

	char command;
	do
	{
		// game.Print(); 

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
	//	case ' ': game.DropBomb(); break;
	//	case 'q': game.BlowAllBombsNow(); break;
		}
	} while (!game.GameOver());

	_game_action_thread.join();
    */
}