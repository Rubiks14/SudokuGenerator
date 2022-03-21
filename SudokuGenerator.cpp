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
};


typedef std::vector<Point> Points;

std::random_device rd;
std::mt19937 rngeesus(rd());
std::uniform_int_distribution<unsigned short> quarterDistribution(0, 3);
std::uniform_int_distribution<unsigned short> fullDistribution(0, 8);
auto quadrantCoordinate = std::bind(quarterDistribution, rngeesus);
auto boardCoordinate = std::bind(fullDistribution, rngeesus);

void fillBoard(Board& board);
void displayBoard(const Board& board);
const int solveBoard(Board board, Points points);

int main()
{
    Board board;
    Points points;
    Points removed;
    fillBoard(board);
    int clues{ WIDTH * HEIGHT };
    int solutions{ 0 };
    int threshold{ 0 };
    while (clues >= 20 && threshold < 20)
    {
        unsigned short row = quadrantCoordinate();
        unsigned short col = quadrantCoordinate();
        
        points.clear();
        if (clues >= (WIDTH * HEIGHT) - 30)
        {
            while (board[row * WIDTH + col] == 0)
            {
                row = quadrantCoordinate();
                col = quadrantCoordinate();
            }
            removed.push_back(Point(row, col, board[row * WIDTH + col]));
            board.removeCell(row, col);
            removed.push_back(Point((WIDTH - 1) - row, (HEIGHT - 1) - col, board[(HEIGHT - 1 - row) * WIDTH + (WIDTH - 1 - col)]));
            board.removeCell((WIDTH - 1) - row, (HEIGHT - 1) - col);
            clues -= 2;
        }
        else
        {
            while (board[row * WIDTH + col] == 0)
            {
                row = boardCoordinate();
                col = boardCoordinate();
            }
            removed.push_back(Point(row, col, board[row * WIDTH + col]));
            board.removeCell(row, col);
            --clues;
        }
        if (clues >= (WIDTH * HEIGHT) - 20)
        {
            removed.push_back(Point((WIDTH - 1) - row, col, board[(WIDTH - 1 - row) * WIDTH + col]));
            board.removeCell((WIDTH - 1) - row, col);
            removed.push_back(Point(row, (HEIGHT - 1) - col, board[row * WIDTH + (HEIGHT - 1 - col)]));
            board.removeCell(row, (HEIGHT - 1) - col);
            clues -= 2;
        }

        for (int iRow = 0; iRow < HEIGHT; ++iRow)
        {
            for (int jCol = 0; jCol < WIDTH; ++jCol)
            {
                const int number = board[iRow * WIDTH + jCol];
                if (number == 0)
                {
                    points.push_back(Point(iRow, jCol));
                }
            }
        }
 
        solutions = solveBoard(board, points);
        // Reinserts the last removed points from the list of removed points until
        // the board has a single solution
        while (solutions != 1 && !removed.empty())
        {
            auto removedPoint = removed.back();
            board.addCell(removedPoint.row, removedPoint.col, removedPoint.availability[0]);
            removed.pop_back();

            // I want to find a better way to handle this part. 
            for (auto pIter = points.begin(); pIter != points.end(); ++pIter)
            {
                if (pIter->row == removedPoint.row && pIter->col == removedPoint.col)
                {
                    points.erase(pIter);
                    break;
                }
            }
            solutions = solveBoard(board, points);
            ++clues;
            ++threshold;
        }
    }

    solutions = solveBoard(board, points);

    if (solutions > 1)
    {
        std::cout << "The puzzle has more than one solution" << std::endl;
    }
    else if (solutions == 1)
    {
        std::cout << "The puzzle has one solution" << std::endl;
    }
    else
    {
        std::cout << "The puzzle has no solution" << std::endl;
    }

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
            // if there is no availability for the current cell then the row is messed up and needs to be redone.
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
            board.addCell(row, col, availability[0]);
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
        if (i != 0 && i % 9 == 0)
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
                board.addCell(pointIter->row, pointIter->col, *availableIter);
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