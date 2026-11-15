#include "ai.h"
#include "game.h"
#include <algorithm>
#include <climits>

namespace {
    // Center-first column ordering maximizes Alpha-Beta cutoffs
    constexpr int COL_ORDER[COLS] = { 3, 2, 4, 1, 5, 0, 6 };

    // Removes the topmost piece in `col` to backtrack after a recursive call
    void undoPiece(char board[ROWS][COLS], int col) {
        for (int r = 0; r < ROWS; r++) {
            if (board[r][col] != '.') {
                board[r][col] = '.';
                return;
            }
        }
    }

    // Checks specifically if `p` ('X' or 'O') has 4 in a row
    bool hasPlayerWon(char board[ROWS][COLS], char p) {
        // Horizontal
        for (int r = 0; r < ROWS; r++) {
            for (int c = 0; c <= COLS - 4; c++) {
                if (board[r][c] == p && board[r][c+1] == p && board[r][c+2] == p && board[r][c+3] == p)
                    return true;
            }
        }
        // Vertical
        for (int r = 0; r <= ROWS - 4; r++) {
            for (int c = 0; c < COLS; c++) {
                if (board[r][c] == p && board[r+1][c] == p && board[r+2][c] == p && board[r+3][c] == p)
                    return true;
            }
        }
        // Diagonal Down-Right
        for (int r = 0; r <= ROWS - 4; r++) {
            for (int c = 0; c <= COLS - 4; c++) {
                if (board[r][c] == p && board[r+1][c+1] == p && board[r+2][c+2] == p && board[r+3][c+3] == p)
                    return true;
            }
        }
        // Diagonal Up-Right
        for (int r = 3; r < ROWS; r++) {
            for (int c = 0; c <= COLS - 4; c++) {
                if (board[r][c] == p && board[r-1][c+1] == p && board[r-2][c+2] == p && board[r-3][c+3] == p)
                    return true;
            }
        }
        return false;
    }
}

int evaluateWindow(const char window[4], char aiPiece) {
    int score = 0;
    char oppPiece = (aiPiece == 'X') ? 'O' : 'X';

    int aiCount = 0;
    int oppCount = 0;
    int emptyCount = 0;

    for (int i = 0; i < 4; i++) {
        if (window[i] == aiPiece) {
            aiCount++;
        } else if (window[i] == oppPiece) {
            oppCount++;
        } else {
            emptyCount++;
        }
    }

    if (aiCount == 4) {
        score += 100;
    } else if (aiCount == 3 && emptyCount == 1) {
        score += 5;
    } else if (aiCount == 2 && emptyCount == 2) {
        score += 2;
    }

    if (oppCount == 3 && emptyCount == 1) {
        score -= 80;
    }

    return score;
}

int evaluateBoard(char board[ROWS][COLS], char aiPiece) {
    int score = 0;

    // Center column preference
    int centerCol = COLS / 2;
    int centerCount = 0;
    for (int r = 0; r < ROWS; r++) {
        if (board[r][centerCol] == aiPiece) {
            centerCount++;
        }
    }
    score += centerCount * 3;

    // Horizontal windows
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c <= COLS - 4; c++) {
            char window[4] = { board[r][c], board[r][c+1], board[r][c+2], board[r][c+3] };
            score += evaluateWindow(window, aiPiece);
        }
    }

    // Vertical windows
    for (int r = 0; r <= ROWS - 4; r++) {
        for (int c = 0; c < COLS; c++) {
            char window[4] = { board[r][c], board[r+1][c], board[r+2][c], board[r+3][c] };
            score += evaluateWindow(window, aiPiece);
        }
    }

    // Positive diagonal windows (Down-Right)
    for (int r = 0; r <= ROWS - 4; r++) {
        for (int c = 0; c <= COLS - 4; c++) {
            char window[4] = { board[r][c], board[r+1][c+1], board[r+2][c+2], board[r+3][c+3] };
            score += evaluateWindow(window, aiPiece);
        }
    }

    // Negative diagonal windows (Up-Right)
    for (int r = 3; r < ROWS; r++) {
        for (int c = 0; c <= COLS - 4; c++) {
            char window[4] = { board[r][c], board[r-1][c+1], board[r-2][c+2], board[r-3][c+3] };
            score += evaluateWindow(window, aiPiece);
        }
    }

    return score;
}

int minimax(char board[ROWS][COLS], int depth, int alpha, int beta, bool isMaximizing, char aiPiece) {
    char oppPiece = (aiPiece == 'X') ? 'O' : 'X';

    // Terminal state checks
    if (hasPlayerWon(board, aiPiece)) {
        return 100000 + depth; // Prefer faster wins
    }
    if (hasPlayerWon(board, oppPiece)) {
        return -100000 - depth; // Delay losses as long as possible
    }
    if (checkDraw(board) == 1) {
        return 0;
    }
    if (depth == 0) {
        return evaluateBoard(board, aiPiece);
    }

    if (isMaximizing) {
        int maxEval = INT_MIN;
        for (int col : COL_ORDER) {
            if (board[0][col] == '.') {
                dropPiece(board, col, aiPiece);
                int eval = minimax(board, depth - 1, alpha, beta, false, aiPiece);
                undoPiece(board, col);

                maxEval = std::max(maxEval, eval);
                alpha = std::max(alpha, eval);
                if (beta <= alpha) {
                    break; // Beta cutoff
                }
            }
        }
        return maxEval;
    } else {
        int minEval = INT_MAX;
        for (int col : COL_ORDER) {
            if (board[0][col] == '.') {
                dropPiece(board, col, oppPiece);
                int eval = minimax(board, depth - 1, alpha, beta, true, aiPiece);
                undoPiece(board, col);

                minEval = std::min(minEval, eval);
                beta = std::min(beta, eval);
                if (beta <= alpha) {
                    break; // Alpha cutoff
                }
            }
        }
        return minEval;
    }
}

int getBestMove(char board[ROWS][COLS], char aiPiece, int maxDepth) {
    int bestScore = INT_MIN;
    int bestCol = -1;

    for (int col : COL_ORDER) {
        if (board[0][col] == '.') {
            dropPiece(board, col, aiPiece);
            int score = minimax(board, maxDepth - 1, INT_MIN, INT_MAX, false, aiPiece);
            undoPiece(board, col);

            if (score > bestScore) {
                bestScore = score;
                bestCol = col;
            }
        }
    }

    return bestCol;
}