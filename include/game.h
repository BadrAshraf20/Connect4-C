#pragma once
#include "config.h"

struct GameState {
    char board[ROWS][COLS];
    int turn = 0; // 0 = Red ('X'), 1 = Yellow ('O')
    bool gameOver = false;
    bool gameDraw = false;
    char winner = '.';
    int winCells[4][2] = { {-1,-1}, {-1,-1}, {-1,-1}, {-1,-1} };
    int redFours = 0;
    int yellowFours = 0;
};

void initGame(GameState& g);
bool dropPiece(char board[ROWS][COLS], int col, char piece);
int checkWin(GameState& g);
int checkWin(const char board[ROWS][COLS]);
int checkDraw(const char board[ROWS][COLS]);
int countConnectFours(const char board[ROWS][COLS], char piece);
bool playTurn(GameState& g, int col, GameRules rules = GameRules::FullBoard);