// ☕ Привет
#pragma once

#include "Object/Point.h"

namespace Bomberman
{
    namespace Object
    {
        struct Bomb {
            Point _point;
            time_t _time_of_explosion;

            Bomb(Point point, time_t time_of_explosion) : _point(point), _time_of_explosion(time_of_explosion) {}

            Bomb(const Bomb& bomb) = delete;
            void operator=(const Bomb& bomb) = delete;
            
            Bomb(Bomb&& bomb) = default;
            Bomb& operator=(Bomb&& bomb) = default;

            
            bool operator==(const Bomb& bomb) const {
                return (bomb._point == _point && bomb._time_of_explosion == _time_of_explosion);
            }
        };
    }
}