#include "../include/Board.h"

Board::Board() {
    //Initialize empty grid
    for (int i = 0; i < BOARD_SIZE; i++)
        for (int j = 0; j < BOARD_SIZE; j++)
            grid[i][j] = EMPTY;
    
    //Set initial positions
    p1Pos = {0, 3}; //AI
    p2Pos = {6, 3}; //Human
    grid[p1Pos.r][p1Pos.c] = P1_PIECE;
    grid[p2Pos.r][p2Pos.c] = P2_PIECE;
}

bool Board::isValid(int r, int c) const {
    return (r >= 0 && r < BOARD_SIZE && c >= 0 && c < BOARD_SIZE);
}

bool Board::canMoveTo(int r, int c) const {
    return isValid(r, c) && grid[r][c] == EMPTY;
}

std::vector<Point> Board::getPieceMoves(Point currentPos) const {
    std::vector<Point> moves;
    int dr[] = {-1, -1, -1, 0, 0, 1, 1, 1};
    int dc[] = {-1, 0, 1, -1, 1, -1, 0, 1};

    for (int i = 0; i < 8; i++) {
        int nr = currentPos.r + dr[i];
        int nc = currentPos.c + dc[i];
        if (canMoveTo(nr, nc)) {
            moves.push_back({nr, nc});
        }
    }
    return moves;
}

bool Board::hasNoMoves(bool isPlayer1) const {
    return getPieceMoves(isPlayer1 ? p1Pos : p2Pos).empty();
}