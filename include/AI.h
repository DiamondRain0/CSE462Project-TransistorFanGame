#ifndef AI_H
#define AI_H

#include "Board.h"

void initAI();

//Gets speified depth minmax AI move
TurnMove getBestMove(Board b, int depth);

#endif