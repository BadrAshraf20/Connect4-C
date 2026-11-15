#pragma once
#include "config.h"

struct GameState {
    char board[ROWS][COLS];
    int turn;          // 0 = Player 1 ('X'), 1 = Player 2 ('O')
    bool gameOver;
    bool gameDraw;
    char winner;
    int winCells[4][2]; // Stores {row, col} of the 4 winning tokens
};

void initGame(GameState& g);

// Pure board logic (these only need a board, which is handy for minimax)
bool dropPiece(char board[ROWS][COLS], int col, char player);
int checkWin(char board[ROWS][COLS], int winCells[4][2] = nullptr);
int checkDraw(char board[ROWS][COLS]);

// Plays one move for the current player; returns true if a piece was placed
bool playTurn(GameState& g, int col);