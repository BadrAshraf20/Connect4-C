#pragma once
#include "config.h"

struct GameState {
    char board[ROWS][COLS];
    int  turn;       // 0 = Red (X), 1 = Yellow (O)
    bool gameOver;
    bool gameDraw;
    char winner;
};

void initGame(GameState& g);

// Pure board logic (these only need a board, which is handy for minimax)
int  dropPiece(char board[ROWS][COLS], int col, char player); // returns row, or -1 if column is full
int  checkWin(char s[ROWS][COLS]);
int  checkDraw(char s[ROWS][COLS]);

// Plays one move for the current player; returns true if a piece was placed
bool playTurn(GameState& g, int col);