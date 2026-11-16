#include "raylib.h"
#include "render.h"
#include "config.h"
#include <algorithm>
#include <cmath>

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

    constexpr Color HINT_CYAN       = { 34,  211, 238, 255 };
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

    bool isWinningCell(const GameState& g, int r, int c) {
        if (!g.gameOver) return false;
        for (int i = 0; i < 4; i++) {
            if (g.winCells[i][0] == r && g.winCells[i][1] == c) {
                return true;
            }
        }
        return false;
    }
}

int drawMenu(AIDifficulty& difficulty) {
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();
    Vector2 mouse = GetMousePosition();
    int selectedOption = -1;

    BeginDrawing();
    ClearBackground(BG_COLOR);

    DrawCircle(screenW / 2 - 340, screenH / 2 - 240, 180, { 239, 68, 68, 14 });
    DrawCircle(screenW / 2 + 340, screenH / 2 + 240, 200, { 245, 158, 11, 14 });

    float containerW = 560.0f;
    float containerH = 620.0f;
    float containerX = (screenW - containerW) / 2.0f;
    float containerY = (screenH - containerH) / 2.0f;

    DrawRectangleRounded({ containerX, containerY + 10, containerW, containerH }, 0.05f, 16, { 0, 0, 0, 120 });
    DrawRectangleRounded({ containerX, containerY, containerW, containerH }, 0.05f, 16, BOARD_BODY);
    DrawRectangleRoundedLinesEx({ containerX, containerY, containerW, containerH }, 0.05f, 16, 2.0f, BOARD_BORDER);

    int cx = screenW / 2;
    int headerY = (int)containerY + 38;

    drawToken(cx - 165, headerY + 22, 18.0f, RED_MAIN, RED_DARK, RED_LIGHT);
    drawToken(cx + 165, headerY + 22, 18.0f, YELLOW_MAIN, YELLOW_DARK, YELLOW_LIGHT);

    const char* title = "CONNECT 4";
    DrawText(title, cx - MeasureText(title, 44) / 2, headerY, 44, RAYWHITE);

    const char* subtitle = "Select AI Difficulty & Game Mode";
    DrawText(subtitle, cx - MeasureText(subtitle, 19) / 2, headerY + 54, 19, LIGHTGRAY);

    // AI Difficulty Pill Selector
    float pillGroupW = containerW - 80.0f;
    float pillGroupH = 46.0f;
    float pillGroupX = containerX + 40.0f;
    float pillGroupY = containerY + 132.0f;

    DrawRectangleRounded({ pillGroupX, pillGroupY, pillGroupW, pillGroupH }, 0.25f, 12, SLOT_EMPTY);
    DrawRectangleRoundedLinesEx({ pillGroupX, pillGroupY, pillGroupW, pillGroupH }, 0.25f, 12, 1.5f, BOARD_BORDER);

    struct DiffItem {
        const char* label;
        AIDifficulty level;
        Color activeColor;
    };

    DiffItem diffs[3] = {
        { "Easy",   AIDifficulty::Easy,   { 34,  197, 94,  255 } },
        { "Medium", AIDifficulty::Medium, { 245, 158, 11,  255 } },
        { "Hard",   AIDifficulty::Hard,   { 239, 68,  68,  255 } }
    };

    float segW = (pillGroupW - 12.0f) / 3.0f;
    for (int i = 0; i < 3; i++) {
        Rectangle segRect = { pillGroupX + 6.0f + i * segW, pillGroupY + 5.0f, segW, pillGroupH - 10.0f };
        bool active = (difficulty == diffs[i].level);
        bool hovered = CheckCollisionPointRec(mouse, segRect);

        if (active) {
            DrawRectangleRounded(segRect, 0.25f, 10, diffs[i].activeColor);
        } else if (hovered) {
            DrawRectangleRounded(segRect, 0.25f, 10, CARD_HOVER);
        }

        Color txtColor = active ? BG_COLOR : (hovered ? RAYWHITE : LIGHTGRAY);
        int txtW = MeasureText(diffs[i].label, 19);
        DrawText(diffs[i].label, (int)(segRect.x + (segW - txtW) / 2.0f), (int)segRect.y + 8, 19, txtColor);

        if (hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            difficulty = diffs[i].level;
        }
    }

    // Three Game Mode Buttons
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
    float btnH = 90.0f;
    float btnX = containerX + 40.0f;
    float startY = containerY + 202.0f;
    float spacing = 108.0f;

    for (int i = 0; i < 3; i++) {
        Rectangle btnRect = { btnX, startY + i * spacing, btnW, btnH };
        bool hovered = CheckCollisionPointRec(mouse, btnRect);

        DrawRectangleRounded(btnRect, 0.16f, 12, hovered ? CARD_HOVER : SLOT_EMPTY);
        DrawRectangleRoundedLinesEx(btnRect, 0.16f, 12, hovered ? 2.5f : 1.5f, hovered ? options[i].accent : BOARD_BORDER);

        DrawRectangleRounded({ btnRect.x + 12, btnRect.y + 18, 6, btnH - 36 }, 0.5f, 6, options[i].accent);

        DrawText(options[i].title, (int)btnRect.x + 34, (int)btnRect.y + 18, 26, RAYWHITE);
        DrawText(options[i].desc,  (int)btnRect.x + 34, (int)btnRect.y + 52, 17, LIGHTGRAY);

        int badgeW = MeasureText(options[i].badge, 20);
        DrawText(options[i].badge, (int)(btnRect.x + btnW - badgeW - 24), (int)btnRect.y + 35, 20, hovered ? options[i].accent : GRAY);

        if (hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            selectedOption = i;
        }
    }

    const char* footer = "Press [1-3] to Start  |  Press [ESC] to Quit";
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
    int availH = screenH - 160;

    int cellSize = std::min(availW / COLS, availH / ROWS);
    int boardW = cellSize * COLS;
    int boardH = cellSize * ROWS;

    int totalGroupW = panelW + gap + boardW;
    int panelX = (screenW - totalGroupW) / 2;
    int boardX = panelX + panelW + gap;
    int boardY = (screenH - boardH) / 2 + 22;

    return { boardX, boardY, boardW, boardH, cellSize, panelX, panelW };
}

