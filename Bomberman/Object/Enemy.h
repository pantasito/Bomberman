// ☕ Привет
#pragma once

#include "Object/Point.h"

namespace Bomberman
{
    namespace Object
    {
        struct Enemy {
            Point _current_coords;
          
            Point _direction_of_movement; // ANTODO delta 
            //UpdateDirectionAndMove
            
            int number_of_moves_made_in_one_direction = 0;
            
            Enemy(Point current_coords, Point direction_of_movement) :
                _current_coords(current_coords), _direction_of_movement(direction_of_movement) 
            {}
            
            void UpdateDirection(const Point dir) {
                _direction_of_movement = dir;
                number_of_moves_made_in_one_direction = 0;
            }

            void MoveInCurrentDirection() {
                _current_coords = _current_coords + _direction_of_movement;
                ++number_of_moves_made_in_one_direction;
            }

            Enemy(Enemy&& enemy) = default;
            Enemy& operator=(Enemy&& enemy) = default;

            Enemy enemy(const Enemy& enemy) = delete;
            void operator=(const Enemy& enemy) = delete;
        };
    }
}

