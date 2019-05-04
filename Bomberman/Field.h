#pragma once 

#include <string> 
#include <assert.h> 


struct Point {
	int _row_num;
	int _col_num;

	Point(int row_num, int col_num) : _row_num(row_num), _col_num(col_num) {}

	bool operator == (const Point point) const {
		return (_row_num == point._row_num && _col_num == point._col_num);
	}

	Point operator* (int num) const {
		return Point(_row_num * num, _col_num * num);
	}

	Point operator+(Point point) const {
		return Point(point._row_num + _row_num, point._col_num + _col_num);
	}

	Point operator-(Point point) const {
		return Point(_row_num - point._row_num, _col_num - point._col_num);
	}
};

struct PlantedBomb {
	Point _point;
	time_t _time;
	PlantedBomb(Point point, time_t time) : _point(point), _time(time) {}

	bool operator == (const PlantedBomb planted_bomb) const {
		return (planted_bomb._point == _point && planted_bomb._time == _time);
	}
};

enum FieldObject : int {
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
	Frame1 = 4096,
	Frame2 = 8192,
	Frame3 = 16384,
	Frame4 = 32768,
	Frame5 = 65536,
	Frame6 = 131072,
	Tested = 262144,
};

static const std::vector<std::pair<FieldObject, char>> v =
{
	std::pair<FieldObject, char>{Tested, (char)251},
	std::pair<FieldObject, char>{Frame1 , 205}
};

class Field
{
	const int _rows_count;
	const int _cols_count;

	int** _field;
	std::string _str_field;

	void SetSymbol(Point point, char symbol)
	{
		_str_field[point._row_num * (_cols_count + 1) + point._col_num] = symbol;

	}

	void UpdateStringPoint(Point point)
	{
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

		if (IsIn(Enemy, point)) {
			SetSymbol(point, 245);
			return;
		}

		if (IsIn(Wall, point) && IsIn(BoMan, point)) {
			SetSymbol(point, 176);
			return;
		}

		if (IsIn(BoMan, point)) {
			SetSymbol(point, 'o');
			return;
		}

		if (IsIn(Tested, point))
		{
			SetSymbol(point, 251);
			return;
		}
			
		if (IsIn(IndestructibleWall, point))
		{
			SetSymbol(point, 219);
			return;
		}		

		if (IsIn(Wall, point))
		{
			SetSymbol(point, 177);
			return;
		}

		if (IsIn(Bomb, point)) {
			SetSymbol(point, 253);
			return;
		}
		
		if (IsIn(MagicDoor, point)) {
			SetSymbol(point, 127);
			return;
		}

		if (IsIn(IncreaseBombBlastRadius, point)) {
			SetSymbol(point, 'r');
			return;
		}

		if (IsIn(IncreasingNumberOfBombsDeliveredAtTime, point)) {
			SetSymbol(point, 'n');
			return;
		}

		if (IsIn(AbilityToPassThroughWalls, point)) {
			SetSymbol(point, 'a');
			return;
		}

		if (IsIn(ImmunityToExplosion, point)) {
			SetSymbol(point, 'g');
			return;
		}

		if (IsIn(DetonateBombAtTouchOfButton, point)) {
			SetSymbol(point, 'q');
			return;
		}

		if (IsIn(Frame1, point))
		{
			SetSymbol(point, 205);
			return;
		}

		if (IsIn(Frame2, point))
		{
			SetSymbol(point, 186);
			return;
		}

		if (IsIn(Frame3, point))
		{
			SetSymbol(point, 201);
			return;
		}

		if (IsIn(Frame4, point))
		{
			SetSymbol(point, 187);
			return;
		}

		if (IsIn(Frame5, point))
		{
			SetSymbol(point, 200);
			return;
		}

		if (IsIn(Frame6, point))
		{
			SetSymbol(point, 188);
			return;
		}

		assert(IsEmpty(point));

		SetSymbol(point, ' ');
	}

public:
	Field(int rows_count, int cols_count)
		: _rows_count(rows_count)
		, _cols_count(cols_count)
	{
		_field = new int* [_rows_count];
		for (int i = 0; i < _rows_count; ++i) {
			_field[i] = new int[_cols_count];

			for (int j = 0; j < _cols_count; ++j) {
				_field[i][j] = FieldObject::Empty;
			}
		}

		_str_field.resize(rows_count * (cols_count + 1), ' ');
		for (int i = 0; i < rows_count; i++)
		{
			SetSymbol(Point(i, cols_count), '\n');
		}
	}

	void Erase() {
		for (auto& ch : _str_field) {
			ch = ' ';
		}
		for (int i = 0; i < RowsCount(); i++)
		{
			SetSymbol(Point(i, ColsCount()), '\n');
		}

		for (int i = 1; i < RowsCount() - 1; ++i) {
			for (int j = 1; j < ColsCount() - 1; ++j) {
				_field[i][j] = FieldObject::Empty;
			}
		}
	}


	void Remove(FieldObject object, Point point) {
		_field[point._row_num][point._col_num] &= (~object);
		UpdateStringPoint(point);
	}

	bool IsIn(FieldObject object, Point point) const {
		return _field[point._row_num][point._col_num] & object;
	}

	bool IsEmpty(Point point) const {
		return _field[point._row_num][point._col_num] == Empty;
	}

	bool IsCellContainsOnlyWall(Point point) const {
		return _field[point._row_num][point._col_num] == Wall;

		if (!IsIn(Wall, point)) {
			return false;
		}

		if (IsIn(MagicDoor, point)) {
			return false;
		}

		if (IsIn(IncreaseBombBlastRadius, point)) {
			return false;
		}

		return true;
	}

	void Add(FieldObject object, Point point) {
		_field[point._row_num][point._col_num] |= object;
		UpdateStringPoint(point);
	}

	void Set(FieldObject object, Point point) {
		_field[point._row_num][point._col_num] = object;
		UpdateStringPoint(point);
	}


	bool IsOnField(Point point) const {
		return (point._col_num >= 1 && point._col_num < ColsCount() - 1 && point._row_num >= 1 && point._row_num < RowsCount() - 1);
	}

	void Print()
	{
		std::cout << _str_field;
		std::cout.flush();
	}

	int ColsCount() const { return _cols_count; }
	int RowsCount() const { return _rows_count; }
};