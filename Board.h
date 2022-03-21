#pragma once
#ifndef BOARD_H
#define BOARD_H

#include <vector>
#include <array>

const unsigned short WIDTH = 9;
const unsigned short HEIGHT = 9;

enum class CheckFlow
{
	VERTICAL,
	HORIZONTAL,
	BLOCK,
};

class Board
{
private:
	std::array<unsigned short, WIDTH*HEIGHT> m_cells;
public:
	Board();

	const unsigned short operator[](std::size_t i) const;
	void addCell(const unsigned short row, const unsigned short col, const unsigned short value);
	std::vector<unsigned short> getAvailability(const unsigned short row, const unsigned short col) const;
	void removeCell(const unsigned short row, const unsigned short col);
	void clearRow(const unsigned short row);

};

#endif
