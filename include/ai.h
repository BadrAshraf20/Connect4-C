#pragma once
#include "config.h"

int evaluateBoard(char board[ROWS][COLS], char aiPiece);

int getBestMove(char board[ROWS][COLS], char aiPiece, int maxDepth,
                AIAlgorithm algo = AIAlgorithm::AlphaBeta,
                SearchStats* outStats = nullptr, bool printTree = true);