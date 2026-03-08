#include "../include/AI.h"
#include <climits>
#include <algorithm>
#include <vector>
#include <iostream>

using namespace std;


const int MAX_DEPTH = 4;

//--- FAST LOOKUP TABLES ---
//Maps a 1D index (0-48) to a list of neighbor 1D indices.
int neighbors[49][8]; 
int neighborCount[49];

//Conversion helpers
inline int toIdx(int r, int c) { return r * BOARD_SIZE + c; }
inline Point toPoint(int idx) { return {idx / BOARD_SIZE, idx % BOARD_SIZE}; }

//Initialize tables
void initAI() {
    static bool initialized = false;
    if (initialized) return;

    int dr[] = {-1, -1, -1, 0, 0, 1, 1, 1};
    int dc[] = {-1, 0, 1, -1, 1, -1, 0, 1};

    for (int r = 0; r < BOARD_SIZE; r++) {
        for (int c = 0; c < BOARD_SIZE; c++) {
            int current = toIdx(r, c);
            int count = 0;
            for (int i = 0; i < 8; i++) {
                int nr = r + dr[i];
                int nc = c + dc[i];
                if (nr >= 0 && nr < BOARD_SIZE && nc >= 0 && nc < BOARD_SIZE) {
                    neighbors[current][count++] = toIdx(nr, nc);
                }
            }
            neighborCount[current] = count;
        }
    }
    initialized = true;
}

//--- EVALUATION ---
//Fast evaluation function
int evaluate(const Board& b) {
    int p1Idx = toIdx(b.p1Pos.r, b.p1Pos.c);
    int p2Idx = toIdx(b.p2Pos.r, b.p2Pos.c);
    
    int p1Mobility = 0;
    for(int i=0; i<neighborCount[p1Idx]; i++) {
        int n = neighbors[p1Idx][i];
        if(b.grid[n/BOARD_SIZE][n%BOARD_SIZE] == EMPTY) p1Mobility++;
    }

    int p2Mobility = 0;
    for(int i=0; i<neighborCount[p2Idx]; i++) {
        int n = neighbors[p2Idx][i];
        if(b.grid[n/BOARD_SIZE][n%BOARD_SIZE] == EMPTY) p2Mobility++;
    }

    //Heuristic: Maximize My Moves - (Opponent Moves * 2)
    //We multiply opponent moves by 2 to make AI aggressive
    return p1Mobility - (p2Mobility * 2);
}

//--- OPTIMIZED GENERATORS ---

//Get removals ONLY around the players. 
//Uses a simple bool array for speed.
//Returns a fixed-size array logic to avoid vector allocation overhead.
struct FastMoveList {
    int indices[20]; //Max 20 relevant removals
    int count = 0;
};

FastMoveList getRelevantRemovals(const Board& b) {
    FastMoveList list;
    bool visited[49] = {false};
    
    //1. Add Opponent Neighbors (Priority: Attack)
    //int pOppIdx = toIdx(b.p2Pos.r, b.p2Pos.c); 
    // ->If AI is thinking, opponent is P2
    // ->If we are simulating human turn, opponent is P1. 
    //We just check neighbors of BOTH for simplicity.

    int centers[] = { toIdx(b.p1Pos.r, b.p1Pos.c), toIdx(b.p2Pos.r, b.p2Pos.c) };

    for (int center : centers) {
        for (int i = 0; i < neighborCount[center]; i++) {
            int n = neighbors[center][i];
            if (!visited[n]) {
                if (b.grid[n/BOARD_SIZE][n%BOARD_SIZE] == EMPTY) {
                    list.indices[list.count++] = n;
                    visited[n] = true;
                    //Cap at 20 to prevent overflow
                    if (list.count >= 20) return list; 
                }
            }
        }
    }
    
    //Fallback: If trapped, pick ANY empty square
    if (list.count == 0) {
        for(int r=0; r<BOARD_SIZE; r++) {
            for(int c=0; c<BOARD_SIZE; c++) {
                if(b.grid[r][c] == EMPTY) {
                    list.indices[list.count++] = toIdx(r, c);
                    return list;
                }
            }
        }
    }
    return list;
}

//--- MINMAX ENGINE ---

