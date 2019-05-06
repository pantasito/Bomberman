#pragma once

#include "Field.h"

class Enemy {
    Point point;

    Enemy enemy(const Enemy& enemy) = delete;
    void operator = (const Enemy& enemy) = delete;
};
