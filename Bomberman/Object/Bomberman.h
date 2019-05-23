// ☕ Привет

#pragma once

#include "Point.h"

namespace Bomberman
{
    namespace Object {

        using Object::Point;

        struct BoManBonuses {
            bool _is_walk_through_walls = false;
            bool _is_blast_immunity = false;
            bool _is_detonate_bomb_by_button = false;

            int _bomb_blast_radius = 1;
            int _max_bomb_count = 1;
        };

        struct Bomberman {
            BoManBonuses _bonuses;
            int _lives = 3;
            Point _bo_man_coords = Point(0, 0);
        };
    }
}