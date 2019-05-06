#pragma once

#include "Field.h"

struct Bomb {
    Point _point;
    time_t _time_of_explosion;
    Bomb(Point point, time_t time_of_explosion) : _point(point), _time_of_explosion(time_of_explosion) {}

    Bomb(const Bomb& bomb) = delete;
    void operator=(const Bomb& bomb) = delete;

    bool operator == (const Bomb bomb) const {
        return (bomb._point == _point && bomb._time_of_explosion == _time_of_explosion);
    }
};
