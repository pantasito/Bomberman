// ☕ Привет
#pragma once

#include <iostream> 

#include <conio.h> 

#include <ctime> 

#include <string>
#include <vector> 
#include <queue>

#include <chrono> 

#include <thread> 
#include <mutex> 

#include <utility>
#include <algorithm> 

#include <memory> 

#include "Field.h" 
#include "Object/Bomb.h"
#include "Object/Enemy.h"
#include "Object/Point.h"

namespace Bomberman
{
    static const int kTimeFromPlantingBombToBlowUp = 3;

    static const int kProbabilityBonusOfOneType = 0.1;

    static const double kProbabilityOfWallCreation = 0.16;

    static const int kNumberOfLivesAtTheStart = 3;

    static const Object::Point kStartPoint(0, 0);

    static const std::vector<Object::Point> kMoveDeltas = { Object::Point(0,-1), Object::Point(-1,0), Object::Point(0, 1), Object::Point(1, 0) };

    enum class Direction : int {
        Left = 0,
        Up = 1,
        Right = 2,
        Down = 3
    };


    class Game {
        Field _field;

        // ANTODO - разбить на структурки
        // ANTODO - все булы начинай с 
        bool is_game_over = false;
        bool are_you_won = false;

        int _cur_lives;

        bool _is_your_ability_walk_through_walls_activated = false;
        bool _is_your_blast_immunity_activated = false;
        bool _is_detonate_bomb_at_touch_of_button_activated = false;

        int _bomb_blast_radius = 1;
        int _max_bomb_num = 10;

        std::vector<Object::Bomb> _bombs;
        std::vector<Object::Enemy> _enemies;

        Object::Point _bo_man_coords = kStartPoint;

        std::vector<Object::Point> _direction_of_movement_of_enemy; // ANTODO _direction_delta

        bool AreAllCellsAvailable(int number_of_indestructible_walls);

        void GenerateIndestructibleWalls(int number_of_indestructible_walls);

        std::vector<Object::Point> GenerateWalls(int walls_count);

        void GenerateMagicDoor(std::vector<Object::Point>& walls);
         
        void GenerateBonuses(std::vector<Object::Point>& walls);

        //void GenerateEnemies(int enemies_num); 

        //void GenerateDirectionOfEnemyMovement();

        bool IsIsolated(Object::Point enemy) const;

        Object::Point RefreshNewPosAndDirection(Object::Point point, int num_in_enemy_coords_vector);

        void MoveEnemies();

        void ReduceCurLifeByOneAndMoveToStart();

        void ExplosionTimeController();


    public:
        Game(int rows_count, int cols_count);
      
        //void BlowAllBombsNow();
        
        void TakeBonusOrMagicDoor(Object::Point point);

        void MoveBoMan(Direction direction);

        void DropBomb();

        void BombBlowUp(Object::Bomb& bomb);

        void Print();

        void Run();

        bool IsGameOver();

        void Stop();
    };
}