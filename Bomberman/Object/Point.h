// ☕ Привет
#pragma once

namespace Bomberman
{
    namespace Object
    {
        struct Point {
            int _row_num;
            int _col_num;

            Point(int row_num, int col_num) : _row_num(row_num), _col_num(col_num) {}

            bool operator==(const Point point) const {
                return (_row_num == point._row_num && _col_num == point._col_num);
            }

            bool operator!=(const Point point) const {
                return (_row_num != point._row_num || _col_num != point._col_num);
            }

            Point operator*(int num) const {
                return Point(_row_num * num, _col_num * num);
            }

            Point operator+(Point point) const {
                return Point(point._row_num + _row_num, point._col_num + _col_num);
            }
        };
    }
}