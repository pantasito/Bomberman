// ☕ Привет

#include "Game.h"

namespace Bomberman
{
    bool Game::AreAllCellsAvailable(int number_of_indestructible_walls) {
        assert(_field.IsEmpty(kStartPoint));

        std::queue<Object::Point> cells_to_check;

        std::vector<std::vector<bool>> marked_cells(_field.RowsCount(), std::vector<bool>(_field.ColsCount(), false));

        cells_to_check.push(kStartPoint);
        int num_of_tested_cells = 1;
        marked_cells[kStartPoint._row_num][kStartPoint._col_num] = true;

        while (!cells_to_check.empty()) {
            const auto cell_to_check = cells_to_check.front();
            cells_to_check.pop();

            for (int i = 0; i < kMoveDeltas.size(); ++i) {
                const auto cell = cell_to_check + kMoveDeltas[i];

                if (!_field.IsOnField(cell)) {
                    continue;
                }

                if (marked_cells[cell._row_num][cell._col_num]) {
                    continue;
                }

                if (_field.IsIn(FieldObject::IndestructibleWall, cell)) {
                    continue;
                }

                marked_cells[cell._row_num][cell._col_num] = true;
                cells_to_check.push(cell);
                ++num_of_tested_cells;
            }
        }

        if (num_of_tested_cells + number_of_indestructible_walls != _field.ColsCount() * _field.RowsCount()) {
            return false;
        }

        return true;
    }

    void Game::GenerateIndestructibleWalls(int number_of_indestructible_walls)
    {
        int indestructible_walls_generated = 0;

        while (indestructible_walls_generated < number_of_indestructible_walls) {
            const Object::Point point(rand() % _field.RowsCount(), rand() % _field.ColsCount());

            if (point == kStartPoint || _field.IsIn(FieldObject::IndestructibleWall, point)) {
                continue;
            }

            _field.Set(FieldObject::IndestructibleWall, point);
            ++indestructible_walls_generated;
        }

        assert(indestructible_walls_generated == number_of_indestructible_walls);
    }

    std::vector<Object::Point> Game::GenerateWalls(int walls_count) {
        std::vector<Object::Point> walls;

        while (walls.size() < walls_count) {
            const Object::Point point(rand() % _field.RowsCount(), rand() % _field.ColsCount()); //не делаю проверку IsOnField
            if (_field.IsEmpty(point)) {
                _field.Set(FieldObject::Wall, point);
                walls.push_back(point);
            }
        }
        return walls;
    }

    void Game::GenerateMagicDoor(std::vector<Object::Point>& walls) {
        if (!walls.empty()) {
            _field.Add(FieldObject::MagicDoor, walls.back());
            walls.pop_back();
            return;
        }

        // Карта без стен, выход на новый уровень нужно поставить куда угодно, кроме неразрушаемой стены и бомбермена.
        while (true) {
            const Object::Point point(rand() % _field.RowsCount(), rand() % _field.ColsCount());
            if (_field.IsIn(FieldObject::IndestructibleWall, point) || _field.IsIn(FieldObject::BoMan, point)) {
                continue;
            }
            _field.Add(FieldObject::MagicDoor, point);
            return;
        }
    }

