#include "game.h"
#include "audio.h"

void initGame(GameState& g) {
    g.turn = 0;
    g.gameOver = false;
    g.gameDraw = false;
    g.winner = ' ';

    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            g.board[r][c] = '.';
        }
    }
}

int dropPiece(char board[ROWS][COLS], int col, char player) {
    for (int i = ROWS - 1; i >= 0; i--) {
        if (board[i][col] == '.') {
            board[i][col] = player;
            return i;
        }
    }
    return -1;
}

int checkWin(char s[ROWS][COLS]) {
    // Horizontal
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c <= COLS - 4; c++) {
            if (s[r][c] != '.' && s[r][c] == s[r][c+1] && s[r][c] == s[r][c+2] && s[r][c] == s[r][c+3])
                return 1;
        }
    }

    // Vertical
    for (int r = 0; r <= ROWS - 4; r++) {
        for (int c = 0; c < COLS; c++) {
            if (s[r][c] != '.' && s[r][c] == s[r+1][c] && s[r][c] == s[r+2][c] && s[r][c] == s[r+3][c])
                return 1;
        }
    }

    // Diagonal Right
    for (int r = 0; r <= ROWS - 4; r++) {
        for (int c = 0; c <= COLS - 4; c++) {
            if (s[r][c] != '.' && s[r][c] == s[r+1][c+1] && s[r][c] == s[r+2][c+2] && s[r][c] == s[r+3][c+3])
                return 1;
        }
    }

    // Diagonal Left
    for (int r = 0; r <= ROWS - 4; r++) {
        for (int c = 3; c < COLS; c++) {
            if (s[r][c] != '.' && s[r][c] == s[r+1][c-1] && s[r][c] == s[r+2][c-2] && s[r][c] == s[r+3][c-3])
                return 1;
        }
    }

    return 0;
}

int checkDraw(char s[ROWS][COLS]) {
    for (int c = 0; c < COLS; c++) {
        if (s[0][c] == '.') {
            return 0;
        }
    }
    return 1;
}

bool playTurn(GameState& g, int col) {
    if (g.gameOver || g.gameDraw) return false;
    if (col < 0 || col > COLS - 1) return false;

    char playerChar = (g.turn == 0) ? 'X' : 'O';

    if (dropPiece(g.board, col, playerChar) == -1)
        return false;

    playDropSound();

    if (checkWin(g.board) == 1) {
        g.gameOver = true;
        g.winner = playerChar;
        playWinSound();
    }
    else if (checkDraw(g.board) == 1) {
        g.gameDraw = true;
        playDrawSound();
    }
    g.turn = 1 - g.turn;

    return true;
}