int minMax(Board& b, int depth, bool isMaximizing, int alpha, int beta) {
    //Base case
    if (depth == 0) return evaluate(b);

    if (isMaximizing) {
        int maxEval = -100000;
        
        //1. Get Piece Moves
        Point orig = b.p1Pos;
        int origIdx = toIdx(orig.r, orig.c);
        
        //Use static table for moves
        int moves[8];
        int mCount = 0;
        for(int i=0; i<neighborCount[origIdx]; i++) {
            int n = neighbors[origIdx][i];
            if(b.grid[n/BOARD_SIZE][n%BOARD_SIZE] == EMPTY) moves[mCount++] = n;
        }

        if (mCount == 0) return -5000; //AI Lost

        //2. Iterate Moves
        for (int i=0; i<mCount; i++) {
            int moveIdx = moves[i];
            Point pm = toPoint(moveIdx);

            //Apply Move
            b.grid[orig.r][orig.c] = EMPTY;
            b.grid[pm.r][pm.c] = P1_PIECE;
            b.p1Pos = pm;

            //3. Iterate Removals
            FastMoveList removals = getRelevantRemovals(b);
            
            for (int j=0; j<removals.count; j++) {
                int remIdx = removals.indices[j];
                Point rem = toPoint(remIdx);

                b.grid[rem.r][rem.c] = REMOVED;

                int eval = minMax(b, depth - 1, false, alpha, beta);

                b.grid[rem.r][rem.c] = EMPTY;

                if (eval > maxEval) maxEval = eval;
                if (eval > alpha) alpha = eval;
                if (beta <= alpha) break; //Pruning
            }

            //Undo Move
            b.grid[pm.r][pm.c] = EMPTY;
            b.grid[orig.r][orig.c] = P1_PIECE;
            b.p1Pos = orig;

            if (beta <= alpha) break; //Pruning
        }
        return maxEval;

    } else { //Minimizing (Human)
        int minEval = 100000;
        
        Point orig = b.p2Pos;
        int origIdx = toIdx(orig.r, orig.c);

        int moves[8];
        int mCount = 0;
        for(int i=0; i<neighborCount[origIdx]; i++) {
            int n = neighbors[origIdx][i];
            if(b.grid[n/BOARD_SIZE][n%BOARD_SIZE] == EMPTY) moves[mCount++] = n;
        }

        if (mCount == 0) return 5000; //Human Lost

        for (int i=0; i<mCount; i++) {
            int moveIdx = moves[i];
            Point pm = toPoint(moveIdx);

            b.grid[orig.r][orig.c] = EMPTY;
            b.grid[pm.r][pm.c] = P2_PIECE;
            b.p2Pos = pm;

            FastMoveList removals = getRelevantRemovals(b);

            for (int j=0; j<removals.count; j++) {
                int remIdx = removals.indices[j];
                Point rem = toPoint(remIdx);

                b.grid[rem.r][rem.c] = REMOVED;

                int eval = minMax(b, depth - 1, true, alpha, beta);

                b.grid[rem.r][rem.c] = EMPTY;

                if (eval < minEval) minEval = eval;
                if (eval < beta) beta = eval;
                if (beta <= alpha) break;
            }

            b.grid[pm.r][pm.c] = EMPTY;
            b.grid[orig.r][orig.c] = P2_PIECE;
            b.p2Pos = orig;

            if (beta <= alpha) break;
        }
        return minEval;
    }
}


//Main AI entry point
TurnMove getBestMove(Board b, int depth) {
    initAI();
    
    int bestVal = -200000;
    TurnMove bestMove = {{-1,-1}, {-1,-1}};

    Point orig = b.p1Pos;
    int origIdx = toIdx(orig.r, orig.c);
    
    //Find every square the AI can physically reach right now.
    vector<int> validMoves;
    for(int i=0; i<neighborCount[origIdx]; i++) {
        int n = neighbors[origIdx][i];
        if(b.grid[n/BOARD_SIZE][n%BOARD_SIZE] == EMPTY) validMoves.push_back(n);
    }

    //Test every possible move and removal to see which leads to the best future.
    for (int moveIdx : validMoves) {
        Point pm = toPoint(moveIdx);

        b.grid[orig.r][orig.c] = EMPTY;
        b.grid[pm.r][pm.c] = P1_PIECE;
        b.p1Pos = pm;

        FastMoveList removals = getRelevantRemovals(b);

        //Sort Removals: Prioritize blocking the human's immediate area.
        //This makes the search much faster by finding killer moves early.
        Point opp = b.p2Pos;
        for (int k = 0; k < removals.count; k++) {
            if (abs(toPoint(removals.indices[k]).r - opp.r) <= 1 && 
                abs(toPoint(removals.indices[k]).c - opp.c) <= 1) {
                swap(removals.indices[k], removals.indices[0]);
            }
        }

        for (int j = 0; j < removals.count; j++) {
            int remIdx = removals.indices[j];
            Point rem = toPoint(remIdx);

            b.grid[rem.r][rem.c] = REMOVED;

            //Start the deep simulation from this starting move.
            int val = minMax(b, depth, false, -200000, 200000);

            b.grid[rem.r][rem.c] = EMPTY;

            //If this move leads to a better future than anything we've seen, remember it.
            if (val > bestVal) {
                bestVal = val;
                bestMove.moveDest = pm;
                bestMove.removeLoc = rem;
            }
        }
        
        b.grid[pm.r][pm.c] = EMPTY;
        b.grid[orig.r][orig.c] = P1_PIECE;
        b.p1Pos = orig;
    }

    return bestMove;
}