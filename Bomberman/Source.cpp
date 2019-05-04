#include <iostream> 

#include <conio.h> 

#include <ctime> 

#include <string>
#include <vector> 
#include <queue>


#include <chrono> 

#include <thread> 
#include <mutex> 

#include <memory> 

#include "Field.h" 

/*
��� ��������� ��������� ��� �����:
	1) ��� ���� ����� ������� ������ �� ����� (�����, ��� �����)
	n �������� m+1 ��� (n - ������ ����, m - ���������� ������)

	2) ��������� ��� ���� � ��������� ����� �� ����� � ����� � ����������� �������?
	(n �������� � n ������, ����� ��� ������ � ������� ����� �������� ���� �� ���-�� � ������ ����� ���, ���� ��� - �����, ������ ����� ����� �
	�������� ��������)

	3) ��������� ��������, ����� � ���� ���� ������� ����� � ������� ���������� �������.
	�������� ��� �����, ����� ��������. ��� �����!
*/

static const int time_from_planting_bomb_until_its_explosion = 3;

enum class Direction : int
{
	Left,
	Up,
	Right,
	Down
};


static const std::vector<Point> kMoveDeltas = { Point(0,-1), Point(-1,0), Point(0,1), Point(1,0) };

bool FieldCellAvailabilityTest(Field& field, Point start) {
	std::queue<Point> points;
	points.push(start);
	//� ����� �� �� ��� ���� ������� �� ����� ����� ���������:
	//���� ����� �� �������, ������� � ������� ��� �������� ����� ���������������� ���������
	//������ ������������ �����
	while (!points.empty()) {
		Point p = points.front();
		points.pop();
		field.Add(Tested, start);

		for (int i = 0; i < kMoveDeltas.size(); ++i) {
			const Point new_point = p + kMoveDeltas[i];

			if (!field.IsOnField(new_point)) {
				continue;
			}

			if (field.IsIn(Tested, new_point)) {
				continue;
			}

			if (field.IsIn(IndestructibleWall, new_point)) {
				continue;
			}

			field.Add(Tested, new_point);
			points.push(new_point);
		}
	}

	// ANTODO  �������� ��� ����� �����

	//���� �� ����, ���� �������� �����, ������� ����� � ���� 
	//� �� �������� ����� ������ � �� �������� ������������� ������ - ����������� false;
	for (int i = 0; i < field.RowsCount(); ++i) {
		for (int j = 0; j < field.ColsCount(); ++j) {
			if (!field.IsOnField(Point(i, j))) {
				continue;
			}
			if (field.IsIn(IndestructibleWall, Point(i, j))) {
				continue;
			}
			if (field.IsIn(Tested, Point(i, j))) {
				continue;
			}

			return false;
		}
	}
	//field.Print();
	//system("pause");
	//system("cls");

	for (int i = 0; i < field.RowsCount(); ++i) {
		for (int j = 0; j < field.ColsCount(); ++j) {
			if (!field.IsOnField(Point(i, j))) {
				continue;
			}
			if (field.IsIn(IndestructibleWall, Point(i, j))) {
				continue;
			}
			if (field.IsIn(Tested, Point(i, j))) {
				field.Set(Empty, Point(i, j));
			}
		}
	}
	//field.Print();
	//system("pause");
	//system("cls");
	return true;
}

class Game {
	Field _field;

	// ANTODO - ������� �� ����������
	// ANTODO - ��� ���� ������� � 
	bool _game_over = false;
	bool _you_won = false;
	
	// ANTODO �������� ��������� ���������� ���-�� ������. � �������� ��� cur
	int _lives = 3;
	
	bool _ability_to_pass_through_walls = false;
	bool _immunity_to_explosions = false;
	bool _detonate_bomb_at_touch_of_button = false;

	int _bomb_blast_radius = 1;
	int _max_bomb_num = 1;
	int _cur_bomb_count = 0; // ANTODO �������

	const Point _start = Point(1, 1); // ANTODO ������� static � ������� � ����� ����������
	Point _bo_man_coords = _start;

