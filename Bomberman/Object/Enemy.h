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
            
            Enemy(Enemy&& enemy) = default;
            Enemy& operator=(Enemy&& enemy) = default;

            Enemy enemy(const Enemy& enemy) = delete;
            void operator=(const Enemy& enemy) = delete;
            /*
            bool operator<(const Enemy& enemy) {
                if (_current_coords._row_num == enemy._current_coords._row_num) return _current_coords._col_num < enemy._current_coords._col_num;
                return _current_coords._row_num < enemy._current_coords._row_num;
            }
            */
        };
    }
}