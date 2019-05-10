// ☕ Привет
#pragma once 

#include <string> 

#include <iostream> 

#include <assert.h> 

#include "Object/Point.h"

namespace Bomberman
{
    enum class FieldObject : int {
        Empty = 0,
        BoMan = 1,
        Wall = 2,
        IndestructibleWall = 4,
        Enemy = 8,
        MagicDoor = 16,
        Bomb = 32,
        IncreaseBombBlastRadius = 64,					//r
        IncreasingNumberOfBombsDeliveredAtTime = 128,	//n
        AbilityToPassThroughWalls = 256,				//a
        ImmunityToExplosion = 512,						//g
        RunningSpeed = 1024,							//s
        DetonateBombAtTouchOfButton = 2048,				//q
    };

    /*
    static const std::vector<std::pair<FieldObject, char>> v =
    {
    //обойти сравнение на размещение в клетке двух объектов могу создав третий объект?
        std::pair<FieldObject, char>{Tested, (char)251},
        std::pair<FieldObject, char>{Frame1 , 205}
    };
    */

    using Object::Point;

    class Field {
        const int _rows_count;
        const int _cols_count;

        int** _field;
        std::string _str_field;

        void SetSymbol(Point point, char symbol) {
            _str_field[(point._row_num + 1) * (_cols_count + 3) + point._col_num + 1] = symbol;
        }

        void UpdateStringPoint(Point point) { //это в последнюю очередь, не знаю, как рефакторить
            if (IsEmpty(point))
            {
                SetSymbol(point, ' ');
                return;
            }
            /*
            for (auto& elem : v)
            {
                if (IsIn(elem.first, point))
                {
                    SetSymbol(point, elem.second);
                }
            }
            */

            if (IsIn(FieldObject::Enemy, point)) {
                SetSymbol(point, 245);
                return;
            }

            if (IsIn(FieldObject::Wall, point) && IsIn(FieldObject::BoMan, point)) {
                SetSymbol(point, 176);
                return;
            }

            if (IsIn(FieldObject::BoMan, point)) {
                SetSymbol(point, 'o');
                return;
            }

            if (IsIn(FieldObject::IndestructibleWall, point))
            {
                SetSymbol(point, 219);
                return;
            }

            if (IsIn(FieldObject::Wall, point))
            {
                SetSymbol(point, 177);
                return;
            }

            if (IsIn(FieldObject::Bomb, point)) {
                SetSymbol(point, 253);
                return;
            }

            if (IsIn(FieldObject::MagicDoor, point)) {
                SetSymbol(point, 127);
                return;
            }

            if (IsIn(FieldObject::IncreaseBombBlastRadius, point)) {
                SetSymbol(point, 'r');
                return;
            }

            if (IsIn(FieldObject::IncreasingNumberOfBombsDeliveredAtTime, point)) {
                SetSymbol(point, 'n');
                return;
            }

            if (IsIn(FieldObject::AbilityToPassThroughWalls, point)) {
                SetSymbol(point, 'a');
                return;
            }

            if (IsIn(FieldObject::ImmunityToExplosion, point)) {
                SetSymbol(point, 'g');
                return;
            }

            if (IsIn(FieldObject::DetonateBombAtTouchOfButton, point)) {
                SetSymbol(point, 'q');
                return;
            }

            assert(IsEmpty(point));

            SetSymbol(point, ' ');
        }

        void InitializeStrField()
        {
            static const char kLeftUp = (char)201; // '╔' // ANTODO
            
            const std::string horizontal_part_of_field_frame(_cols_count, (char)205 /*'═'*/);
            
            const std::string top_of_field_frame = kLeftUp + horizontal_part_of_field_frame + (char)187 /*'╗'*/ + '\n';
            const std::string lower_part_of_field_frame = (char)200 /*'╚'*/ + horizontal_part_of_field_frame + (char)188 /*'╝'*/ + '\n';

            std::string str_field_with_side_parts_of_frame(_rows_count * (_cols_count + 3), ' ');
            for (int i = 0; i < _rows_count; ++i) {
                str_field_with_side_parts_of_frame[i * (_cols_count + 3)] = (char)186 /*'║'*/;
                str_field_with_side_parts_of_frame[i * (_cols_count + 3) + _cols_count + 1] = (char)186 /*'║'*/;
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