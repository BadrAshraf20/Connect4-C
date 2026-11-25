#include "raylib.h"
#include "config.h"
#include "game.h"
#include "input.h"
#include "render.h"
#include "audio.h"
#include "ai.h"
#include <cmath>
#include <algorithm>

// Connects all parts of the project together and manages the game life cycle
// (Game Loop) from the moment the window is opened until it is closed.

namespace {
    // Iterate through the top row of the board (`board[0][c]`), identify the
    // columns that still have an empty space (`.`), collect them into an array.
    // If the human player runs out of time (humanTimer <= 0) without making
    // a move, the game automatically drops a random piece for them.
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

    // returns the search tree depth (K) for the AI; if the difficulty is set
    // to "Custom," it returns the value chosen by the user (customK).
    // Otherwise, it converts the difficulty level (Easy=2, Medium=4, Hard=6)
    // from enum into an integer.
    int getEffectiveDepth(AIDifficulty diff, int customK) {
        if (diff == AIDifficulty::Custom) return std::max(1, customK);
        return static_cast<int>(diff);
    }
}

int main() {
    GameState game;
    initGame(game);

    ScreenState screen = ScreenState::Menu;
    GameMode mode = GameMode::HumanVsAI;
    GameRules rules = GameRules::FullBoard;
    AIAlgorithm algorithm = AIAlgorithm::AlphaBeta;
    AIDifficulty difficulty = AIDifficulty::Medium;
    int customK = 5;

    SessionScore score{};
    SearchStats lastStats{};

    SetConfigFlags(FLAG_FULLSCREEN_MODE | FLAG_MSAA_4X_HINT);
    int monitor = GetCurrentMonitor();
    InitWindow(GetMonitorWidth(monitor), GetMonitorHeight(monitor), "Connect Four");
    /*
        To cancel the full-screen use that code
    */
    // SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
    // InitWindow(1280, 820, "Connect Four");
    // SetWindowMinSize(960, 680);
    // MaximizeWindow();
    initAudio();
    SetTargetFPS(60);

    // A simple delay timer, so the AI ​​doesn't respond in a fraction of a
    // second—making it seem as though it’s actually thinking.
    float aiTimer = 0.0f;
    float humanTimer = HUMAN_TURN_TIME;
    int selectedCol = COLS / 2;
    bool roundRecorded = false;

    int hintsRemaining[2] = { MAX_HINTS_PER_PLAYER, MAX_HINTS_PER_PLAYER };
    int hintCol = -1;

    bool showTreeOverlay = false;
    float treeScrollY = 0.0f;

    // It repeats 60 times per second as long as the user has the game open.
    while (!WindowShouldClose()) {
        int activeK = getEffectiveDepth(difficulty, customK);

        if (screen == ScreenState::Menu) {
            int choice = drawMenu(rules, algorithm, difficulty, customK);
            if (difficulty != AIDifficulty::Custom) {
                if (IsKeyPressed(KEY_ONE))   choice = 0;
                if (IsKeyPressed(KEY_TWO))   choice = 1;
                if (IsKeyPressed(KEY_THREE)) choice = 2;
            }

            // Setting up a new game session as soon as the player selects the game mode.
            if (choice != -1) {
                mode = (choice == 0) ? GameMode::HumanVsHuman :
                       (choice == 1) ? GameMode::HumanVsAI : GameMode::AIVsAI;
                score = SessionScore{};
                lastStats = SearchStats{};
                initGame(game);
                aiTimer = (mode == GameMode::AIVsAI) ? 0.65f : 0.0f;
                humanTimer = HUMAN_TURN_TIME;
                selectedCol = COLS / 2;
                roundRecorded = false;
                hintsRemaining[0] = MAX_HINTS_PER_PLAYER;
                hintsRemaining[1] = MAX_HINTS_PER_PLAYER;
                hintCol = -1;
                showTreeOverlay = false;
                treeScrollY = 0.0f;
                screen = ScreenState::Playing;
            }
        }
        // Manages the game cycle, the turn timer, and updates the score and hints.
        else if (screen == ScreenState::Playing) {
            bool isAITurn = (mode == GameMode::AIVsAI) ||
                            (mode == GameMode::HumanVsAI && game.turn == 1);

            if (!game.gameOver && !game.gameDraw && !showTreeOverlay) {
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

                    if (col != -1 && playTurn(game, col, rules)) {
                        humanTimer = HUMAN_TURN_TIME;
                        hintCol = -1;
                        if (mode == GameMode::HumanVsAI) {
                            aiTimer = GetRandomValue(60, 110) / 100.0f;
                        }
                    }
                } else {
                    aiTimer -= GetFrameTime();
                    if (aiTimer <= 0.0f) {
                        char aiPiece = (game.turn == 0) ? 'X' : 'O';
                        int depth = (mode == GameMode::AIVsAI && game.turn == 0)
                                    ? std::max(1, activeK - 1)
                                    : activeK;

                        int aiCol = getBestMove(game.board, aiPiece, depth, algorithm, rules, &lastStats, true);

                        if (aiCol != -1 && playTurn(game, aiCol, rules)) {
                            humanTimer = HUMAN_TURN_TIME;
                            hintCol = -1;
                            if (mode == GameMode::AIVsAI) {
                                aiTimer = GetRandomValue(65, 110) / 100.0f;
                            }
                        }
                    }
                }
            }

            if (!roundRecorded && (game.gameOver || game.gameDraw)) {
                if (game.gameOver) {
                    if (game.winner == 'X') score.redWins++;
                    else if (game.winner == 'O') score.yellowWins++;
                } else {
                    score.draws++;
                }
                roundRecorded = true;
            }

            int activeHints = (!isAITurn) ? hintsRemaining[game.turn] : 0;
            int action = drawGame(game, mode, rules, algorithm, activeK, humanTimer, score,
                                  selectedCol, activeHints, hintCol, lastStats,
                                  showTreeOverlay, treeScrollY);

            if (mode != GameMode::HumanVsHuman && (action == 4 || IsKeyPressed(KEY_T))) {
                showTreeOverlay = !showTreeOverlay;
                if (showTreeOverlay) treeScrollY = 0.0f;
            }

            if (!showTreeOverlay && !game.gameOver && !game.gameDraw && !isAITurn &&
                (action == 3 || IsKeyPressed(KEY_H))) {
                if (hintsRemaining[game.turn] > 0 && hintCol == -1) {
                    char playerPiece = (game.turn == 0) ? 'X' : 'O';
                    hintCol = getBestMove(game.board, playerPiece, 6, AIAlgorithm::AlphaBeta, rules, nullptr, false);
                    if (hintCol != -1) {
                        selectedCol = hintCol;
                        hintsRemaining[game.turn]--;
                    }
                }
            }

            if (!showTreeOverlay && (game.gameOver || game.gameDraw) && (action == 1 || IsKeyPressed(KEY_R))) {
                initGame(game);
                lastStats = SearchStats{};
                aiTimer = (mode == GameMode::AIVsAI) ? 0.65f : 0.0f;
                humanTimer = HUMAN_TURN_TIME;
                selectedCol = COLS / 2;
                roundRecorded = false;
                hintsRemaining[0] = MAX_HINTS_PER_PLAYER;
                hintsRemaining[1] = MAX_HINTS_PER_PLAYER;
                hintCol = -1;
            }
            else if (!showTreeOverlay && (action == 2 || IsKeyPressed(KEY_M))) {
                screen = ScreenState::Menu;
            }
        }
    }

    closeAudio();
    CloseWindow();
    return 0;
}
