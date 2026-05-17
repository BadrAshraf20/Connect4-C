#include "raylib.h"

#define ROWS 6
#define COLS 7
#define CELL_SIZE 100
#define PADDING 50


int checkWin(char s[ROWS][COLS]) {
    // Horizontal
    for (int r = 0; r < ROWS; r++) {
        for(int c = 0; c <= COLS - 4; c++) {
            if(s[r][c] !='.' && s[r][c]==s[r][c+1] && s[r][c]==s[r][c+2] && s[r][c]==s[r][c+3])
                return 1;
        }
    }

    // Vertical
    for (int r = 0; r <= ROWS - 4; r++) {
        for(int c = 0; c < COLS; c++) {
            if(s[r][c] !='.' && s[r][c]==s[r+1][c] && s[r][c]==s[r+2][c] && s[r][c]==s[r+3][c])
                return 1;
        }
    }    

    // Diagonal Right
    for (int r = 0; r <= ROWS - 4; r++) {
        for(int c = 0; c <= COLS - 4; c++) {
            if(s[r][c] !='.' && s[r][c]==s[r+1][c+1] && s[r][c]==s[r+2][c+2] && s[r][c]==s[r+3][c+3])
                return 1;
        }
    }

    // Diagonal Left
    for (int r = 0; r <= ROWS - 4; r++) {
        for(int c = 3; c < COLS; c++) { 
            if(s[r][c] !='.' && s[r][c]==s[r+1][c-1] && s[r][c]==s[r+2][c-2] && s[r][c]==s[r+3][c-3])
                return 1;
        }
    }
    
    return 0; 
}


int checkDraw(char s[ROWS][COLS]) {
    for(int c = 0; c < COLS; c++) {
        if(s[0][c] == '.') {
            return 0;
        }
    }

    return 1;
}


int main() {
    int turn = 0, flag = 0, gameOver = 0, gameDraw = 0;
    char board[ROWS][COLS];
    char winner = ' ';
    char playerChar;
    for (int r = 0; r < 6; r++) {
        for(int c = 0; c < 7; c++) {
            board[r][c] = '.';
        }
    }

    int width  = COLS * CELL_SIZE + PADDING * 2;
    int height = ROWS * CELL_SIZE + PADDING * 2;

    InitWindow(width, height, "Connect Four");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        flag = 0;
        if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !gameOver && !gameDraw) {
            int mouseX = GetMouseX();
            int col = (mouseX - PADDING) / CELL_SIZE;
            if(col >= 0 && col <=6) {
                for (int i = 5; i >= 0; i--) {
                    if(board[i][col] == '.') {
                        playerChar = (turn == 0) ? 'X' : 'O';
                        board[i][col] = playerChar;
                        flag = 1;
                        break;
                    }
                }
            }
            if(flag == 1) {
                if(checkWin(board) == 1) {
                    gameOver = 1;
                    winner = playerChar;
                }
                else if(checkDraw(board) == 1) {
                    gameDraw = 1;
                }
                turn = 1 - turn;
            }
        }

        BeginDrawing();
        ClearBackground(DARKBLUE);
        if (!gameOver && !gameDraw) {
            if (turn == 0)
                DrawText("Red's turn",   20, 10, 24, RED);
            else
                DrawText("Yellow's turn", 20, 10, 24, YELLOW);
        }
        // Draw the grid of circles
        for (int r = 0; r < ROWS; r++) {
            for (int c = 0; c < COLS; c++) {
                int x = PADDING + c * CELL_SIZE + CELL_SIZE / 2;
                int y = PADDING + r * CELL_SIZE + CELL_SIZE / 2;
                if(board[r][c] == '.')
                    DrawCircle(x, y, 40, DARKGRAY);
                else if(board[r][c] == 'X')
                    DrawCircle(x, y, 40, RED);
                else if(board[r][c] == 'O')
                    DrawCircle(x, y, 40, YELLOW);
            }
        }

        if (gameOver) {
            DrawRectangle(150, 260, 450, 70, BLACK);   // dark box
            if (winner == 'X')
                DrawText("  Red wins!", 220, 275, 50, RED);
            else
                DrawText("    Yellow wins!", 175, 275, 50, YELLOW);
        }

        else if(gameDraw) {
            DrawRectangle(150, 260, 450, 70, BLACK);
            DrawText("  Draw!", 220, 275, 50, GOLD);
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
