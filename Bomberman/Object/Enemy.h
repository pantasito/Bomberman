// ☕ Привет
#pragma once

#include "Object/Point.h"

namespace Bomberman
{
    namespace Object
    {
        struct Enemy {
            Point _current_coords; // AN replce to coords
          
            Point _coords_delta; 
            
            int number_of_moves_made_in_one_direction = 0;
            
            Enemy(Point current_coords, Point coords_delta) :
                _current_coords(current_coords), _coords_delta(coords_delta)
            {}
            
            void UpdateMovementDelta(const Point delta) {
                _coords_delta = delta;
                number_of_moves_made_in_one_direction = 0;
            }

            void MoveInCurrentDirection() {
                _current_coords +=_coords_delta;
                ++number_of_moves_made_in_one_direction;
            }

            Enemy(Enemy&& enemy) = default;
            Enemy& operator=(Enemy&& enemy) = default;

            Enemy enemy(const Enemy& enemy) = delete;
            void operator=(const Enemy& enemy) = delete;
        };
    }
}