	// ANTODO ������� ����� Enemy
	std::vector<Point> _enemies_coords;
	std::vector<Point> _direction_of_movement_of_enemy;
	
	std::vector<Point> _only_walls; // ANTODO ������ �� �����������
	std::vector<PlantedBomb> _planted_bombs;

	void GenerateFrame() {
		for (int i = 1; i < _field.ColsCount() - 1; ++i) {
			_field.Set(Frame1, Point(0, i));
			_field.Set(Frame1, Point(_field.RowsCount() - 1, i));
		}

		for (int i = 1; i < _field.RowsCount() - 1; ++i) {
			_field.Set(Frame2, Point(i, 0));
			_field.Set(Frame2, Point(i, _field.ColsCount() - 1));
		}
		_field.Set(Frame3, Point(0, 0));
		_field.Set(Frame4, Point(0, _field.ColsCount() - 1));
		_field.Set(Frame5, Point(_field.RowsCount() - 1, 0));
		_field.Set(Frame6, Point(_field.RowsCount() - 1, _field.ColsCount() - 1));
	}

	void GenerateIndestructibleWalls(int IndestructibleWallsCount) { // ANTODO code style
		int indestructible_walls_generated = 0;

		while (indestructible_walls_generated < IndestructibleWallsCount) {
			const Point point(rand() % _field.RowsCount(), rand() % _field.ColsCount());

			if (_field.IsOnField(point)) {
				_field.Set(IndestructibleWall, point);
				++indestructible_walls_generated;
			}
		}
		_field.Set(Empty, Point(1, 1)); // ANTODO
	}

	void GenerateWalls(int WallsCount) {
		int walls_generated = 0;
		while (walls_generated < WallsCount) {
			const Point point(rand() % _field.RowsCount(), rand() % _field.ColsCount());
			if (_field.IsEmpty(point)) {
				_field.Set(Wall, point);
				_only_walls.push_back(point);
				++walls_generated;
			}
		}
	}

	void GenerateMagicDoor() {
		if (!_only_walls.empty()) {
			_field.Add(MagicDoor, _only_walls.back());
			_only_walls.pop_back();
			return;
		}

		// ����� ��������� �����, ����� �� ����� �������� ����� ��������� ���� ������, ����� 
		// ������������� ����� � ����������. �.�. ������ �� ����� ��� ����� ��� ������ ������
		while (true) {
			const Point point(rand() % _field.RowsCount(), rand() % _field.ColsCount()); {
				if (!_field.IsIn(IndestructibleWall, point) && !_field.IsIn(BoMan, point)) {
					_field.Set(MagicDoor, point);
					return;
				}
			}
		}
	}

	void GenerateEnemies(int enemies_num) {
		int generated_enemies_counter = 0;
		while (generated_enemies_counter < enemies_num) {
			const Point point(rand() % _field.RowsCount(), rand() % _field.ColsCount());

			if (_field.IsEmpty(point)) {
				_field.Add(Enemy, point);
				_enemies_coords.push_back(point);
				++generated_enemies_counter;
			}
		}
	}

	void GenerateDirectionOfEnemyMovement() { // ANTODO
		for (int i = 0; i < _enemies_coords.size(); ++i) {
			for (int j = 0; j < 4; ++j) {
				const Point possible_direction = _enemies_coords[i] + kMoveDeltas[j];
				if (IsIsolated(possible_direction)) {
					_direction_of_movement_of_enemy.emplace_back(0, 0);
					break;
				}
				if (!_field.IsOnField(possible_direction)) {
					continue;
				}

				_direction_of_movement_of_enemy.push_back(kMoveDeltas[j]);
				break;
			}
		}
	}

