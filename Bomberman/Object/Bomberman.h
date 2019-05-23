// ☕ Привет

#pragma once

#include "Point.h"

namespace Bomberman
{
    static const int kNumberOfLivesAtTheStart = 3;

    static const Point kStartPoint(0, 0);

    namespace Object {

        using Object::Point;

        struct Bomberman {
            struct Bonuses {
                bool _is_walk_through_walls = false;
                bool _is_blast_immunity = false;
                bool _is_detonate_bomb_by_button = false;

                int _bomb_blast_radius = 1;
                int _max_bomb_count = 1;
            } _bonuses;

            int _lives = kNumberOfLivesAtTheStart;
            Point _bo_man_coords = kStartPoint;
        };
    }
}