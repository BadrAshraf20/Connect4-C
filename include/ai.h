#pragma once
#include "config.h"

// Main entry point called by main.cpp when it's the AI's turn.
// Returns the best column (0 to COLS - 1) for aiPiece ('O' or 'X').
int getBestMove(char board[ROWS][COLS], char aiPiece, int maxDepth = 6);

// Recursive Minimax search with Alpha-Beta pruning.
int minimax(char board[ROWS][COLS], int depth, int alpha, int beta, bool isMaximizing, char aiPiece);

// Heuristic evaluation function for non-terminal board states.
int evaluateBoard(char board[ROWS][COLS], char aiPiece);

// Helper to score a single 4-cell window (horizontal, vertical, or diagonal).
int evaluateWindow(const char window[4], char aiPiece);