	void GenerateBonuses() {
		const int num_of_walls_with_bonuses_of_one_type = (int)(_only_walls.size() / 30); // ANTODO 30 �� ��������																  
									  // ����� ����������� ��� �������� �� ����� �������.

		for (int i = 0; i < num_of_walls_with_bonuses_of_one_type; ++i) {
			_field.Add(IncreasingNumberOfBombsDeliveredAtTime, _only_walls.back());
			_only_walls.pop_back();
			_field.Add(IncreaseBombBlastRadius, _only_walls.back());
			_only_walls.pop_back();
		}
		
		//�� ������ ������� ������
		if (_only_walls.size() > 3) {										
			_field.Add(AbilityToPassThroughWalls, _only_walls.back());
			_only_walls.pop_back();
			_field.Add(ImmunityToExplosion, _only_walls.back());
			_only_walls.pop_back();
			_field.Add(DetonateBombAtTouchOfButton, _only_walls.back());
			_only_walls.pop_back();
		}
		//�� ��������� �������� ���������� �� ���������
	}

	bool IsIsolated(Point enemy) {
		for (int i = 0; i < 4; ++i) {
			const Point point = enemy + kMoveDeltas[i];
			if (!_field.IsOnField(point)) {
				continue;
			}

			// ANTODO ������� � ������� 
			if (!(_field.IsIn(Wall, point) || _field.IsIn(IndestructibleWall, point) || _field.IsIn(Enemy, point) || _field.IsIn(Bomb, point))) {
				return false;
			}
		}
		return true;
	}

	Point RefreshNewPosAndDirection(Point point, int num_in_enemy_coords_vector) {
		bool got_right_dir = false;
		while (!got_right_dir) {
			int dir = rand() % 4;
			const Point new_pos = point + kMoveDeltas[dir];
			if (!_field.IsOnField(new_pos)) {
				continue;
			}
			if (IsIsolated(new_pos)) {
				return point;
			}
			if (_field.IsIn(Wall, new_pos) || _field.IsIn(IndestructibleWall, new_pos) || _field.IsIn(Bomb, new_pos) || _field.IsIn(Enemy, new_pos)) {
				continue;
			}
			got_right_dir = true;
			_direction_of_movement_of_enemy[num_in_enemy_coords_vector] = kMoveDeltas[dir];
			return point + kMoveDeltas[dir];
		}
	}

	void MoveEnemies() {
		// ���� ���� ���� �������� �� ������ �� �����,
		// � ������ 20% ����� �����������
		// �������� �� �����
		for (int i = 0; i < _enemies_coords.size(); ++i) {
			if (IsIsolated(_enemies_coords[i])) {
				continue;
			}
			Point new_pos = _enemies_coords[i] + _direction_of_movement_of_enemy[i];
			if (!_field.IsOnField(new_pos)) {
				new_pos = RefreshNewPosAndDirection(_enemies_coords[i], i);
			}

			if (_field.IsIn(Wall, new_pos) || _field.IsIn(IndestructibleWall, new_pos) || _field.IsIn(Enemy, new_pos) || _field.IsIn(Bomb, new_pos)) {
				new_pos = RefreshNewPosAndDirection(_enemies_coords[i], i);
			}

			if (_field.IsIn(BoMan, new_pos)) {
				MinusOneLife();
			}

			// ��������� ��� ���� ����� 1. ���� ���, � ������ 2 �������, � �� ��� ������� ��� ����� ����

			_field.Remove(Enemy, _enemies_coords[i]); // ��������� ��� ������ ����� �������� ��� ��������, �� �� ��������� � ���������
			_field.Add(Enemy, new_pos);
			_enemies_coords[i] = new_pos;
		}
		/*
		//��������� ����, ���� ���� �� ����� �� �����

		for (auto& enemy_coords : _enemies_coords) {
			while (true) {
				if (IsIsolated(enemy_coords)) {
					break;
				}
				int dir = rand() % 4;
				const Point new_pos = kMoveDeltas[dir] + enemy_coords;

				if (!_field.IsOnField(new_pos)) {
					continue;
				}

				if (_field.IsIn(Wall, new_pos) || _field.IsIn(IndestructibleWall, new_pos)
					|| _field.IsIn(Bomb, new_pos) || _field.IsIn(Enemy, new_pos)) {
					continue;
				}

				if (_field.IsIn(BoMan, new_pos)) {
					MinusOneLife();
				}

				// ��������� ��� ���� ����� 1. ���� ���, � ������ 2 �������, � �� ��� ������� ��� ����� ����

				_field.Remove(Enemy, enemy_coords); // ��������� ��� ������ ����� �������� ��� ��������, �� �� ��������� � ���������
				_field.Add(Enemy, new_pos);
				enemy_coords = new_pos;
				break;
			}
		}
		*/
	}

