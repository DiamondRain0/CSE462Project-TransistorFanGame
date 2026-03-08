#ifndef CONSTANTS_H
#define CONSTANTS_H

const int BOARD_SIZE = 7;
const int CELL_SIZE = 80;
const int OFFSET_X = 50;
const int OFFSET_Y = 50;
const int WIN_WIDTH = 800;
const int WIN_HEIGHT = 700;

const char EMPTY = '.';
const char REMOVED = 'X';
const char P1_PIECE = '1'; //AI
const char P2_PIECE = '2'; //Human

struct Point {
    int r, c;
    bool operator==(const Point& other) const { return r == other.r && c == other.c; }
};

struct TurnMove {
    Point moveDest;
    Point removeLoc;
};

#endif