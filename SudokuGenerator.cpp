#include <iostream>
#include <random>
#include <thread>
#include <chrono>
#include <array>
#include <algorithm>
#include <functional>

#include "Board.h"


// Places to refactor

// the points that are actively being removed from the puzzle are
// being stored twice. This seems like a waste. Is it possible to only store them once?
// One solution is to track each cell as they are removed and then when the list of cells
// is passed into the solving algorithm then they are ordered by row and column so that
// they are worked from top to bottom, left to right.

// The availability is being attached to the point struct but doesn't really need to be there.
// is there a better way to store that information so it is only being stored when needed and
// not wasting memory?

// the code used to backtrack through the solution is duplicated in the solveBoard function. It is
// being used when there is no availability for the current cell and again when a solution is found
// and the algorithm is backtracking through the previous solution to look for alternate solutions

struct Point
{
    unsigned short row{ 0 }, col{ 0 };
    std::vector<unsigned short> availability;
    explicit Point(unsigned short r, unsigned short c) : row{ r }, col{ c } {}
    explicit Point(unsigned short r, unsigned short c, unsigned short value) : row{r}, col{c} 
    {
        availability.push_back(value);
    }
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
std::uniform_int_distribution<unsigned short> quarterDistribution(0, 3);
std::uniform_int_distribution<unsigned short> fullDistribution(0, 8);
std::uniform_int_distribution<unsigned short> allCells(0, WIDTH* HEIGHT - 1);
auto quadrantCoordinate = std::bind(quarterDistribution, rngeesus);
auto boardCoordinate = std::bind(fullDistribution, rngeesus);
auto randomCell = std::bind(allCells, rngeesus);

void fillBoard(Board& board);
void displayBoard(const Board& board);
const int solveBoard(Board board, Points points);

int main()
{
    Board board;
    std::vector<unsigned short> values;
    Points removed;
    fillBoard(board);
    int clues{ WIDTH * HEIGHT };
    int solutions{ 0 };
    int threshold{ 0 };

    while (clues >= 20 && threshold < 20)
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

        if (clues < WIDTH * HEIGHT - 40)
        {
            removed.push_back(firstQuad);
            values.push_back(board.getCell(firstQuad.row, firstQuad.col));
            --clues;
        }
        else if (clues < WIDTH * HEIGHT - 20)
        {
            if (firstQuad == fourthQuad)
            {
                continue;
            }
            removed.push_back(firstQuad);
            values.push_back(board.getCell(firstQuad.row, firstQuad.col));
            removed.push_back(fourthQuad);
            values.push_back(board.getCell(fourthQuad.row, fourthQuad.col));
            clues -= 2;
        }
        else
        {
            if (firstQuad == secondQuad || firstQuad == thirdQuad || firstQuad == fourthQuad)
            {
                continue;
            }
            removed.push_back(firstQuad);
            values.push_back(board.getCell(firstQuad.row, firstQuad.col));
            removed.push_back(secondQuad);
            values.push_back(board.getCell(secondQuad.row, secondQuad.col));
            removed.push_back(thirdQuad);
            values.push_back(board.getCell(thirdQuad.row, thirdQuad.col));
            removed.push_back(fourthQuad);
            values.push_back(board.getCell(fourthQuad.row, fourthQuad.col));
            clues -= 4;
        }

        for (auto point : removed)
        {
            board.removeCell(point.row, point.col);
        }
 
        solutions = solveBoard(board, removed);
        // Reinserts the last removed points from the list of removed points until
        // the board has a single solution
        while (solutions != 1 && !removed.empty())
        {
            auto removedPoint = removed.back();
            board.alterCell(removedPoint.row, removedPoint.col, values.back());
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
    unsigned short row = 0;
    unsigned short col = 0;
    unsigned short messedUpCount = 0;

    while (row < HEIGHT)
    {
        while (col < WIDTH)
        {
            std::vector<unsigned short> availability = board.getAvailability(row, col);
            // if there is no availability for the current cell then the row is messed up and needs to be rerandomized.
            if (availability.size() == 0)
            {
                board.clearRow(row);
                col = 0;
                ++messedUpCount;
                if (messedUpCount == 5)
                {
                    for (int i = 0; i < HEIGHT; ++i)
                    {
                        board.clearRow(i);
                        col = 0;
                        row = 0;
                    }
                }
                continue;
            }
            std::shuffle(availability.begin(), availability.end(), rngeesus);
            board.alterCell(row, col, availability[0]);
            ++col;
        }
        col = 0;
        ++row;
    }
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

    // sort the points to make finding the solution easier
    std::stable_sort(points.begin(), points.end());

    std::vector<std::vector<unsigned short>> solutions;
    std::vector<unsigned short> solution;

    auto pointIter = points.begin();
    pointIter->availability = board.getAvailability(pointIter->row, pointIter->col);
    auto availableIter = pointIter->availability.begin();
    while (pointIter != points.end())
    {
        while (true)
        {
            if (availableIter != pointIter->availability.end())
            {
                solution.push_back(*availableIter);
                board.alterCell(pointIter->row, pointIter->col, *availableIter);
                ++availableIter;
                break;
            }
            else if (availableIter == pointIter->availability.end() && solution.size() < points.size())
            {
                if (pointIter == points.begin())
                    break;

                --pointIter;
                for (auto iter = pointIter->availability.begin(); iter != pointIter->availability.end(); ++iter)
                {
                    if (board[pointIter->row * WIDTH + pointIter->col] == *iter)
                    {
                        board.removeCell(pointIter->row, pointIter->col);
                        solution.pop_back();
                        availableIter = iter + 1;
                        break;
                    }
                }
            }
        }
        if (solution.size() == points.size())
        {
            solutions.push_back(solution);
            board.removeCell(pointIter->row, pointIter->col);
            solution.pop_back();
            if (pointIter != points.begin())
            {
                --pointIter;
            }
            else
                break;
            for (auto iter = pointIter->availability.begin(); iter != pointIter->availability.end(); ++iter)
            {
                if (board[pointIter->row * WIDTH + pointIter->col] == *iter)
                {
                    board.removeCell(pointIter->row, pointIter->col);
                    solution.pop_back();
                    availableIter = iter + 1;
                    break;
                }
            }
        }
        else if (solution.empty() && pointIter == points.begin())
        {
            break;
        }
        else
        {
            ++pointIter;
            pointIter->availability = board.getAvailability(pointIter->row, pointIter->col);
            availableIter = pointIter->availability.begin();
        }
    }
    return solutions.size();
}