	void MinusOneLife() {
		_field.Remove(BoMan, _bo_man_coords);
		--_lives;

		if (_lives == 0) {
			_game_over = true;
			return;
		}

		_bo_man_coords = _start;
		_field.Set(BoMan, _bo_man_coords);
	}

	void ExplosionTimeController() {
		if (_planted_bombs.empty()) {
			return;
		}
		for (auto& bomb : _planted_bombs) {
			if (time(0) - bomb._time >= time_from_planting_bomb_until_its_explosion) {
				BombBlowUp(bomb);
			}
		}
	}

public:
	Game(int rows_count, int cols_count)
		: _field(rows_count, cols_count) {
		int number_of_objects = (int)(_field.RowsCount() * _field.ColsCount() * 0.16);

		while (true) {
			GenerateIndestructibleWalls(number_of_objects);
			if (FieldCellAvailabilityTest(_field, _start)) {
				break;
			}
			_field.Clear();
		}

		GenerateFrame();
		GenerateWalls(number_of_objects);
		_field.Set(BoMan, _bo_man_coords);
		GenerateMagicDoor();
		GenerateEnemies(_field.RowsCount());
		GenerateDirectionOfEnemyMovement();
		GenerateBonuses();
	}

	void BlowAllBombsNow() {
		if (_detonate_bomb_at_touch_of_button == false) {
			return;
		}

		for (auto bomb : _planted_bombs) {
			BombBlowUp(bomb);
		}
	}

	void GetInfoFromThisPoint(Point point) {
		if (_field.IsIn(Empty, point)) {
			return;
		}

		if (_field.IsIn(Wall, point)) {
			return;
		}

		if (_field.IsIn(MagicDoor, point)) {
			if (!_enemies_coords.empty()) {
				return;
			}
			_game_over = true;
			_you_won = true;
			return;
		}

		if (_field.IsIn(IncreaseBombBlastRadius, point)) {
			++_bomb_blast_radius;
			_field.Remove(IncreaseBombBlastRadius, point);
			return;
		}

		if (_field.IsIn(IncreasingNumberOfBombsDeliveredAtTime, point)) {
			++_max_bomb_num;
			_field.Remove(IncreasingNumberOfBombsDeliveredAtTime, point);
			return;
		}

		if (_field.IsIn(AbilityToPassThroughWalls, point)) {
			_ability_to_pass_through_walls = true;
			_field.Remove(AbilityToPassThroughWalls, point);
			return;
		}

		if (_field.IsIn(ImmunityToExplosion, point)) {
			_immunity_to_explosions = true;
			_field.Remove(ImmunityToExplosion, point);
			return;
		}

		if (_field.IsIn(DetonateBombAtTouchOfButton, point)) {
			_detonate_bomb_at_touch_of_button = true;
			_field.Remove(DetonateBombAtTouchOfButton, point);
			return;
		}
	}

