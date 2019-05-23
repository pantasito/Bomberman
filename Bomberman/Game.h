// ☕ Привет

#pragma once

#include <iostream> 

#include <ctime> 
#include <chrono> 

#include <string>
#include <vector> 
#include <queue>
#include <map>

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
#include "Object/Bomberman.h"

namespace Bomberman
{
    using Object::Point;
    
    static const int kTimeFromPlantingBombToBlowUp = 3;

    static const double kProbabilityBonusOfOneType = 0.05;

    static const double kProbabilityOfWallCreation = 0.16;

    static const int kProbabilityOfChangeDirectionAfterOneMove = 5;

    static const std::vector<Point> kMoveDeltas = { Point(-1, 0), Point(0, 1), Point(1, 0), Point(0,-1) };

    enum class Direction : char {
        Up = 0,
        Right = 1,
        Down = 2,
        Left = 3
    };

    static const Direction kPosibleDirection[4][3] =
    {
        {Direction::Up  , Direction::Right, Direction::Down},
        {Direction::Left, Direction::Right, Direction::Down},
        {Direction::Up  , Direction::Left , Direction::Down},
        {Direction::Up  , Direction::Right, Direction::Left},
    };

    struct GameStatus {
        bool _is_game_over = false;
        bool _are_you_won  = false;
    };
    
    class Game {
        Field _field;

        GameStatus         _game_status;
        Object::Bomberman  _bomberman;

        std::vector<Object::Bomb>  _bombs;
        std::vector<Object::Enemy> _enemies;
        
        std::vector<std::pair<FieldObject, std::function<void()>>> _bonuses_types;
        static const int _bitmask_all_bonus_types = static_cast <int>(FieldObject::AbilityToPassThroughWalls) | static_cast <int>(FieldObject::DetonateBombAtTouchOfButton) |
                                                    static_cast <int>(FieldObject::ImmunityToExplosion)       | static_cast <int>(FieldObject::IncreaseBombBlastRadius) | 
                                                    static_cast <int>(FieldObject::IncreasingNumberOfBombs);
        

        static const int _bitmask_field_objects_enemy_unable_to_stay = static_cast <int>(FieldObject::Wall)  | static_cast <int>(FieldObject::IndestructibleWall) | 
                                                                       static_cast <int>(FieldObject::Enemy) | static_cast <int>(FieldObject::Bomb);


        bool AreAllCellsAvailable(int indestructible_walls_count) const;

        std::vector<Object::Point> GenerateWalls(int walls_count);
        void GenerateIndestructibleWalls(int indestructible_walls_count);
        void GenerateMagicDoor(std::vector<Object::Point>& walls);
        void GenerateBonuses(std::vector<Object::Point>& walls);
        void GenerateEnemies(int enemies_count);
        
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

        void CheckAndTakeBonusOrMagicDoor();

        void MoveBoMan(Direction direction);

        void DropBomb();

        void BombBlowUp(const Object::Bomb& bomb);

        void Run();

        bool IsGameOver();

        void Stop();
    };
}