// ☕ Привет
#pragma once

#include <iostream> 

#include <ctime> 

#include <string>
#include <vector> 
#include <queue>
#include <map>

#include <chrono> 

#include <windows.h>

#include <thread> 
#include <mutex> 

#include <utility>
#include <algorithm> 

#include <functional>
#include <memory> 

#include "Field.h" 
#include "Object/Bomb.h"
#include "Object/Enemy.h"
#include "Object/Point.h"

namespace Bomberman
{
    static const int kTimeFromPlantingBombToBlowUp = 3;

    static const double kProbabilityBonusOfOneType = 0.05;

    static const double kProbabilityOfWallCreation = 0.16;

    static const int kNumberOfLivesAtTheStart = 3;

    static const Object::Point kStartPoint(0, 0);

    static const int kProbabilityOfChangeDirectionAfterOneMove = 5;

    static const std::vector<Object::Point> kMoveDeltas = { Object::Point(-1, 0), Object::Point(0, 1), Object::Point(1, 0), Object::Point(0,-1) };

    enum class Direction : char {
        Up = 0,
        Right = 1,
        Down = 2,
        Left = 3
    };

    static const Direction kPosibleDirection[4][3] =
    {
        {Direction::Up, Direction::Right, Direction::Down},
        {Direction::Left, Direction::Right, Direction::Down},
        {Direction::Up, Direction::Left, Direction::Down},
        {Direction::Up, Direction::Right, Direction::Left},
    };

    struct BoManBonuses {
        bool _is_your_ability_walk_through_walls_activated  = false;
        bool _is_your_blast_immunity_activated              = false;
        bool _is_detonate_bomb_at_touch_of_button_activated = false;

        int _bomb_blast_radius = 1;
        int _max_bomb_num      = 1;
    };

    struct GameStatus {
        bool _is_game_over = false;
        bool _are_you_won  = false;
    };

    struct Bomberman {
        BoManBonuses _bonuses;
        int _lives;
        Object::Point _bo_man_coords = kStartPoint;
    };

    class Game {
        Field _field;

        Bomberman  _bomberman;
        GameStatus _game_status;

        std::vector<Object::Bomb>  _bombs;
        std::vector<Object::Enemy> _enemies;

        std::vector<std::pair<FieldObject, std::function<void()>>> _bonuses_types;
        int _bitmask_all_bonus_types;

        //где инициализировать эту переменную?
        static const int _bitmask_field_objects_enemy_unable_to_stay = static_cast <int>(FieldObject::Wall) 
            | static_cast <int>(FieldObject::IndestructibleWall) | static_cast <int>(FieldObject::Enemy) | static_cast <int>(FieldObject::Bomb);

        bool AreAllCellsAvailable(int number_of_indestructible_walls);

        void GenerateIndestructibleWalls(int number_of_indestructible_walls);

        std::vector<Object::Point> GenerateWalls(int walls_count);

        void GenerateMagicDoor(std::vector<Object::Point>& walls);

        void GenerateBonuses(std::vector<Object::Point>& walls);

        void GenerateEnemies(int enemies_num); 
        
        Point GetNewDirection(const Object::Enemy& enemy) const;
        
        bool IsEnemyCanReachThis(Point point) const;
        
        bool IsEnemyChangeDirection(const Object::Enemy& enemy) const;

        void MoveEnemies();

        void ReduceOneLifeAndMoveToStart();

        void BlowReadyBombs();

        void InitializationBonusesTypes();

    public:
        Game(int rows_count, int cols_count);

        void SetBombsTimerToBlowNow();

        void CheckAndTakeBonusOrMagicDoor(Object::Point point);

        void MoveBoMan(Direction direction);

        void DropBomb();

        void BombBlowUp(const Object::Bomb& bomb);

        void Print();

        void Run();

        bool IsGameOver();

        void Stop();
    };
}