#include "raylib.h"
#include "render.h"
#include "config.h"
#include <algorithm>

namespace {
    constexpr Color BG_COLOR        = { 15,  23,  42, 255 };
    constexpr Color BOARD_BODY      = { 30,  41,  59, 255 };
    constexpr Color BOARD_BORDER    = { 51,  65,  85, 255 };
    constexpr Color CARD_HOVER      = { 41,  55,  78, 255 };
    constexpr Color SLOT_EMPTY      = { 15,  23,  42, 255 };
    constexpr Color SLOT_SHADOW     = { 10,  15,  28, 255 };

    constexpr Color RED_MAIN        = { 239, 68,  68, 255 };
    constexpr Color RED_DARK        = { 185, 28,  28, 255 };
    constexpr Color RED_LIGHT       = { 252, 165, 165, 90 };

    constexpr Color YELLOW_MAIN     = { 245, 158, 11, 255 };
    constexpr Color YELLOW_DARK     = { 180, 83,  9,  255 };
    constexpr Color YELLOW_LIGHT    = { 253, 230, 138, 110 };

    constexpr Color BTN_PRIMARY     = { 37,  99,  235, 255 };
    constexpr Color BTN_PRIM_HOVER  = { 59,  130, 246, 255 };

    void drawToken(int x, int y, float radius, Color mainColor, Color rimColor, Color highlight) {
        DrawCircle(x, y + 4, radius, { 0, 0, 0, 75 });
        DrawCircle(x, y, radius, rimColor);
        DrawCircle(x, y, radius - 4.0f, mainColor);
        DrawCircle(x - radius * 0.25f, y - radius * 0.25f, radius * 0.35f, highlight);
    }

    bool isCurrentTurnAI(int turn, GameMode mode) {
        if (mode == GameMode::HumanVsHuman) return false;
        if (mode == GameMode::HumanVsAI) return turn == 1;
        return true;
    }
}

int drawMenu() {
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();
    Vector2 mouse = GetMousePosition();
    int selectedOption = -1;

    BeginDrawing();
    ClearBackground(BG_COLOR);

    DrawCircle(screenW / 2 - 340, screenH / 2 - 220, 180, { 239, 68, 68, 14 });
    DrawCircle(screenW / 2 + 340, screenH / 2 + 220, 200, { 245, 158, 11, 14 });

    float containerW = 560.0f;
    float containerH = 540.0f;
    float containerX = (screenW - containerW) / 2.0f;
    float containerY = (screenH - containerH) / 2.0f;

    DrawRectangleRounded({ containerX, containerY + 10, containerW, containerH }, 0.05f, 16, { 0, 0, 0, 120 });
    DrawRectangleRounded({ containerX, containerY, containerW, containerH }, 0.05f, 16, BOARD_BODY);
    DrawRectangleRoundedLinesEx({ containerX, containerY, containerW, containerH }, 0.05f, 16, 2.0f, BOARD_BORDER);

    int cx = screenW / 2;
    int headerY = (int)containerY + 42;

    drawToken(cx - 165, headerY + 22, 18.0f, RED_MAIN, RED_DARK, RED_LIGHT);
    drawToken(cx + 165, headerY + 22, 18.0f, YELLOW_MAIN, YELLOW_DARK, YELLOW_LIGHT);

    const char* title = "CONNECT 4";
    DrawText(title, cx - MeasureText(title, 44) / 2, headerY, 44, RAYWHITE);

    const char* subtitle = "Select a Game Mode to Start";
    DrawText(subtitle, cx - MeasureText(subtitle, 20) / 2, headerY + 56, 20, LIGHTGRAY);

    struct MenuOption {
        const char* title;
        const char* desc;
        const char* badge;
        Color accent;
    };

    MenuOption options[3] = {
        { "Human vs Human", "Local 2-player match on the same screen", "[1]", RED_MAIN },
        { "Human vs AI",    "Challenge the Minimax Alpha-Beta engine", "[2]", YELLOW_MAIN },
        { "AI vs AI",       "Watch two Minimax engines battle live",   "[3]", { 56, 189, 248, 255 } }
    };

    float btnW = containerW - 80.0f;
    float btnH = 92.0f;
    float btnX = containerX + 40.0f;
    float startY = containerY + 150.0f;
    float spacing = 110.0f;

    for (int i = 0; i < 3; i++) {
        Rectangle btnRect = { btnX, startY + i * spacing, btnW, btnH };
        bool hovered = CheckCollisionPointRec(mouse, btnRect);

        DrawRectangleRounded(btnRect, 0.16f, 12, hovered ? CARD_HOVER : SLOT_EMPTY);
        DrawRectangleRoundedLinesEx(btnRect, 0.16f, 12, hovered ? 2.5f : 1.5f, hovered ? options[i].accent : BOARD_BORDER);

        DrawRectangleRounded({ btnRect.x + 12, btnRect.y + 18, 6, btnH - 36 }, 0.5f, 6, options[i].accent);

        DrawText(options[i].title, (int)btnRect.x + 34, (int)btnRect.y + 18, 26, RAYWHITE);
        DrawText(options[i].desc,  (int)btnRect.x + 34, (int)btnRect.y + 52, 17, LIGHTGRAY);

        int badgeW = MeasureText(options[i].badge, 20);
        DrawText(options[i].badge, (int)(btnRect.x + btnW - badgeW - 24), (int)btnRect.y + 36, 20, hovered ? options[i].accent : GRAY);

        if (hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            selectedOption = i;
        }
    }

    const char* footer = "Press [1-3] to Select  |  Press [ESC] to Quit";
    DrawText(footer, cx - MeasureText(footer, 17) / 2, (int)(containerY + containerH - 36), 17, GRAY);

    EndDrawing();
    return selectedOption;
}