	void MoveBoMan(Direction direction) {
		const auto new_pos = _bo_man_coords + kMoveDeltas[static_cast<int>(direction)];

		if (!_field.IsOnField(new_pos)) {
			return;
		}

		if (_field.IsIn(IndestructibleWall, new_pos)) {
			return;
		}

		if ((_field.IsIn(Wall, new_pos) || _field.IsIn(Bomb, new_pos)) && _ability_to_pass_through_walls == false) {
			return;
		}

		// ..������� � ���� ����� � ��� ���� ��������. ������, ��� ����� ������� ��� ��� � ��� ��� ������� ����������� ��������������
		// ���. ��, ��� ����� �� ������ ������ ����������������. ����� �� ������ ����� �������
		// �.�. ���� �������� MoveBoMan, �� ����� �������� ������� �� Run.
		// ����� ���� ����� �������� ������������� �������, ���� �������� ��������
		// ����������� � ���� ������ ����� ��������� ���� ��. ������ ����� ����� ����� � ����, �� �� ����� ����� ��� ��� �� ��������
		 // ��� ������	�������� ���� ����. � ���� ������� ����� ���������� ���, ��� 
						// ������ �� ���� ������ �� ����� � ��� ��� �� �� ����� ���������


		if (_field.IsIn(Enemy, new_pos)) {
			MinusOneLife();
			return;
		}

		// �.�. ���� ��� 3 ������� ����������� ��������
		//���-�� ���� lock()
		_field.Add(BoMan, new_pos);
		_field.Remove(BoMan, _bo_man_coords);
		_bo_man_coords = new_pos;
		//unlock(); ?..
	}

	void DropBomb() {
		if (_cur_bomb_count >= _max_bomb_num) {
			return;
		}

		_planted_bombs.emplace_back(_bo_man_coords, time(0));
		_field.Add(Bomb, _bo_man_coords);
		++_cur_bomb_count;
	}

	void BombBlowUp(PlantedBomb bomb_explodes_at_the_moment) {
		bool _bo_man_exploded = false;
		if (_field.IsIn(BoMan, bomb_explodes_at_the_moment._point) && _immunity_to_explosions == false) {
			_bo_man_exploded = true;
		}

		if (_field.IsIn(MagicDoor, bomb_explodes_at_the_moment._point)) {
			_field.Add(Enemy, bomb_explodes_at_the_moment._point);
			_enemies_coords.push_back(bomb_explodes_at_the_moment._point);
			_direction_of_movement_of_enemy.push_back(kMoveDeltas[rand() % 4]);
		}


		for (int i = 0; i < 4; ++i) {
			for (int j = 1; j <= _bomb_blast_radius; ++j) {
				const auto exploded_cell = bomb_explodes_at_the_moment._point + kMoveDeltas[i] * j;

				if (!_field.IsOnField(exploded_cell) || _field.IsIn(IndestructibleWall, exploded_cell)) {
					break;
				}

				if (_field.IsIn(Wall, exploded_cell)) {
					if (_field.IsCellContainsOnlyWall(exploded_cell)) {
						for (int i = 0; i < _only_walls.size(); ++i) {
							if (_only_walls[i] == exploded_cell) {
								_only_walls[i] = _only_walls.back();
								_only_walls.pop_back();
								break;
							}
						}
					}

					_field.Remove(Wall, exploded_cell);
					if (_field.IsIn(BoMan, exploded_cell) && _immunity_to_explosions == false) {
						_bo_man_exploded = true;
					}
					break;
				}

				if (_field.IsIn(BoMan, exploded_cell) && _immunity_to_explosions == false) {
					_bo_man_exploded = true;
				}

				if (_field.IsIn(Enemy, exploded_cell)) {
					_field.Remove(Enemy, exploded_cell);

					for (int i = 0; i < _enemies_coords.size(); ++i) {
						if (_enemies_coords[i] == exploded_cell) {
							_direction_of_movement_of_enemy[i] = _direction_of_movement_of_enemy.back();
							_enemies_coords[i] = _enemies_coords.back();
							_enemies_coords.pop_back();
							_direction_of_movement_of_enemy.pop_back();
							break;
						}
					}
				}

				/*
					IncreaseBombBlastRadius = 64,					//r
					IncreasingNumberOfBombsDeliveredAtTime = 128,	//n
					AbilityToPassThroughWalls = 256,				//a
					ImmunityToExplosion = 512,						//g
					RunningSpeed = 1024,							//s
					DetonateBombAtTouchOfButton = 2048,				//q
				*/

				if (_field.IsIn(IncreaseBombBlastRadius, exploded_cell) || _field.IsIn(IncreasingNumberOfBombsDeliveredAtTime, exploded_cell) ||
					_field.IsIn(AbilityToPassThroughWalls, exploded_cell) || _field.IsIn(ImmunityToExplosion, exploded_cell) ||
					_field.IsIn(DetonateBombAtTouchOfButton, exploded_cell)) {
					_field.Set(Enemy, exploded_cell);
					_enemies_coords.push_back(exploded_cell);
					_direction_of_movement_of_enemy.push_back(kMoveDeltas[rand() % 4]);
				}

				if (_field.IsIn(MagicDoor, exploded_cell)) {
					_field.Add(Enemy, exploded_cell);
					_enemies_coords.push_back(exploded_cell);
				}

				if (_field.IsIn(Bomb, exploded_cell)) {
					for (int i = 0; i < _planted_bombs.size(); ++i) {
						if (_planted_bombs[i]._point == exploded_cell) {
							_planted_bombs[i]._time = time(0) - time_from_planting_bomb_until_its_explosion;
							break;
						}
					}
				}
			}
		}
		_field.Remove(Bomb, bomb_explodes_at_the_moment._point);

		for (auto& bomb : _planted_bombs) {
			if (bomb == bomb_explodes_at_the_moment) {
				bomb = _planted_bombs.back();
				_planted_bombs.pop_back();
			}
		}

		--_cur_bomb_count;
		if (_bo_man_exploded) {
			MinusOneLife();
		}
	}

