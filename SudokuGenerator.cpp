#include <iostream>
#include <random>
#include <thread>
#include <chrono>
#include <array>
#include <algorithm>
#include <functional>

#include "Board.h"


// Places to refactor

// the code used to backtrack through the solution is duplicated in the solveBoard function. It is
// being used when there is no availability for the current cell and again when a solution is found
// and the algorithm is backtracking through the previous solution to look for alternate solutions

struct Point
{
    unsigned short row{ 0 }, col{ 0 };
    explicit Point(unsigned short r, unsigned short c) : row{ r }, col{ c } {}
    const bool operator==(const Point& other) const
    {
        return row == other.row && col == other.col;
    }
    const bool operator!=(const Point& other) const
    {
        return !(*this == other);
    }
    const bool operator<(const Point& other) const
    {
        if (row < other.row)
            return true;
        else if (other.row < row)
            return false;
        else if (col < other.col)
            return true;
        else
            return false;
    }
};


typedef std::vector<Point> Points;

std::random_device rd;
std::mt19937 rngeesus(rd());
std::uniform_int_distribution<unsigned short> allCells(0, WIDTH* HEIGHT - 1);
auto randomCell = std::bind(allCells, rngeesus);
const bool removeCells(Board& board, Points& removed, std::vector<unsigned short>& values, unsigned short& clues);
void fillBoard(Board& board);
void displayBoard(const Board& board);
const int solveBoard(Board board, Points points);

int main()
{
    Board board;
    std::vector<unsigned short> values;
    Points removed;
    fillBoard(board);
    unsigned short clues{ WIDTH * HEIGHT };
    
    int solutions{ 0 };
    int threshold{ 0 };

    while (clues >= 20 && threshold < 20)
    {
        if (!removeCells(board, removed, values, clues))
        {
            continue;
        }

        for (auto point : removed)
        {
            board.emptyCell(point.row, point.col);
        }
 
        solutions = solveBoard(board, removed);
        // Reinserts the last removed points from the list of removed points until
        // the board has a single solution
        while (solutions != 1 && !removed.empty())
        {
            auto removedPoint = removed.back();
            board.fillCell(removedPoint.row, removedPoint.col, values.back());
            removed.pop_back();
            values.pop_back();
            solutions = solveBoard(board, removed);
            ++clues;
            ++threshold;
        }
    }
    std::cout << "Puzzle has been generated";
    std::cin.get();
    displayBoard(board);
}

void fillBoard(Board& board) 
{
    for (unsigned short i = 0; i < WIDTH * HEIGHT;)
    {
        unsigned short row = i / WIDTH;
        unsigned short col = i % WIDTH;

        std::vector<unsigned short> availability = board.getAvailability(row, col);
        // if there is no availability for the current cell then the row is messed up and needs to be rerandomized.
        if (availability.size() == 0)
        {
            board.clearRow(row);
            i = row * WIDTH;

            static unsigned short messedUpCount = 0;
            ++messedUpCount;
            if (messedUpCount == 5)
            {
                for (int clearRow = 0; clearRow < HEIGHT; ++clearRow)
                {
                    board.clearRow(clearRow);
                }
                i = 0;
            }
        }
        else
        {
            std::shuffle(availability.begin(), availability.end(), rngeesus);
            board.fillCell(row, col, availability[0]);
            ++i;
        }
    }
}

const bool removeCells(Board& board, Points& removed, std::vector<unsigned short>& values, unsigned short& clues)
{
    unsigned short cell{ 0 };
    do
    {
        cell = randomCell();

    } while (board[cell] == 0);

    Point firstQuad(cell / WIDTH, cell % WIDTH);
    Point secondQuad(firstQuad.row, WIDTH - 1 - firstQuad.col);
    Point thirdQuad(HEIGHT - 1 - firstQuad.row, firstQuad.col);
    Point fourthQuad(HEIGHT - 1 - firstQuad.row, WIDTH - 1 - firstQuad.col);

    if (clues < WIDTH * HEIGHT - 20 && firstQuad != fourthQuad)
    {
        removed.push_back(firstQuad);
        values.push_back(board.getCell(firstQuad.row, firstQuad.col));
        removed.push_back(fourthQuad);
        values.push_back(board.getCell(fourthQuad.row, fourthQuad.col));
        clues -= 2;
        return true;
    }
    else if (firstQuad != secondQuad && firstQuad != thirdQuad && firstQuad != fourthQuad)
    {
        removed.push_back(firstQuad);
        values.push_back(board.getCell(firstQuad.row, firstQuad.col));
        removed.push_back(secondQuad);
        values.push_back(board.getCell(secondQuad.row, secondQuad.col));
        removed.push_back(thirdQuad);
        values.push_back(board.getCell(thirdQuad.row, thirdQuad.col));
        removed.push_back(fourthQuad);
        values.push_back(board.getCell(fourthQuad.row, fourthQuad.col));
        clues -= 4;
        return true;
    }
    return false;
}

void displayBoard(const Board& board)
{
    system("cls");
    std::cout << "-------------------------------------" << std::endl;
    for (size_t i = 0; i < WIDTH * HEIGHT; ++i)
    {
        if (i != 0 && i % WIDTH == 0)
        {
            std::cout << "|" << std::endl << "-------------------------------------" << std::endl;
        }
        if (board[i] == 0)
            std::cout << "|   ";
        else
            std::cout << "| " << board[i] << " ";
    }
    std::cout << "|" << std::endl << "-------------------------------------" << std::endl;
}

const int solveBoard(Board board, Points points) {

    // sort the points so they are aligned left to right top to bottom.
    // this makes finding a solution much faster than trying to solve the board
    // with the empty points stored in a random order.
    std::stable_sort(points.begin(), points.end());

    std::vector<std::vector<unsigned short>> solutions;
    std::vector<unsigned short> solution;
    std::vector<std::vector<unsigned short>> availability;

    auto pointIter = points.begin();
    availability.push_back(board.getAvailability(pointIter->row, pointIter->col));
    auto availableIter = availability.begin();
    auto currentAvailableIter = availableIter->begin();
    while (true)
    {
        if (currentAvailableIter != availableIter->end())
        {
            solution.push_back(*currentAvailableIter);
            board.fillCell(pointIter->row, pointIter->col, *currentAvailableIter);
            currentAvailableIter = availableIter->erase(availableIter->begin());
        }
        else if (solution.size() < points.size() && pointIter != points.begin())
        {
            --pointIter;
            board.emptyCell(pointIter->row, pointIter->col);
            solution.pop_back();
            availableIter = availability.erase(availability.end() - 1) - 1;
            currentAvailableIter = availableIter->begin();
            continue;
        }

        if (solution.empty() && pointIter == points.begin())
        {
            break;
        }
        else if (solution.size() == points.size())
        {
            solutions.push_back(solution);
            if (solutions.size() > 1)
            {
                break;
            }

            board.emptyCell(pointIter->row, pointIter->col);
            solution.pop_back();

            if (pointIter == points.begin())
                break;

            --pointIter;
            board.emptyCell(pointIter->row, pointIter->col);
            solution.pop_back();
            availableIter = availability.erase(availability.end() - 1) - 1;
            currentAvailableIter = availableIter->begin();
        }
        else
        {
            ++pointIter;
            availableIter = availability.insert(availability.end(), board.getAvailability(pointIter->row, pointIter->col));
            currentAvailableIter = availableIter->begin();
        }
    }
    return solutions.size();
}