// ☕ Привет

#include "Game.h"

namespace Bomberman
{
    bool Game::AreAllCellsAvailable(int indestructible_walls_count) const {
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

        if (num_of_tested_cells + indestructible_walls_count != _field.ColsCount() * _field.RowsCount()) {
            return false;
        }

        return true;
    }

    void Game::GenerateIndestructibleWalls(int indestructible_walls_count)
    {
        int indestructible_walls_generated = 0;

        while (indestructible_walls_generated < indestructible_walls_count) {
            const Object::Point point(rand() % _field.RowsCount(), rand() % _field.ColsCount());

            if (point == kStartPoint || _field.IsIn(FieldObject::IndestructibleWall, point)) {
                continue;
            }

            _field.Set(FieldObject::IndestructibleWall, point);
            ++indestructible_walls_generated;
        }

        assert(indestructible_walls_generated == indestructible_walls_count);
    }

    std::vector<Object::Point> Game::GenerateWalls(int walls_count) {
        std::vector<Object::Point> walls;

        while (walls.size() < walls_count) {
            const Object::Point point(rand() % _field.RowsCount(), rand() % _field.ColsCount());
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
        const int num_of_regular_bonuses_of_one_type = (int)(walls.size() * kProbabilityBonusOfOneType);

        static const std::vector<FieldObject> kRegularBonuses = { FieldObject::IncreasingNumberOfBombs, FieldObject::IncreaseBombBlastRadius };
        static const std::vector<FieldObject> kCoolBonuses    = { FieldObject::AbilityToPassThroughWalls, FieldObject::ImmunityToExplosion, FieldObject::DetonateBombAtTouchOfButton };
        
        assert(walls.size() >= kRegularBonuses.size() * num_of_regular_bonuses_of_one_type + kCoolBonuses.size());

        for (int i = 0; i < num_of_regular_bonuses_of_one_type; ++i) {
            for (auto bonus_type : kRegularBonuses) {
                _field.Add(bonus_type, walls.back());
                walls.pop_back();
            }
        }

        //по одному крутому бонусу
        if (walls.size() > 3) {
            for (auto bonus_type : kCoolBonuses) {
                _field.Add(bonus_type, walls.back());
                walls.pop_back();
            }
        }
    }

    void Game::GenerateEnemies(int enemies_count) {
        int generated_enemies_count = 0;
        while (generated_enemies_count < enemies_count) {
            const Point point(rand() % _field.RowsCount(), rand() % _field.ColsCount());
            if (_field.IsEmpty(point)) {
                _field.Add(FieldObject::Enemy, point);
                _enemies.emplace_back(point, kMoveDeltas[rand() % 4]);
                ++generated_enemies_count;
            }
        }
    }

    bool Game::IsEnemyCanReachThis(Point point) const {
        return (_field.IsOnField(point) && !_field.AtLeastOneContained(_bitmask_field_objects_enemy_unable_to_stay, point));
    }

    Point Game::GetNewDirection(const Object::Enemy& enemy) const {
        const Point current_direction = enemy._coords_delta;

        char possible_directions[4];
        int possible_directions_count = 0;

        for (int i = 0; i < 4; ++i) {
            if (kMoveDeltas[i] == current_direction) {
                continue;
            }
            
            const Point tested_position = enemy._coords + kMoveDeltas[i];
            if (IsEnemyCanReachThis(tested_position)) {
                possible_directions[possible_directions_count] = i;
                ++possible_directions_count;
            }
        }
        if (possible_directions_count == 0) {
            return current_direction;
        }
 
        return kMoveDeltas[possible_directions[rand() % possible_directions_count]]; 
    }
    
    bool Game::IsEnemyChangeDirection(const Object::Enemy& enemy) const {
        return ((rand() % 100) < (enemy.number_of_moves_made_in_one_direction * kProbabilityOfChangeDirectionAfterOneMove));
    }

    void Game::MoveEnemies() {
        for (auto& enemy : _enemies) {
            const Point new_coordinates = enemy._coords + enemy._coords_delta;
            
            if ((!IsEnemyCanReachThis(new_coordinates) || IsEnemyChangeDirection(enemy))) {
                const Point new_direction = GetNewDirection(enemy);
                if (enemy._coords_delta == new_direction) {
                    continue;
                }
                enemy.UpdateMovementDelta(new_direction);
            }

            _field.Remove(FieldObject::Enemy, enemy._coords);
            enemy.MoveInCurrentDirection();
            _field.Add(FieldObject::Enemy, enemy._coords);

            if (_bomberman._bo_man_coords == enemy._coords) {
                ReduceOneLifeAndMoveToStart();
            }
        }
    }

    void Game::ReduceOneLifeAndMoveToStart() {
        _field.Remove(FieldObject::BoMan, _bomberman._bo_man_coords);
        --_bomberman._lives;

        if (_bomberman._lives == 0) {
           _game_status._is_game_over = true;
            return;
        }

        _bomberman._bo_man_coords = kStartPoint;
        _field.Set(FieldObject::BoMan, _bomberman._bo_man_coords);
    }

    void Game::BlowReadyBombs() {
        const auto it_to_ready_bombs = std::partition(_bombs.begin(), _bombs.end(), [cur_time = time(0)](const Object::Bomb& bomb) 
        {
            return bomb._time_of_explosion > cur_time; 
        });
        
        for (auto it = it_to_ready_bombs; it != _bombs.end(); ++it) {
            BombBlowUp(*it);
            _field.Remove(FieldObject::Bomb, it->_point);
        }

        _bombs.erase(it_to_ready_bombs, _bombs.end());
    }

    void Game::InitializationBonusesTypes() {
        _bonuses_types = {
            { FieldObject::IncreaseBombBlastRadius,     [this]() { ++_bomberman._bonuses._bomb_blast_radius; }},
            { FieldObject::IncreasingNumberOfBombs,     [this]() { ++_bomberman._bonuses._max_bomb_count; }},
            { FieldObject::AbilityToPassThroughWalls,   [this]() {   _bomberman._bonuses._is_walk_through_walls = true; }},
            { FieldObject::ImmunityToExplosion,         [this]() {   _bomberman._bonuses._is_blast_immunity = true; }},
            { FieldObject::DetonateBombAtTouchOfButton, [this]() {   _bomberman._bonuses._is_detonate_bomb_by_button = true; }}
        };
    }

    Game::Game(int rows_count, int cols_count)
        : _field(rows_count, cols_count)
    {
        const int number_of_indestructible_walls = (int)(_field.RowsCount() * _field.ColsCount() * kProbabilityOfWallCreation);
        const int number_of_walls                = (int)(_field.RowsCount() * _field.ColsCount() * kProbabilityOfWallCreation);

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
         
        InitializationBonusesTypes();
    }

    void Game::SetBombsTimerToBlowNow() {
        if (_bomberman._bonuses._is_detonate_bomb_by_button == false) {
            return;
        }

        const time_t cur_time = time(0);
        for (auto& bomb : _bombs) {
            bomb._time_of_explosion = cur_time;
        }
    }

    void Game::CheckAndTakeBonusOrMagicDoor() {
        if (_field.IsIn(FieldObject::Wall, _bomberman._bo_man_coords)) {
            return;
        }

        if (_field.IsIn(FieldObject::MagicDoor, _bomberman._bo_man_coords)) {
            if (!_enemies.empty()) {
                return;
            }
            _game_status._is_game_over = true;
            _game_status._are_you_won = true;
            return;
        }

        if (_field.AtLeastOneContained(_bitmask_all_bonus_types, _bomberman._bo_man_coords)) {
            for (const auto& [bonus, bonus_effect] : _bonuses_types) {
                if (_field.IsIn(bonus, _bomberman._bo_man_coords)) {
                    bonus_effect();
                    _field.Remove(bonus, _bomberman._bo_man_coords);
                    return;
                }
            }
        }
    }

    void Game::MoveBoMan(Direction direction) {
        const auto new_pos = _bomberman._bo_man_coords + kMoveDeltas[static_cast<int>(direction)];

        if (!_field.IsOnField(new_pos)) {
            return;
        }

        if (_field.IsIn(FieldObject::IndestructibleWall, new_pos)) {
            return;
        }

        if (!_bomberman._bonuses._is_walk_through_walls && (_field.IsIn(FieldObject::Wall, new_pos) || _field.IsIn(FieldObject::Bomb, new_pos))) {
            return;
        }

        if (_field.IsIn(FieldObject::Enemy, new_pos)) {
            ReduceOneLifeAndMoveToStart();
            return;
        }

        _field.Add(FieldObject::BoMan, new_pos);
        _field.Remove(FieldObject::BoMan, _bomberman._bo_man_coords);
        _bomberman._bo_man_coords = new_pos;
    }

    void Game::DropBomb() {
        if (_bombs.size() >= _bomberman._bonuses._max_bomb_count || _field.IsIn(FieldObject::MagicDoor, _bomberman._bo_man_coords)
            || _field.IsIn(FieldObject::Wall, _bomberman._bo_man_coords)) {
            return;
        }

        _bombs.emplace_back(_bomberman._bo_man_coords, time(0) + kTimeFromPlantingBombToBlowUp);
        _field.Add(FieldObject::Bomb, _bomberman._bo_man_coords);
    }

    void Game::BombBlowUp(const Object::Bomb& bomb) {
        bool is_bo_man_exploded = false;

        const auto SetIsBoManExploded = [this, &is_bo_man_exploded](Point exploded_point) {
            if (_field.IsIn(FieldObject::BoMan, exploded_point) && _bomberman._bonuses._is_blast_immunity == false) {
                is_bo_man_exploded = true;
            }
        };

        SetIsBoManExploded(bomb._point);

        for (int i = 0; i < 4; ++i) {
            for (int j = 1; j <= _bomberman._bonuses._bomb_blast_radius; ++j) {
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

                    auto exploded_enemy_it = find_if(_enemies.begin(), _enemies.end(), [exploded_cell](const Object::Enemy& enemy) { return enemy._coords == exploded_cell; });
                    assert(exploded_enemy_it != _enemies.end());
                    
                    std::swap(*exploded_enemy_it, _enemies.back());
                    _enemies.pop_back();
                    break;
                }
                
                if (_field.AtLeastOneContained(_bitmask_all_bonus_types, exploded_cell)) {
                    _field.Set(FieldObject::Enemy, exploded_cell);
                    _enemies.emplace_back(exploded_cell, kMoveDeltas[rand() % 4]);
                    break;
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
    
    /*
    class ConsoleCursorSetter
    {
       SetCursor
    }
    */

    void Game::Run() {
        auto enemy_move_time = time(0);

        while (!_game_status._is_game_over) {
            
            CheckAndTakeBonusOrMagicDoor();
            
            if (time(0) - enemy_move_time > 1) {
                MoveEnemies();
                enemy_move_time = time(0);
            }

            if (!_bombs.empty()) {
                BlowReadyBombs();
            }

            _console_cursor_setter.Set();

            std::cout << "lives: " << _bomberman._lives;
            std::cout << "  max bomb num: " << _bomberman._bonuses._max_bomb_count;
            std::cout << "  bomb blast radius: " << _bomberman._bonuses._bomb_blast_radius << std::endl;
           
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