	bool GameOver() {
		return _game_over;
	}

	void Print() {
		std::system("cls");
		_field.Print();
	}

	void Run() {
		auto enemy_move_time = time(0);

		while (!_game_over) {
			GetInfoFromThisPoint(_bo_man_coords);
			if (time(0) - enemy_move_time > 1) {
				MoveEnemies();
				enemy_move_time = time(0);
			}

			ExplosionTimeController();
			std::system("cls");

			std::cout << "lives: " << _lives;
			std::cout << " max bomb num: " << _max_bomb_num;
			std::cout << " bomb blast radius: " << _bomb_blast_radius << "  ";
			if (_planted_bombs.size() == 1) {
				std::cout << "timer: " << time(0) - _planted_bombs[0]._time << "/" << 3;
			}
			std::cout << std::endl;

			_field.Print();
		}
		std::system("cls");
		std::cout << (_you_won == true ? "YOU WON!" : "YOU LOST!") << std::endl;
		std::system("pause");
	}

	void Stop() {
		_game_over = true;
	}
};

/*
std::mutex cout_mutex;

void f(char symbol)
{
	for (int i = 0; i < 100; i++)
	{
		cout_mutex.lock();
		std::cout << symbol << " - " << i << std::endl;
		cout_mutex.unlock();
	}
}

void g()
{
	for (int i = 0; i < 100; i++)
	{
		std::cout << "g - " << i << std::endl;
	}
}
*/

#include <ios>

int main() {
	/*
	std::cout << "Helloooooooooooooooooooooooooooooooooooo";
	std::cout.seekp(5, std::ios_base::beg);
	std::cout << '1';
	std::cout.flush();
	*/

	/*
	std::thread _f_thread(f, 'f');
	f('g');
	_f_thread.join(); // ��������� ���������� _f_thread
	std::system("pause");
	return 0;
	*/
	/*
	for (int i = 0; i < 256; i++)
	{
	std::cout << i << "-" << (char)i << std::endl;
	}
	system("pause");
	*/

	srand((int)(time(0)));
	//Game game(17, 20);
	Game game(17, 35);

	std::thread _game_action_thread(&Game::Run, &game);

	char command;
	do
	{
		// game.Print(); 

		command = _getch();

		if (command == 27)
		{
			game.Stop();
			break;
		}

		switch (command)
		{
		case 'w': game.MoveBoMan(Direction::Up); break;
		case 'd': game.MoveBoMan(Direction::Right); break;
		case 's': game.MoveBoMan(Direction::Down); break;
		case 'a': game.MoveBoMan(Direction::Left); break;
		case ' ': game.DropBomb(); break;
		case 'q': game.BlowAllBombsNow(); break;
		}
	} while (!game.GameOver());

	_game_action_thread.join();
}