// ☕ Привет
#pragma once

#include "Object/Point.h"

namespace Bomberman
{
    namespace Object
    {
        struct Enemy {
            Point _current_coords;
            Point _direction_of_movement;
            
            Enemy(Point current_coords, Point direction_of_movement) :
                _current_coords(current_coords), _direction_of_movement(direction_of_movement) 
            {}
            
            Enemy(Enemy&& bomb) = default;

            Enemy enemy(const Enemy& enemy) = delete;
            void operator = (const Enemy& enemy) = delete;
        };
    }
}