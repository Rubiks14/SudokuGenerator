#include <iostream>
#include <stdexcept>
#include <vector>
#include "Board.h"

Board::Board() : m_cells{} noexcept
{
	m_cells.fill(0);
}

const unsigned short Board::operator[](std::size_t i) const
{
	if (i < 0 || i >= WIDTH * HEIGHT)
		throw std::range_error("the range of the board is [0-" + WIDTH*HEIGHT + ')');
	return m_cells[i];
}

const unsigned short Board::getCell(const unsigned short row, const unsigned short col) const
{
	if (row < 0 || row >= HEIGHT || col < 0 || col >= WIDTH)
		throw std::range_error("row and col must be between [0-" + WIDTH + ')');
	return m_cells[row * WIDTH + col];
}

std::vector<unsigned short> Board::getAvailability(const unsigned short row, const unsigned short col) const
{
	if (row < 0 || row >= HEIGHT || col < 0 || col >= WIDTH)
		throw std::range_error("row and col must be between [0-" + WIDTH + ')');
	
	std::vector<unsigned short> available{ 1, 2, 3, 4, 5, 6, 7, 8, 9 };

	const unsigned short BlockStartRow = row / 3 * 3;
	const unsigned short BlockStartCol = col / 3 * 3;

	for (int i = 0; i < m_cells.size(); ++i)
	{
		unsigned short cellRow = i / WIDTH;
		unsigned short cellCol = i % WIDTH;
		unsigned short cellBlockRow = cellRow / 3 * 3;
		unsigned short cellBlockCol = cellCol / 3 * 3;

		if (m_cells[i] != 0 && (cellRow == row || cellCol == col || (BlockStartRow == cellBlockRow && BlockStartCol == cellBlockCol)))
		{
			for (size_t i = 0; i < available.size(); ++i)
			{
				if (available[i] == m_cells[cellRow * WIDTH + cellCol])
				{
					available.erase(available.begin() + i);
					break;
				}
			}
		}
	}
	return available;
}

void Board::fillCell(const unsigned short row, const unsigned short col, const unsigned short value)
{
	if (row < 0 || row >= HEIGHT || col < 0 || col >= WIDTH)
		throw std::range_error("row and col must be between [0-" + WIDTH + ')');
	if (value < 1 || value > 9)
		throw std::out_of_range("Values can only be between 1 and 9");
	m_cells[row * WIDTH + col] = value;
}

void Board::emptyCell(const unsigned short row, const unsigned short col)
{
	if (row < 0 || row >= HEIGHT || col < 0 || col >= WIDTH)
		throw std::range_error("row and col must be between [0-" + WIDTH + ')');
	if (m_cells[row * WIDTH + col] != 0)
		m_cells[row * WIDTH + col] = 0;
}

void Board::clearRow(const unsigned short row)
{
	if (row < 0 || row >= HEIGHT)
		throw std::range_error("row must be between [0-" + WIDTH + ')');
	for (unsigned short i = 0; i < WIDTH; ++i)
	{
		emptyCell(row, i);
	}
}
