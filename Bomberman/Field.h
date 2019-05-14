// ☕ Привет
#pragma once 

#include <string> 

#include <vector> 

#include <iostream> 

#include <assert.h> 

#include "Object/Point.h"

namespace Bomberman
{
    enum class FieldObject : int {
        Empty = 0,
        BoMan = 1,
        Wall = 2,
        IndestructibleWall = 4, // ANTODO backup
        Enemy = 8,
        MagicDoor = 16,
        Bomb = 32,
        IncreaseBombBlastRadius = 64,					//r
        IncreasingNumberOfBombs = 128,              	//n
        AbilityToPassThroughWalls = 256,				//a
        ImmunityToExplosion = 512,						//g
        RunningSpeed = 1024,							//s
        DetonateBombAtTouchOfButton = 2048,				//q
    };


    using Object::Point;

    class Field {
        const int _rows_count;
        const int _cols_count;

        int** _field;
        std::string _str_field;

        void SetSymbol(Point point, char symbol) {
            _str_field[(point._row_num + 1) * (_cols_count + 3) + point._col_num + 1] = symbol;
        }

        static constexpr int UnionTwoFieldObjects(FieldObject obj1, FieldObject obj2)
        {
            return static_cast<int>(obj1) | static_cast<int>(obj2);
        }

        static inline const std::vector<std::pair<int, char>> FieldObjectAndObjectSymbol = 
        {
            {static_cast<int>(FieldObject::Enemy), (char)245 },              // 'ї'
            {UnionTwoFieldObjects(FieldObject::BoMan, FieldObject::Wall), (char)176 },       // '░' 
            {static_cast<int>(FieldObject::BoMan), 'o' },
            {static_cast<int>(FieldObject::IndestructibleWall), (char)219 }, // '█'
            {static_cast<int>(FieldObject::Bomb), (char)253 },               // '¤' 
            {static_cast<int>(FieldObject::Wall), (char)177 },               // '▒'
            {static_cast<int>(FieldObject::MagicDoor), (char)127 },          // '⌂'
            {static_cast<int>(FieldObject::IncreaseBombBlastRadius), 'r' },
            {static_cast<int>(FieldObject::IncreasingNumberOfBombs), 'n' },
            {static_cast<int>(FieldObject::AbilityToPassThroughWalls), 'i' },
            {static_cast<int>(FieldObject::ImmunityToExplosion), 'g' },
            {static_cast<int>(FieldObject::DetonateBombAtTouchOfButton), 'q' }
        };
        
        void UpdateStringPoint(Point point) {
            if (IsEmpty(point))
            {
                SetSymbol(point, ' ');
                return;
            }

            for (auto& elem : FieldObjectAndObjectSymbol)
            {
                if (FullyContained(elem.first, point))
                {
                    SetSymbol(point, elem.second);
                    return;
                }
            }
            
            assert(IsEmpty(point));
        }

        void InitializeStrField() {
            static const char kLeftUp     = (char)201; // '╔'
            static const char kRightUp    = (char)187; // '╗'
            static const char kHorizontal = (char)205; // '═'
            static const char kVertical   = (char)186; // '║'
            static const char kLeftDown   = (char)200; // '╚' 
            static const char kRightDown  = (char)188; // '╝' 

            const std::string horizontal_part_of_field_frame(_cols_count, kHorizontal);

            const std::string top_of_field_frame = kLeftUp + horizontal_part_of_field_frame + kRightUp + '\n';
            const std::string lower_part_of_field_frame = kLeftDown + horizontal_part_of_field_frame + kRightDown + '\n';

            std::string str_field_with_side_parts_of_frame(_rows_count * (_cols_count + 3), ' ');
            for (int i = 0; i < _rows_count; ++i) {
                str_field_with_side_parts_of_frame[i * (_cols_count + 3)] = kVertical;
                str_field_with_side_parts_of_frame[i * (_cols_count + 3) + _cols_count + 1] = kVertical;
                str_field_with_side_parts_of_frame[i * (_cols_count + 3) + _cols_count + 2] = '\n';
            }

            _str_field = top_of_field_frame + str_field_with_side_parts_of_frame + lower_part_of_field_frame;
        }

    public:
        Field(int rows_count, int cols_count)
            : _rows_count(rows_count)
            , _cols_count(cols_count) {

            assert(rows_count >= 3 && cols_count >= 3);

            _field = new int*[_rows_count];
            for (int i = 0; i < _rows_count; ++i) {
                _field[i] = new int[_cols_count];

                for (int j = 0; j < _cols_count; ++j) {
                    _field[i][j] = static_cast<int>(FieldObject::Empty);
                }
            }

            InitializeStrField();
        }

        void Clear() {
            for (int i = 0; i < _rows_count; ++i) {
                for (int j = 0; j < _cols_count; ++j) {
                    Set(FieldObject::Empty, Point(i, j));
                }
            }
        }

        void Remove(FieldObject object, Point point) {
            _field[point._row_num][point._col_num] &= ~static_cast<int>(object);
            UpdateStringPoint(point);
        }

        bool IsIn(FieldObject object, Point point) const {
            return _field[point._row_num][point._col_num] & static_cast<int>(object);
        }

        bool FullyContained(int objects_mask, Point point) const {
            return (_field[point._row_num][point._col_num] & objects_mask) == objects_mask;
        }

        bool AtLeastOneContained(int objects_mask, Point point) const {
            return _field[point._row_num][point._col_num] & objects_mask;
        }

        bool IsEmpty(Point point) const {
            return _field[point._row_num][point._col_num] == static_cast<int>(FieldObject::Empty);
        }

        void Add(FieldObject object, Point point) {
            _field[point._row_num][point._col_num] |= static_cast<int>(object);
            UpdateStringPoint(point);
        }

        void Set(FieldObject object, Point point) {
            _field[point._row_num][point._col_num] = static_cast<int>(object);
            UpdateStringPoint(point);
        }

        bool IsOnField(Point point) const {
            return (point._col_num >= 0 && point._col_num < ColsCount() && point._row_num >= 0 && point._row_num < RowsCount());
        }

        void Print()
        {
            std::cout << _str_field;
        }

        int ColsCount() const { return _cols_count; }
        int RowsCount() const { return _rows_count; }
    };
}