int drawGame(const GameState& g, GameMode mode, AIDifficulty difficulty, float humanTimer,
             const SessionScore& score, int selectedCol, int hintsLeft, int hintCol) {
    BoardLayout layout = getBoardLayout();
    bool aiTurn = isCurrentTurnAI(g.turn, mode);
    Vector2 mouse = GetMousePosition();
    int panelAction = 0; // 1 = Play Again, 2 = Main Menu, 3 = Hint
    float pulse = 0.5f + 0.5f * std::sin((float)GetTime() * 6.0f);

    BeginDrawing();
    ClearBackground(BG_COLOR);

    // --- LEFT SIDE PANEL ---
    float cardH = (float)layout.boardH;
    Rectangle panelRect = { (float)layout.panelX, (float)layout.boardY, (float)layout.panelW, cardH };
    DrawRectangleRounded(panelRect, 0.05f, 16, BOARD_BODY);
    DrawRectangleRoundedLinesEx(panelRect, 0.05f, 16, 2.0f, BOARD_BORDER);

    int cx = layout.panelX + layout.panelW / 2;
    int topY = layout.boardY + 26;

    const char* gameTitle = "CONNECT 4";
    DrawText(gameTitle, cx - MeasureText(gameTitle, 34) / 2, topY, 34, RAYWHITE);

    const char* diffStr = (difficulty == AIDifficulty::Easy)   ? "EASY" :
                          (difficulty == AIDifficulty::Medium) ? "MEDIUM" : "HARD";
    const char* modeLabel = (mode == GameMode::HumanVsHuman) ? "HUMAN VS HUMAN" :
                            (mode == GameMode::HumanVsAI)    ? TextFormat("HUMAN VS AI (%s)", diffStr) :
                                                               TextFormat("AI VS AI (%s)", diffStr);

    DrawText(modeLabel, cx - MeasureText(modeLabel, 15) / 2, topY + 38, 15, SKYBLUE);
    DrawLine(layout.panelX + 28, topY + 60, layout.panelX + layout.panelW - 28, topY + 60, BOARD_BORDER);

    // --- STATUS / WINNER SECTION ---
    int statusY = topY + 74;
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

        drawToken(cx, statusY + 64, 32.0f, activeColor, rimColor, lightColor);

        const char* playerStr = "RED";
        if (mode == GameMode::HumanVsAI) {
            playerStr = (g.turn == 0) ? "RED (YOU)" : "YELLOW (AI)";
        } else if (mode == GameMode::AIVsAI) {
            playerStr = (g.turn == 0) ? "RED (AI 1)" : "YELLOW (AI 2)";
        } else {
            playerStr = (g.turn == 0) ? "RED (P1)" : "YELLOW (P2)";
        }
        DrawText(playerStr, cx - MeasureText(playerStr, 23) / 2, statusY + 108, 23, activeColor);

        if (!aiTurn) {
            // 15s Turn Countdown Bar
            float barW = (float)layout.panelW - 56.0f;
            float barH = 10.0f;
            float barX = (float)layout.panelX + 28.0f;
            float barY = (float)statusY + 166.0f;

            float ratio = std::max(0.0f, humanTimer / HUMAN_TURN_TIME);
            Color timerColor = (humanTimer <= 5.0f) ? RED_MAIN : SKYBLUE;

            const char* timeText = TextFormat("TIME LEFT: %.1fs", std::max(0.0f, humanTimer));
            DrawText(timeText, cx - MeasureText(timeText, 16) / 2, (int)barY - 22, 16, timerColor);

            DrawRectangleRounded({ barX, barY, barW, barH }, 0.5f, 8, SLOT_EMPTY);
            if (ratio > 0.0f) {
                DrawRectangleRounded({ barX, barY, barW * ratio, barH }, 0.5f, 8, timerColor);
            }

            // Hint Button
            float hintW = (float)layout.panelW - 56.0f;
            float hintH = 38.0f;
            float hintX = (float)layout.panelX + 28.0f;
            float hintY = barY + 22.0f;
            Rectangle hintBtn = { hintX, hintY, hintW, hintH };

            bool canUseHint = (hintsLeft > 0 && hintCol == -1);
            bool hoverHint = canUseHint && CheckCollisionPointRec(mouse, hintBtn);

            Color btnFill   = canUseHint ? (hoverHint ? CARD_HOVER : SLOT_EMPTY) : BG_COLOR;
            Color btnBorder = (hintCol != -1) ? HINT_CYAN : (canUseHint ? (hoverHint ? HINT_CYAN : BOARD_BORDER) : BOARD_BORDER);
            Color btnText   = (hintCol != -1) ? HINT_CYAN : (canUseHint ? RAYWHITE : GRAY);

            DrawRectangleRounded(hintBtn, 0.24f, 10, btnFill);
            DrawRectangleRoundedLinesEx(hintBtn, 0.24f, 10, 1.5f, btnBorder);

            const char* hintLabel = (hintCol != -1)
                ? TextFormat("Suggested: Col %d", hintCol + 1)
                : TextFormat("Hint [H] (%d left)", hintsLeft);
            DrawText(hintLabel, cx - MeasureText(hintLabel, 17) / 2, (int)hintY + 10, 17, btnText);

            if (hoverHint && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                panelAction = 3;
            }
        }
    } else {
        const char* resultTitle = "DRAW GAME!";
        Color resultColor = GOLD;

        if (g.gameOver) {
            resultTitle = (g.winner == 'X') ? "RED WINS!" : "YELLOW WINS!";
            resultColor = (g.winner == 'X') ? RED_MAIN : YELLOW_MAIN;
        }

        Rectangle resBox = { (float)layout.panelX + 24, (float)statusY, (float)layout.panelW - 48, 72.0f };
        DrawRectangleRounded(resBox, 0.16f, 12, SLOT_EMPTY);
        DrawRectangleRoundedLinesEx(resBox, 0.16f, 12, 2.0f, resultColor);
        DrawText(resultTitle, cx - MeasureText(resultTitle, 28) / 2, statusY + 22, 28, resultColor);

        float btnX = (float)layout.panelX + 24;
        float btnW = (float)layout.panelW - 48;
        float btnH = 44.0f;

        Rectangle restartBtn = { btnX, (float)statusY + 86.0f, btnW, btnH };
        bool hoverRestart = CheckCollisionPointRec(mouse, restartBtn);
        DrawRectangleRounded(restartBtn, 0.22f, 12, hoverRestart ? BTN_PRIM_HOVER : BTN_PRIMARY);
        const char* restartText = "Play Again  [R]";
        DrawText(restartText, cx - MeasureText(restartText, 19) / 2, (int)restartBtn.y + 13, 19, RAYWHITE);
        if (hoverRestart && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            panelAction = 1;
        }

        Rectangle menuBtn = { btnX, (float)statusY + 140.0f, btnW, btnH };
        bool hoverMenu = CheckCollisionPointRec(mouse, menuBtn);
        DrawRectangleRounded(menuBtn, 0.22f, 12, hoverMenu ? CARD_HOVER : SLOT_EMPTY);
        DrawRectangleRoundedLinesEx(menuBtn, 0.22f, 12, 1.5f, hoverMenu ? RAYWHITE : BOARD_BORDER);
        const char* menuText = "Main Menu  [M]";
        DrawText(menuText, cx - MeasureText(menuText, 19) / 2, (int)menuBtn.y + 13, 19, RAYWHITE);
        if (hoverMenu && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            panelAction = 2;
        }
    }

    // --- SESSION SCOREBOARD CARD ---
    float sbW = (float)layout.panelW - 48.0f;
    float sbH = 74.0f;
    float sbX = (float)layout.panelX + 24.0f;
    float sbY = (float)(layout.boardY + layout.boardH) - 135.0f;

    DrawRectangleRounded({ sbX, sbY, sbW, sbH }, 0.18f, 12, SLOT_EMPTY);
    DrawRectangleRoundedLinesEx({ sbX, sbY, sbW, sbH }, 0.18f, 12, 1.5f, BOARD_BORDER);

    float colW = sbW / 3.0f;
    const char* sbHeaders[3] = { "RED", "DRAW", "YELLOW" };
    int sbValues[3]          = { score.redWins, score.draws, score.yellowWins };
    Color sbColors[3]        = { RED_MAIN, LIGHTGRAY, YELLOW_MAIN };

    for (int i = 0; i < 3; i++) {
        int subCx = (int)(sbX + colW * i + colW / 2.0f);
        DrawText(sbHeaders[i], subCx - MeasureText(sbHeaders[i], 13) / 2, (int)sbY + 12, 13, sbColors[i]);
        const char* valStr = TextFormat("%d", sbValues[i]);
        DrawText(valStr, subCx - MeasureText(valStr, 26) / 2, (int)sbY + 34, 26, RAYWHITE);

        if (i < 2) {
            int divX = (int)(sbX + colW * (i + 1));
            DrawLine(divX, (int)sbY + 14, divX, (int)(sbY + sbH - 14), BOARD_BORDER);
        }
    }

    // Bottom Navigation Hints
    const char* navHint = "[1-7 / Arrows] Drop  |  [M] Menu";
    DrawText(navHint, cx - MeasureText(navHint, 15) / 2, layout.boardY + layout.boardH - 42, 15, GRAY);

    // --- RIGHT SIDE: BOARD CHASSIS ---
    Rectangle boardRect = {
        (float)layout.boardX - 10, (float)layout.boardY - 10,
        (float)layout.boardW + 20, (float)layout.boardH + 20
    };
    DrawRectangleRounded({ boardRect.x, boardRect.y + 8, boardRect.width, boardRect.height }, 0.04f, 16, { 0, 0, 0, 110 });
    DrawRectangleRounded(boardRect, 0.04f, 16, BOARD_BODY);
    DrawRectangleRoundedLinesEx(boardRect, 0.04f, 16, 2.0f, BOARD_BORDER);

    // Column Numbers (1..7), Hint Overlay & Active Column Selector Preview
    for (int c = 0; c < COLS; c++) {
        int colX = layout.boardX + c * layout.cellSize;
        int colCenterX = colX + layout.cellSize / 2;
        bool isSelected = (!g.gameOver && !g.gameDraw && !aiTurn && c == selectedCol);
        bool isHinted   = (!g.gameOver && !g.gameDraw && !aiTurn && c == hintCol);

        const char* colNum = TextFormat("%d", c + 1);
        Color numColor = isHinted ? HINT_CYAN : (isSelected ? RAYWHITE : GRAY);
        DrawText(colNum, colCenterX - MeasureText(colNum, 16) / 2, layout.boardY - 52, 16, numColor);

        // Pulsing Cyan Column Overlay when Hinted
        if (isHinted && g.board[0][c] == '.') {
            Rectangle hintColRect = { (float)colX + 3, (float)layout.boardY + 2, (float)layout.cellSize - 6, (float)layout.boardH - 4 };
            unsigned char fillAlpha = (unsigned char)(18 + 20 * pulse);
            unsigned char lineAlpha = (unsigned char)(160 + 95 * pulse);
            DrawRectangleRounded(hintColRect, 0.12f, 8, { 34, 211, 238, fillAlpha });
            DrawRectangleRoundedLinesEx(hintColRect, 0.12f, 8, 2.0f, { 34, 211, 238, lineAlpha });

            const char* bestTag = "BEST";
            DrawText(bestTag, colCenterX - MeasureText(bestTag, 13) / 2, layout.boardY - 32, 13, HINT_CYAN);
        }

        if (isSelected && g.board[0][c] == '.') {
            DrawRectangle(colX + 4, layout.boardY, layout.cellSize - 8, layout.boardH, { 255, 255, 255, 14 });

            if (!isHinted) {
                int previewY = layout.boardY - 24;
                Color ghostColor = (g.turn == 0) ? RED_MAIN : YELLOW_MAIN;
                ghostColor.a = 190;
                DrawCircle(colCenterX, previewY, layout.cellSize * 0.16f, ghostColor);
            }
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

            if (isWinningCell(g, r, c)) {
                unsigned char alpha = (unsigned char)(170 + 85 * pulse);
                Color ringColor = { 255, 255, 255, alpha };
                DrawRing({ (float)x, (float)y }, radius + 1.0f, radius + 6.0f + pulse * 2.0f, 0.0f, 360.0f, 36, ringColor);
            }
        }
    }

    if (g.gameOver && g.winCells[0][0] != -1) {
        int x1 = layout.boardX + g.winCells[0][1] * layout.cellSize + layout.cellSize / 2;
        int y1 = layout.boardY + g.winCells[0][0] * layout.cellSize + layout.cellSize / 2;
        int x2 = layout.boardX + g.winCells[3][1] * layout.cellSize + layout.cellSize / 2;
        int y2 = layout.boardY + g.winCells[3][0] * layout.cellSize + layout.cellSize / 2;

        DrawLineEx({ (float)x1, (float)y1 }, { (float)x2, (float)y2 }, 6.0f, { 255, 255, 255, 210 });
    }

    EndDrawing();
    return panelAction;
}