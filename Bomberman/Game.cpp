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
            _field.Add(FieldObject::IncreasingNumberOfBombs, walls.back());
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
                _field.Add(FieldObject::Enemy, point);
                _enemies.emplace_back(point, kMoveDeltas[rand() % 4]);
                ++generated_enemies_counter;
            }
        }
    }
    
    Point Game::GetNewDirection(const Object::Enemy& enemy) const {
        const Point current_direction = enemy._direction_of_movement;

        //	const auto prev_dir = _direction_of_movement_of_enemy[num_in_enemy_coords_vector];
        // ANTODO сгенерить перестановку {1, 2, 3}, пробежать по ней фором 

        // не придумал пока способ хорошо сгенерировать такую перестановку, сделал по-другому По идее по сложности не хуже 
        std::vector <Point> possible_directions;

        for (int i = 0; i < 4; ++i) {
            if (kMoveDeltas[i] == current_direction) {
                continue;
            }
            
            const Point tested_position = enemy._current_coords + kMoveDeltas[i];
            if (_field.IsOnField(tested_position) && _field.IsEmpty(tested_position)) {
                possible_directions.push_back(kMoveDeltas[i]);
            }
        }
        if (possible_directions.empty()) {
            return current_direction;
        }
        return possible_directions[rand() % possible_directions.size()];
    }
    
    bool Game::IsEnemyKeepDirection(const Object::Enemy& enemy) const {
        return ((rand() % 100) > (enemy.number_of_moves_made_in_one_direction * kProbabilityOfChangeDirectionAfterOneMove));
    }

    void Game::MoveEnemies() {
        for (auto& enemy : _enemies) {

            const Point coordinates = enemy._current_coords + enemy._direction_of_movement;
            if (_field.IsOnField(coordinates) && _field.IsEmpty(coordinates) && IsEnemyKeepDirection(enemy)) {
                _field.Remove(FieldObject::Enemy, enemy._current_coords);
                enemy.MoveInCurrentDirection();
                _field.Add(FieldObject::Enemy, enemy._current_coords);
                continue;
            }

            const Point old_direction = enemy._direction_of_movement;
            const Point new_direction = GetNewDirection(enemy);
            if (old_direction == new_direction) {
                continue;
            }
            _field.Remove(FieldObject::Enemy, enemy._current_coords);
            enemy.UpdateDirectionAndMove(new_direction);
            _field.Add(FieldObject::Enemy, enemy._current_coords);
        }
    }

    void Game::ReduceOneLifeAndMoveToStart() {
        _field.Remove(FieldObject::BoMan, _bo_man_coords);
        --_lives;

        if (_lives == 0) {
           _game_status._is_game_over = true;
            return;
        }

        _bo_man_coords = kStartPoint;
        _field.Set(FieldObject::BoMan, _bo_man_coords);
    }

    void Game::BlowReadyBombs() {
        const auto it_to_ready_bombs = std::partition(_bombs.begin(), _bombs.end(), [cur_time = time(0)](const Object::Bomb& bomb) {return bomb._time_of_explosion > cur_time; });
        
        for (auto it = it_to_ready_bombs; it != _bombs.end(); ++it) {
            BombBlowUp(*it);
            _field.Remove(FieldObject::Bomb, it->_point);
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
        
        _lives = kNumberOfLivesAtTheStart;
    }

    void Game::SetBombsTimerToBlowNow() {
        if (_bonuses._is_detonate_bomb_at_touch_of_button_activated == false) {
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
            _game_status._is_game_over = true;
            _game_status._are_you_won = true;
            return;
        }

        if (_field.IsIn(FieldObject::IncreaseBombBlastRadius, point)) {
            ++_bonuses._bomb_blast_radius;
            _field.Remove(FieldObject::IncreaseBombBlastRadius, point);
            return;
        }

        if (_field.IsIn(FieldObject::IncreasingNumberOfBombs, point)) {
            ++_bonuses._max_bomb_num;
            _field.Remove(FieldObject::IncreasingNumberOfBombs, point);
            return;
        }

        if (_field.IsIn(FieldObject::AbilityToPassThroughWalls, point)) {
            _bonuses._is_your_ability_walk_through_walls_activated = true;
            _field.Remove(FieldObject::AbilityToPassThroughWalls, point);
            return;
        }

        if (_field.IsIn(FieldObject::ImmunityToExplosion, point)) {
            _bonuses._is_your_blast_immunity_activated = true;
            _field.Remove(FieldObject::ImmunityToExplosion, point);
            return;
        }

        if (_field.IsIn(FieldObject::DetonateBombAtTouchOfButton, point)) {
            _bonuses._is_detonate_bomb_at_touch_of_button_activated = true;
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

        if (!_bonuses._is_your_ability_walk_through_walls_activated && (_field.IsIn(FieldObject::Wall, new_pos) || _field.IsIn(FieldObject::Bomb, new_pos))) {
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
        if (_bombs.size() >= _bonuses._max_bomb_num || _field.IsIn(FieldObject::MagicDoor, _bo_man_coords) || _field.IsIn(FieldObject::Wall, _bo_man_coords)) {
            return;
        }

        _bombs.emplace_back(_bo_man_coords, time(0) + kTimeFromPlantingBombToBlowUp);
        _field.Add(FieldObject::Bomb, _bo_man_coords);
    }

    void Game::BombBlowUp(const Object::Bomb& bomb) {
        bool is_bo_man_exploded = false;

        const auto SetIsBoManExploded = [this, &is_bo_man_exploded](Point exploded_point) {
            if (_field.IsIn(FieldObject::BoMan, exploded_point) && _bonuses._is_your_blast_immunity_activated == false) {
                is_bo_man_exploded = true;
            }
        };

        SetIsBoManExploded(bomb._point);

        for (int i = 0; i < 4; ++i) {
            for (int j = 1; j <= _bonuses._bomb_blast_radius; ++j) {
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

                    auto dead_enemy = std::partition(_enemies.begin(), _enemies.end(), [exploded_cell](const Object::Enemy& enemy) { return enemy._current_coords != exploded_cell; });
                    _enemies.erase(dead_enemy, _enemies.end());                    
                    break;
                }
            
                if (_field.IsIn(FieldObject::IncreaseBombBlastRadius, exploded_cell) ||
                    _field.IsIn(FieldObject::IncreasingNumberOfBombs, exploded_cell) ||
                    _field.IsIn(FieldObject::AbilityToPassThroughWalls, exploded_cell) ||
                    _field.IsIn(FieldObject::ImmunityToExplosion, exploded_cell) ||
                    _field.IsIn(FieldObject::DetonateBombAtTouchOfButton, exploded_cell)) {
                    _field.Set(FieldObject::Enemy, exploded_cell);
                    _enemies.emplace_back(exploded_cell, kMoveDeltas[rand() % 4]);
                }

                if (_field.IsIn(FieldObject::MagicDoor, exploded_cell)) {
                    _field.Add(FieldObject::Enemy, exploded_cell);
                    _enemies.emplace_back(exploded_cell, kMoveDeltas[rand() % 4]);
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

        while (!_game_status._is_game_over) {
            CheckAndTakeBonusOrMagicDoor(_bo_man_coords);
            if (time(0) - enemy_move_time > 1) {
                MoveEnemies();
                enemy_move_time = time(0);
            }

            if (!_bombs.empty()) {
                BlowReadyBombs();
            }

            //std::this_thread::sleep_for(std::chrono::milliseconds(500));
            std::system("cls");
            std::cout << "lives: " << _lives << '\n';
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
        std::cout << (_game_status._are_you_won == true ? "YOU WON!" : "YOU LOST!") << std::endl;
        std::system("pause");
    }

    bool Game::IsGameOver() {
        return _game_status._is_game_over;
    }

    void Game::Stop() {
        _game_status._is_game_over = true;
    }
}