BoardLayout getBoardLayout() {
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();

    int panelW = 320;
    int gap = 60;
    int availW = screenW - panelW - gap * 3;
    int availH = screenH - 140;

    int cellSize = std::min(availW / COLS, availH / ROWS);
    int boardW = cellSize * COLS;
    int boardH = cellSize * ROWS;

    int totalGroupW = panelW + gap + boardW;
    int panelX = (screenW - totalGroupW) / 2;
    int boardX = panelX + panelW + gap;
    int boardY = (screenH - boardH) / 2 + 15;

    return { boardX, boardY, boardW, boardH, cellSize, panelX, panelW };
}

int drawGame(const GameState& g, GameMode mode, float humanTimer) {
    BoardLayout layout = getBoardLayout();
    bool aiTurn = isCurrentTurnAI(g.turn, mode);
    Vector2 mouse = GetMousePosition();
    int panelAction = 0; // 1 = Restart, 2 = Main Menu

    BeginDrawing();
    ClearBackground(BG_COLOR);

    // --- LEFT SIDE PANEL ---
    float cardH = (float)layout.boardH;
    Rectangle panelRect = { (float)layout.panelX, (float)layout.boardY, (float)layout.panelW, cardH };
    DrawRectangleRounded(panelRect, 0.05f, 16, BOARD_BODY);
    DrawRectangleRoundedLinesEx(panelRect, 0.05f, 16, 2.0f, BOARD_BORDER);

    int cx = layout.panelX + layout.panelW / 2;
    int topY = layout.boardY + 35;

    const char* gameTitle = "CONNECT 4";
    DrawText(gameTitle, cx - MeasureText(gameTitle, 36) / 2, topY, 36, RAYWHITE);

    const char* modeLabel = (mode == GameMode::HumanVsHuman) ? "HUMAN VS HUMAN" :
                            (mode == GameMode::HumanVsAI)    ? "HUMAN VS AI" : "AI VS AI";
    DrawText(modeLabel, cx - MeasureText(modeLabel, 15) / 2, topY + 42, 15, SKYBLUE);
    DrawLine(layout.panelX + 30, topY + 68, layout.panelX + layout.panelW - 30, topY + 68, BOARD_BORDER);

    // Status / Winner Card
    int statusY = topY + 95;
    if (!g.gameOver && !g.gameDraw) {
        Color activeColor = (g.turn == 0) ? RED_MAIN : YELLOW_MAIN;
        Color rimColor    = (g.turn == 0) ? RED_DARK : YELLOW_DARK;
        Color lightColor  = (g.turn == 0) ? RED_LIGHT : YELLOW_LIGHT;

        if (aiTurn) {
            int dots = ((int)(GetTime() * 3.0)) % 4;
            const char* thinkingLabels[4] = { "AI THINKING", "AI THINKING.", "AI THINKING..", "AI THINKING..." };
            const char* label = thinkingLabels[dots];
            DrawText(label, cx - MeasureText(label, 18) / 2, statusY, 18, activeColor);
        } else {
            const char* label = "CURRENT TURN";
            DrawText(label, cx - MeasureText(label, 18) / 2, statusY, 18, LIGHTGRAY);
        }

        drawToken(cx, statusY + 85, 38.0f, activeColor, rimColor, lightColor);

        const char* playerStr = "RED";
        if (mode == GameMode::HumanVsAI) {
            playerStr = (g.turn == 0) ? "RED (YOU)" : "YELLOW (AI)";
        } else if (mode == GameMode::AIVsAI) {
            playerStr = (g.turn == 0) ? "RED (AI 1)" : "YELLOW (AI 2)";
        } else {
            playerStr = (g.turn == 0) ? "RED (P1)" : "YELLOW (P2)";
        }
        DrawText(playerStr, cx - MeasureText(playerStr, 26) / 2, statusY + 140, 26, activeColor);
        if (!aiTurn) {
            float barW = (float)layout.panelW - 64.0f;
            float barH = 12.0f;
            float barX = (float)layout.panelX + 32.0f;
            float barY = (float)statusY + 195.0f;

            float ratio = std::max(0.0f, humanTimer / HUMAN_TURN_TIME);
            Color timerColor = (humanTimer <= 5.0f) ? RED_MAIN : SKYBLUE;

            // Numeric countdown label
            const char* timeText = TextFormat("TIME LEFT: %.1fs", std::max(0.0f, humanTimer));
            DrawText(timeText, cx - MeasureText(timeText, 18) / 2, (int)barY - 26, 18, timerColor);

            // Background track
            DrawRectangleRounded({ barX, barY, barW, barH }, 0.5f, 8, SLOT_EMPTY);
            // Shrinking fill bar
            if (ratio > 0.0f) {
                DrawRectangleRounded({ barX, barY, barW * ratio, barH }, 0.5f, 8, timerColor);
            }
        }
    } else {
        const char* resultTitle = "DRAW GAME!";
        Color resultColor = GOLD;

        if (g.gameOver) {
            resultTitle = (g.winner == 'X') ? "RED WINS!" : "YELLOW WINS!";
            resultColor = (g.winner == 'X') ? RED_MAIN : YELLOW_MAIN;
        }

        // Result Banner
        Rectangle resBox = { (float)layout.panelX + 24, (float)statusY, (float)layout.panelW - 48, 86.0f };
        DrawRectangleRounded(resBox, 0.16f, 12, SLOT_EMPTY);
        DrawRectangleRoundedLinesEx(resBox, 0.16f, 12, 2.0f, resultColor);
        DrawText(resultTitle, cx - MeasureText(resultTitle, 30) / 2, statusY + 28, 30, resultColor);

        // --- CLICKABLE ACTION BUTTONS ---
        float btnX = (float)layout.panelX + 24;
        float btnW = (float)layout.panelW - 48;
        float btnH = 52.0f;

        // 1. Play Again Button
        Rectangle restartBtn = { btnX, (float)statusY + 110.0f, btnW, btnH };
        bool hoverRestart = CheckCollisionPointRec(mouse, restartBtn);
        DrawRectangleRounded(restartBtn, 0.22f, 12, hoverRestart ? BTN_PRIM_HOVER : BTN_PRIMARY);
        const char* restartText = "Play Again  [R]";
        DrawText(restartText, cx - MeasureText(restartText, 20) / 2, (int)restartBtn.y + 16, 20, RAYWHITE);
        if (hoverRestart && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            panelAction = 1;
        }

        // 2. Main Menu Button
        Rectangle menuBtn = { btnX, (float)statusY + 176.0f, btnW, btnH };
        bool hoverMenu = CheckCollisionPointRec(mouse, menuBtn);
        DrawRectangleRounded(menuBtn, 0.22f, 12, hoverMenu ? CARD_HOVER : SLOT_EMPTY);
        DrawRectangleRoundedLinesEx(menuBtn, 0.22f, 12, 1.5f, hoverMenu ? RAYWHITE : BOARD_BORDER);
        const char* menuText = "Main Menu  [M]";
        DrawText(menuText, cx - MeasureText(menuText, 20) / 2, (int)menuBtn.y + 16, 20, RAYWHITE);
        if (hoverMenu && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            panelAction = 2;
        }
    }

    // Bottom Navigation Hints
    if (!g.gameOver && !g.gameDraw) {
        const char* menuHint = "Press [M] for Main Menu";
        DrawText(menuHint, cx - MeasureText(menuHint, 17) / 2, layout.boardY + layout.boardH - 65, 17, LIGHTGRAY);
    }
    const char* quitHint = "Press [ESC] to Exit";
    DrawText(quitHint, cx - MeasureText(quitHint, 17) / 2, layout.boardY + layout.boardH - 38, 17, GRAY);

    // --- RIGHT SIDE: BOARD CHASSIS ---
    Rectangle boardRect = {
        (float)layout.boardX - 10, (float)layout.boardY - 10,
        (float)layout.boardW + 20, (float)layout.boardH + 20
    };
    DrawRectangleRounded({ boardRect.x, boardRect.y + 8, boardRect.width, boardRect.height }, 0.04f, 16, { 0, 0, 0, 110 });
    DrawRectangleRounded(boardRect, 0.04f, 16, BOARD_BODY);
    DrawRectangleRoundedLinesEx(boardRect, 0.04f, 16, 2.0f, BOARD_BORDER);

    // Column Hover Preview
    if (!g.gameOver && !g.gameDraw && !aiTurn) {
        int mouseX = GetMouseX();
        int hoverCol = (mouseX - layout.boardX) / layout.cellSize;

        if (mouseX >= layout.boardX && hoverCol >= 0 && hoverCol < COLS && g.board[0][hoverCol] == '.') {
            int colX = layout.boardX + hoverCol * layout.cellSize;
            DrawRectangle(colX + 4, layout.boardY, layout.cellSize - 8, layout.boardH, { 255, 255, 255, 12 });

            int previewX = colX + layout.cellSize / 2;
            int previewY = layout.boardY - 26;
            Color ghostColor = (g.turn == 0) ? RED_MAIN : YELLOW_MAIN;
            ghostColor.a = 180;
            DrawCircle(previewX, previewY, layout.cellSize * 0.18f, ghostColor);
        }
    }

    // Board Grid & Tokens
    const float radius = layout.cellSize * 0.40f;
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            int x = layout.boardX + c * layout.cellSize + layout.cellSize / 2;
            int y = layout.boardY + r * layout.cellSize + layout.cellSize / 2;

            DrawCircle(x, y - 2, radius + 2, SLOT_SHADOW);
            DrawCircle(x, y, radius, SLOT_EMPTY);

            if (g.board[r][c] == 'X') {
                drawToken(x, y, radius, RED_MAIN, RED_DARK, RED_LIGHT);
            } else if (g.board[r][c] == 'O') {
                drawToken(x, y, radius, YELLOW_MAIN, YELLOW_DARK, YELLOW_LIGHT);
            }
        }
    }

    EndDrawing();
    return panelAction;
}