    void Game::GenerateBonuses(std::vector<Object::Point>& walls) {
        const int num_of_bonuses_of_one_type = (int)(walls.size() * kProbabilityBonusOfOneType);

        assert(walls.size() >= 2 * num_of_bonuses_of_one_type + 3);

        // ANTODO переделать систему бонусов, чтоб не было много ифов (ПОТОМ)
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

    void Game::GenerateEnemies(int enemies_num) {
        int generated_enemies_counter = 0;
        while (generated_enemies_counter < enemies_num) {
            const Point point(rand() % _field.RowsCount(), rand() % _field.ColsCount());
            
            if (_field.IsEmpty(point)) {
                const Point dir = kMoveDeltas[rand() % 4];
                _field.Add(FieldObject::Enemy, point);
                _enemies.emplace(Object::Enemy(point, dir), true);
                ++generated_enemies_counter;
            }
        }
    }

       /*
             void Game::GenerateDirectionOfEnemyMovement() { // ANTODO
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

    bool Game::IsIsolated(Object::Point enemy) const {
        for (int i = 0; i < 4; ++i) {
            const Object::Point point = enemy + kMoveDeltas[i];
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

    Object::Point Game::RefreshNewPosAndDirection(Object::Point point, int num_in_enemy_coords_vector) {

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

            const Object::Point new_pos = point + kMoveDeltas[dir];
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

    void Game::MoveEnemies() {
        // Хочу чтоб враг двигался по прямой до упора,
        // с шансом 20% менял направление
        // двигался до упора

        //  for (int i = 0; i < _enemies.size(); ++i) {
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
    //  }
    }

    void Game::ReduceOneLifeAndMoveToStart() {
        _field.Remove(FieldObject::BoMan, _bo_man_coords);
        --_cur_lives;

        if (_cur_lives == 0) {
            is_game_over = true;
            return;
        }

        _bo_man_coords = kStartPoint;
        _field.Set(FieldObject::BoMan, _bo_man_coords);
    }

   

    void Game::ExplosionsController() {
        const auto it_to_ready_bombs = std::partition(_bombs.begin(), _bombs.end(), [cur_time = time(0)](const Object::Bomb& bomb) {return bomb._time_of_explosion > cur_time; });

        for (auto it = it_to_ready_bombs; it != _bombs.end(); ++it) {
            BombBlowUp(*it_to_ready_bombs);
            _field.Remove(FieldObject::Bomb, it_to_ready_bombs->_point);
        }

        _bombs.erase(it_to_ready_bombs, _bombs.end());
    }

    Game::Game(int rows_count, int cols_count)
        : _field(rows_count, cols_count) {
        const int number_of_indestructible_walls = (int)(_field.RowsCount() * _field.ColsCount() * kProbabilityOfWallCreation);
        const int number_of_walls = (int)(_field.RowsCount() * _field.ColsCount() * kProbabilityOfWallCreation);

        do
        {
            _field.Clear();
            GenerateIndestructibleWalls(number_of_indestructible_walls);
        } while (!AreAllCellsAvailable(number_of_indestructible_walls));

        _field.Set(FieldObject::BoMan, kStartPoint);

        auto walls = GenerateWalls(number_of_walls);
        GenerateMagicDoor(walls);
        GenerateBonuses(walls);

        GenerateEnemies(_field.RowsCount());
        //GenerateDirectionOfEnemyMovement();
        _cur_lives = kNumberOfLivesAtTheStart;
    }

    void Game::SetBombsTimerToBlowNow() {
        if (_is_detonate_bomb_at_touch_of_button_activated == false) {
            return;
        }

        //могу переставить всем бомбам время взрыва..они взорвуться из-за работы ExplosionTimeController() 
        //кажется где-то взрыв записан, как BlowUp, а где-то, как Eхposion
        time_t cur_time = time(0);
        for (auto& bomb : _bombs) {
            bomb._time_of_explosion = cur_time;
        }
    }

    void Game::CheckAndTakeBonusOrMagicDoor(Object::Point point) {

        if (_field.IsIn(FieldObject::Wall, point)) {
            return;
        }

        if (_field.IsIn(FieldObject::MagicDoor, point)) {
            if (!_enemies.empty()) {
                return;
            }
            is_game_over = true;
            are_you_won = true;
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
            _is_your_ability_walk_through_walls_activated = true;
            _field.Remove(FieldObject::AbilityToPassThroughWalls, point);
            return;
        }

        if (_field.IsIn(FieldObject::ImmunityToExplosion, point)) {
            _is_your_blast_immunity_activated = true;
            _field.Remove(FieldObject::ImmunityToExplosion, point);
            return;
        }

        if (_field.IsIn(FieldObject::DetonateBombAtTouchOfButton, point)) {
            _is_detonate_bomb_at_touch_of_button_activated = true;
            _field.Remove(FieldObject::DetonateBombAtTouchOfButton, point);
            return;
        }
    }

    void Game::MoveBoMan(Direction direction) {
        const auto new_pos = _bo_man_coords + kMoveDeltas[static_cast<int>(direction)];

        if (!_field.IsOnField(new_pos)) {
            return;
        }

        if (_field.IsIn(FieldObject::IndestructibleWall, new_pos)) {
            return;
        }

        if (!_is_your_ability_walk_through_walls_activated && (_field.IsIn(FieldObject::Wall, new_pos) || _field.IsIn(FieldObject::Bomb, new_pos))) {
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
            ReduceOneLifeAndMoveToStart();
            return;
        }

        // Т.е. чтоб эти 3 строчки выполнялись атомарно
        //что-то типа lock()
        _field.Add(FieldObject::BoMan, new_pos);
        _field.Remove(FieldObject::BoMan, _bo_man_coords);
        _bo_man_coords = new_pos;
        //unlock(); ?..

    }

    void Game::DropBomb() {
        if (_bombs.size() >= _max_bomb_num || _field.IsIn(FieldObject::MagicDoor, _bo_man_coords)) {
            return;
        }

        _bombs.emplace_back(_bo_man_coords, time(0) + kTimeFromPlantingBombToBlowUp);
        _field.Add(FieldObject::Bomb, _bo_man_coords);
    }

    void Game::BombBlowUp(const Object::Bomb& bomb) {
        bool is_bo_man_exploded = false;

        const auto SetIsBoManExploded = [this, &is_bo_man_exploded](Point exploded_point) {
            if (_field.IsIn(FieldObject::BoMan, exploded_point) && _is_your_blast_immunity_activated == false) {
                is_bo_man_exploded = true;
            }
        };

        SetIsBoManExploded(bomb._point);

        for (int i = 0; i < 4; ++i) {
            for (int j = 1; j <= _bomb_blast_radius; ++j) {
                const auto exploded_cell = bomb._point + kMoveDeltas[i] * j;

                if (!_field.IsOnField(exploded_cell) || _field.IsIn(FieldObject::IndestructibleWall, exploded_cell)) {
                    break;
                }

                SetIsBoManExploded(exploded_cell);

                if (_field.IsIn(FieldObject::Wall, exploded_cell)) {
                    _field.Remove(FieldObject::Wall, exploded_cell);
                    break;
                }

                
                if (_field.IsIn(FieldObject::Enemy, exploded_cell)) {
                    _field.Remove(FieldObject::Enemy, exploded_cell);
                   // auto it = _enemies.find(exploded_cell); 
                    //_enemies.erase(it);
                    
                    for (int i = 0; i < _enemies.size(); ++i) { // ANTODO задуматься о более быстром поиске
                    
                    }
                }
            
                if (_field.IsIn(FieldObject::IncreaseBombBlastRadius, exploded_cell) ||
                    _field.IsIn(FieldObject::IncreasingNumberOfBombsDeliveredAtTime, exploded_cell) ||
                    _field.IsIn(FieldObject::AbilityToPassThroughWalls, exploded_cell) ||
                    _field.IsIn(FieldObject::ImmunityToExplosion, exploded_cell) ||
                    _field.IsIn(FieldObject::DetonateBombAtTouchOfButton, exploded_cell)) {
                    _field.Set(FieldObject::Enemy, exploded_cell);
                    _enemies.emplace(exploded_cell, kMoveDeltas[rand() % 4]);
                }

                if (_field.IsIn(FieldObject::MagicDoor, exploded_cell)) {
                    _field.Add(FieldObject::Enemy, exploded_cell);
                    _enemies.emplace(exploded_cell, kMoveDeltas[rand() % 4]);
                }

                if (_field.IsIn(FieldObject::Bomb, exploded_cell)) {
                    for (int i = 0; i < _bombs.size(); ++i) {
                        if (_bombs[i]._point == exploded_cell) {
                            _bombs[i]._time_of_explosion = time(0);
                            break;
                        }
                    }
                }
            }
        }

        if (is_bo_man_exploded) {
            ReduceOneLifeAndMoveToStart();
        }
    }

    void Game::Print() {
        std::system("cls");
        _field.Print();
    }

    void Game::Run() {
        auto enemy_move_time = time(0);

        while (!is_game_over) {
            CheckAndTakeBonusOrMagicDoor(_bo_man_coords);
            //if (time(0) - enemy_move_time > 1) {
            //    MoveEnemies();
             //   enemy_move_time = time(0);
            //}

            if (!_bombs.empty()) {
                ExplosionsController();
            }

            //std::this_thread::sleep_for(std::chrono::milliseconds(500));
            std::system("cls");
            std::cout << "lives: " << _cur_lives << '\n';
            //         std::cout << " max bomb num: " << _max_bomb_num;
           //          std::cout << " bomb blast radius: " << _bomb_blast_radius << "  ";
           //          if (_planted_bombs.size() == 1) {
           //              std::cout << "timer: " << time(0) - _planted_bombs[0]._time << "/" << 3;
            //         }
                     //std::cout << std::endl;

            _field.Print();
            std::cout.flush();
        }
        std::system("cls");
        std::cout << (are_you_won == true ? "YOU WON!" : "YOU LOST!") << std::endl;
        std::system("pause");
    }

    bool Game::IsGameOver() {
        return is_game_over;
    }

    void Game::Stop() {
        is_game_over = true;
    }
}