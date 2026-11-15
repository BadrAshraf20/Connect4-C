#include "raylib.h"
#include "config.h"
#include "game.h"
#include "input.h"
#include "render.h"
#include "audio.h"
#include "ai.h"
#include <cmath>
#include <algorithm>

namespace {
    int getRandomValidColumn(const char board[ROWS][COLS]) {
        int validCols[COLS];
        int count = 0;
        for (int c = 0; c < COLS; c++) {
            if (board[0][c] == '.') {
                validCols[count++] = c;
            }
        }
        if (count == 0) return -1;
        return validCols[GetRandomValue(0, count - 1)];
    }
}

int main() {
    GameState game;
    initGame(game);

    ScreenState screen = ScreenState::Menu;
    GameMode mode = GameMode::HumanVsAI;
    AIDifficulty difficulty = AIDifficulty::Hard;
    SessionScore score{};

    SetConfigFlags(FLAG_FULLSCREEN_MODE | FLAG_MSAA_4X_HINT);
    int monitor = GetCurrentMonitor();
    InitWindow(GetMonitorWidth(monitor), GetMonitorHeight(monitor), "Connect Four");
    initAudio();
    SetTargetFPS(60);

    float aiTimer = 0.0f;
    float humanTimer = HUMAN_TURN_TIME;
    int selectedCol = COLS / 2;
    bool roundRecorded = false;

    while (!WindowShouldClose()) {
        if (screen == ScreenState::Menu) {
            int choice = drawMenu(difficulty);
            if (IsKeyPressed(KEY_ONE))   choice = 0;
            if (IsKeyPressed(KEY_TWO))   choice = 1;
            if (IsKeyPressed(KEY_THREE)) choice = 2;

            if (choice != -1) {
                mode = (choice == 0) ? GameMode::HumanVsHuman :
                       (choice == 1) ? GameMode::HumanVsAI : GameMode::AIVsAI;
                score = SessionScore{}; // Fresh scoreboard when starting a new mode
                initGame(game);
                aiTimer = (mode == GameMode::AIVsAI) ? 0.65f : 0.0f;
                humanTimer = HUMAN_TURN_TIME;
                selectedCol = COLS / 2;
                roundRecorded = false;
                screen = ScreenState::Playing;
            }
        }
        else if (screen == ScreenState::Playing) {
            if (!game.gameOver && !game.gameDraw) {
                bool isAITurn = (mode == GameMode::AIVsAI) ||
                                (mode == GameMode::HumanVsAI && game.turn == 1);

                if (!isAITurn) {
                    float prevTimer = humanTimer;
                    humanTimer -= GetFrameTime();

                    if (humanTimer > 0.0f && humanTimer <= 5.0f) {
                        int prevSec = (int)std::ceil(prevTimer);
                        int currSec = (int)std::ceil(humanTimer);
                        if (currSec < prevSec) {
                            playTickSound(currSec == 1);
                        }
                    }

                    int col = getSelectedColumn(selectedCol);
                    if (humanTimer <= 0.0f) {
                        col = getRandomValidColumn(game.board);
                    }

                    if (col != -1 && playTurn(game, col)) {
                        humanTimer = HUMAN_TURN_TIME;
                        if (mode == GameMode::HumanVsAI) {
                            aiTimer = GetRandomValue(60, 110) / 100.0f;
                        }
                    }
                } else {
                    aiTimer -= GetFrameTime();
                    if (aiTimer <= 0.0f) {
                        char aiPiece = (game.turn == 0) ? 'X' : 'O';
                        int baseDepth = static_cast<int>(difficulty);
                        int depth = (mode == GameMode::AIVsAI && game.turn == 0)
                                    ? std::max(1, baseDepth - 1)
                                    : baseDepth;

                        int aiCol = getBestMove(game.board, aiPiece, depth);

                        if (aiCol != -1 && playTurn(game, aiCol)) {
                            humanTimer = HUMAN_TURN_TIME;
                            if (mode == GameMode::AIVsAI) {
                                aiTimer = GetRandomValue(65, 110) / 100.0f;
                            }
                        }
                    }
                }
            }

            // Record match outcome into SessionScore once per round
            if (!roundRecorded && (game.gameOver || game.gameDraw)) {
                if (game.gameOver) {
                    if (game.winner == 'X') score.redWins++;
                    else if (game.winner == 'O') score.yellowWins++;
                } else {
                    score.draws++;
                }
                roundRecorded = true;
            }

            int action = drawGame(game, mode, difficulty, humanTimer, score, selectedCol);

            if ((game.gameOver || game.gameDraw) && (action == 1 || IsKeyPressed(KEY_R))) {
                initGame(game);
                aiTimer = (mode == GameMode::AIVsAI) ? 0.65f : 0.0f;
                humanTimer = HUMAN_TURN_TIME;
                selectedCol = COLS / 2;
                roundRecorded = false;
            }
            else if (action == 2 || IsKeyPressed(KEY_M)) {
                screen = ScreenState::Menu;
            }
        }
    }

    closeAudio();
    CloseWindow();
    return 0;
}