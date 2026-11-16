#include "game.h"
#include "audio.h"

void initGame(GameState& g) {
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            g.board[r][c] = '.';
        }
    }
    g.turn = 0;
    g.gameOver = false;
    g.gameDraw = false;
    g.winner = '.';
    g.redFours = 0;
    g.yellowFours = 0;
    for (int i = 0; i < 4; i++) {
        g.winCells[i][0] = -1;
        g.winCells[i][1] = -1;
    }
}

bool dropPiece(char board[ROWS][COLS], int col, char piece) {
    if (col < 0 || col >= COLS) return false;
    for (int r = ROWS - 1; r >= 0; r--) {
        if (board[r][col] == '.') {
            board[r][col] = piece;
            return true;
        }
    }
    return false;
}

int checkWin(GameState& g) {
    // Horizontal
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c <= COLS - 4; c++) {
            char p = g.board[r][c];
            if (p != '.' && p == g.board[r][c + 1] && p == g.board[r][c + 2] && p == g.board[r][c + 3]) {
                for (int i = 0; i < 4; i++) { g.winCells[i][0] = r; g.winCells[i][1] = c + i; }
                return 1;
            }
        }
    }
    // Vertical
    for (int r = 0; r <= ROWS - 4; r++) {
        for (int c = 0; c < COLS; c++) {
            char p = g.board[r][c];
            if (p != '.' && p == g.board[r + 1][c] && p == g.board[r + 2][c] && p == g.board[r + 3][c]) {
                for (int i = 0; i < 4; i++) { g.winCells[i][0] = r + i; g.winCells[i][1] = c; }
                return 1;
            }
        }
    }
    // Diagonal down-right
    for (int r = 0; r <= ROWS - 4; r++) {
        for (int c = 0; c <= COLS - 4; c++) {
            char p = g.board[r][c];
            if (p != '.' && p == g.board[r + 1][c + 1] && p == g.board[r + 2][c + 2] && p == g.board[r + 3][c + 3]) {
                for (int i = 0; i < 4; i++) { g.winCells[i][0] = r + i; g.winCells[i][1] = c + i; }
                return 1;
            }
        }
    }
    // Diagonal up-right
    for (int r = 3; r < ROWS; r++) {
        for (int c = 0; c <= COLS - 4; c++) {
            char p = g.board[r][c];
            if (p != '.' && p == g.board[r - 1][c + 1] && p == g.board[r - 2][c + 2] && p == g.board[r - 3][c + 3]) {
                for (int i = 0; i < 4; i++) { g.winCells[i][0] = r - i; g.winCells[i][1] = c + i; }
                return 1;
            }
        }
    }
    return 0;
}

int checkWin(const char board[ROWS][COLS]) {
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c <= COLS - 4; c++) {
            char p = board[r][c];
            if (p != '.' && p == board[r][c + 1] && p == board[r][c + 2] && p == board[r][c + 3]) return 1;
        }
    }
    for (int r = 0; r <= ROWS - 4; r++) {
        for (int c = 0; c < COLS; c++) {
            char p = board[r][c];
            if (p != '.' && p == board[r + 1][c] && p == board[r + 2][c] && p == board[r + 3][c]) return 1;
        }
    }
    for (int r = 0; r <= ROWS - 4; r++) {
        for (int c = 0; c <= COLS - 4; c++) {
            char p = board[r][c];
            if (p != '.' && p == board[r + 1][c + 1] && p == board[r + 2][c + 2] && p == board[r + 3][c + 3]) return 1;
        }
    }
    for (int r = 3; r < ROWS; r++) {
        for (int c = 0; c <= COLS - 4; c++) {
            char p = board[r][c];
            if (p != '.' && p == board[r - 1][c + 1] && p == board[r - 2][c + 2] && p == board[r - 3][c + 3]) return 1;
        }
    }
    return 0;
}

int checkDraw(const char board[ROWS][COLS]) {
    for (int c = 0; c < COLS; c++) {
        if (board[0][c] == '.') return 0;
    }
    return 1;
}

int countConnectFours(const char board[ROWS][COLS], char piece) {
    int count = 0;

    // Horizontal
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c <= COLS - 4; c++) {
            if (board[r][c] == piece && board[r][c + 1] == piece &&
                board[r][c + 2] == piece && board[r][c + 3] == piece) {
                count++;
            }
        }
    }

    // Vertical
    for (int r = 0; r <= ROWS - 4; r++) {
        for (int c = 0; c < COLS; c++) {
            if (board[r][c] == piece && board[r + 1][c] == piece &&
                board[r + 2][c] == piece && board[r + 3][c] == piece) {
                count++;
            }
        }
    }

    // Diagonal down-right
    for (int r = 0; r <= ROWS - 4; r++) {
        for (int c = 0; c <= COLS - 4; c++) {
            if (board[r][c] == piece && board[r + 1][c + 1] == piece &&
                board[r + 2][c + 2] == piece && board[r + 3][c + 3] == piece) {
                count++;
            }
        }
    }

    // Diagonal up-right
    for (int r = 3; r < ROWS; r++) {
        for (int c = 0; c <= COLS - 4; c++) {
            if (board[r][c] == piece && board[r - 1][c + 1] == piece &&
                board[r - 2][c + 2] == piece && board[r - 3][c + 3] == piece) {
                count++;
            }
        }
    }

    return count;
}

bool playTurn(GameState& g, int col, GameRules rules) {
    if (g.gameOver || g.gameDraw) return false;

    char currentPiece = (g.turn == 0) ? 'X' : 'O';
    if (!dropPiece(g.board, col, currentPiece)) {
        return false;
    }

    // Always update live connected-four counts
    g.redFours = countConnectFours(g.board, 'X');
    g.yellowFours = countConnectFours(g.board, 'O');

    if (rules == GameRules::FirstFour) {
        if (checkWin(g) == 1) {
            g.gameOver = true;
            g.winner = currentPiece;
            playWinSound();
            return true;
        }
        if (checkDraw(g.board) == 1) {
            g.gameDraw = true;
            playDrawSound();
            return true;
        }
    } else {
        // Lab 2 Rules: Play until the 6x7 board is completely full, then compare connected-fours
        if (checkDraw(g.board) == 1) {
            if (g.redFours > g.yellowFours) {
                g.gameOver = true;
                g.winner = 'X';
                playWinSound();
            } else if (g.yellowFours > g.redFours) {
                g.gameOver = true;
                g.winner = 'O';
                playWinSound();
            } else {
                g.gameDraw = true;
                playDrawSound();
            }
            return true;
        }
    }

    playDropSound();
    g.turn = 1 - g.turn;
    return true;
}