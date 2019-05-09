// ☕ Привет

#include "Game.h"

namespace Bomberman
{
    bool Game::AreAllCellsAvailable(int number_of_indestructible_walls) {
        assert(_field.IsEmpty(kStartPoint));

        std::queue<Object::Point> cells_to_check;

        std::vector<std::vector<bool>> marked_cells(_field.RowsCount(), std::vector<bool>(_field.ColsCount(), false));

        cells_to_check.push(kStartPoint);
        int num_of_tested_cells = 1;
        marked_cells[kStartPoint._row_num][kStartPoint._col_num] = true;

        while (!cells_to_check.empty()) {
            const auto cell_to_check = cells_to_check.front();
            cells_to_check.pop();

            for (int i = 0; i < kMoveDeltas.size(); ++i) {
                const auto cell = cell_to_check + kMoveDeltas[i];

                if (!_field.IsOnField(cell)) {
                    continue;
                }

                if (marked_cells[cell._row_num][cell._col_num]) {
                    continue;
                }

                if (_field.IsIn(FieldObject::IndestructibleWall, cell)) {
                    continue;
                }

                marked_cells[cell._row_num][cell._col_num] = true;
                cells_to_check.push(cell);
                ++num_of_tested_cells;
            }
        }

        if (num_of_tested_cells + number_of_indestructible_walls != _field.ColsCount() * _field.RowsCount()) {
            return false;
        }

        return true;
    }

}