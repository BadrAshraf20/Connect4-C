#include "game.h"
#include "audio.h"

namespace {
    void recordWinCells(int winCells[4][2], int r0, int c0, int dr, int dc) {
        if (!winCells) return;
        for (int i = 0; i < 4; i++) {
            winCells[i][0] = r0 + i * dr;
            winCells[i][1] = c0 + i * dc;
        }
    }
}

void initGame(GameState& g) {
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            g.board[r][c] = '.';
        }
    }
    for (int i = 0; i < 4; i++) {
        g.winCells[i][0] = -1;
        g.winCells[i][1] = -1;
    }
    g.turn = 0;
    g.gameOver = false;
    g.gameDraw = false;
    g.winner = ' ';
}

bool dropPiece(char board[ROWS][COLS], int col, char player) {
    if (col < 0 || col >= COLS) return false;
    for (int r = ROWS - 1; r >= 0; r--) {
        if (board[r][col] == '.') {
            board[r][col] = player;
            return true;
        }
    }
    return false;
}

int checkWin(char board[ROWS][COLS], int winCells[4][2]) {
    // Horizontal
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c <= COLS - 4; c++) {
            char p = board[r][c];
            if (p != '.' && p == board[r][c+1] && p == board[r][c+2] && p == board[r][c+3]) {
                recordWinCells(winCells, r, c, 0, 1);
                return 1;
            }
        }
    }
    // Vertical
    for (int r = 0; r <= ROWS - 4; r++) {
        for (int c = 0; c < COLS; c++) {
            char p = board[r][c];
            if (p != '.' && p == board[r+1][c] && p == board[r+2][c] && p == board[r+3][c]) {
                recordWinCells(winCells, r, c, 1, 0);
                return 1;
            }
        }
    }
    // Diagonal Down-Right
    for (int r = 0; r <= ROWS - 4; r++) {
        for (int c = 0; c <= COLS - 4; c++) {
            char p = board[r][c];
            if (p != '.' && p == board[r+1][c+1] && p == board[r+2][c+2] && p == board[r+3][c+3]) {
                recordWinCells(winCells, r, c, 1, 1);
                return 1;
            }
        }
    }
    // Diagonal Up-Right
    for (int r = 3; r < ROWS; r++) {
        for (int c = 0; c <= COLS - 4; c++) {
            char p = board[r][c];
            if (p != '.' && p == board[r-1][c+1] && p == board[r-2][c+2] && p == board[r-3][c+3]) {
                recordWinCells(winCells, r, c, -1, 1);
                return 1;
            }
        }
    }
    return 0;
}

int checkDraw(char board[ROWS][COLS]) {
    for (int c = 0; c < COLS; c++) {
        if (board[0][c] == '.') return 0;
    }
    return 1;
}

bool playTurn(GameState& g, int col) {
    if (g.gameOver || g.gameDraw) return false;

    char currentPiece = (g.turn == 0) ? 'X' : 'O';
    if (!dropPiece(g.board, col, currentPiece)) {
        return false;
    }

    if (checkWin(g.board, g.winCells)) {
        g.gameOver = true;
        g.winner = currentPiece;
        playWinSound();
    } else if (checkDraw(g.board)) {
        g.gameDraw = true;
        playDrawSound();
    } else {
        g.turn = 1 - g.turn;
        playDropSound();
    }
    return true;
}