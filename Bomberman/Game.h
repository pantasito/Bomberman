// ☕ Привет
#pragma once

#include <iostream> 

#include <ctime> 

#include <string>
#include <vector> 
#include <queue>
#include <map>

#include <chrono> 

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

    // ANTODO Add structure Bomberman { BoManBonuses, lives, _bo_man_coords}

    struct GameStatus {
        bool _is_game_over = false;
        bool _are_you_won  = false;
    };

    class Game {
        Field _field;

        GameStatus   _game_status;
        BoManBonuses _bonuses; // ANTODO _bo_man_bonuses
        
        int _lives;

        std::vector<Object::Bomb>  _bombs;
        std::vector<Object::Enemy> _enemies;

        Object::Point _bo_man_coords = kStartPoint;

        std::vector<std::pair<FieldObject, std::function<void()>>> _bonuses_types; // ANTODO class (2 static vectors (cool and common bonuses)
        int a = 0;

        std::vector<Object::Point> GenerateWalls(int walls_count);

        void GenerateMagicDoor(std::vector<Object::Point>& walls);

        void GenerateBonuses(std::vector<Object::Point>& walls);

        void GenerateEnemies(int enemies_num); 
        
        Point GetNewDirection(const Object::Enemy& enemy) const;
        
        bool IsEnemyKeepDirection(const Object::Enemy& enemy) const;

        void MoveEnemies();

        void ReduceOneLifeAndMoveToStart();

        void BlowReadyBombs();

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