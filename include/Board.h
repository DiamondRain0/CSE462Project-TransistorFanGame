#ifndef BOARD_H
#define BOARD_H

#include "Constants.h"
#include <vector>

class Board {
public:
    char grid[BOARD_SIZE][BOARD_SIZE];
    Point p1Pos; //AI
    Point p2Pos; //Human

    Board();

    //Logic methods
    bool isValid(int r, int c) const;
    bool canMoveTo(int r, int c) const;
    std::vector<Point> getPieceMoves(Point currentPos) const;
    bool hasNoMoves(bool isPlayer1) const;
};

#endif