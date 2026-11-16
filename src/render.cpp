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
    constexpr Color GREEN_CHOSEN    = { 74,  222, 128, 255 };
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

    bool drawTreeModal(const SearchStats& stats, float& scrollY) {
        int screenW = GetScreenWidth();
        int screenH = GetScreenHeight();
        Vector2 mouse = GetMousePosition();
        bool closeRequested = false;

        DrawRectangle(0, 0, screenW, screenH, { 5, 10, 20, 215 });

        float modalW = std::min(960.0f, (float)screenW - 100.0f);
        float modalH = std::min(740.0f, (float)screenH - 90.0f);
        float modalX = (screenW - modalW) / 2.0f;
        float modalY = (screenH - modalH) / 2.0f;

        DrawRectangleRounded({ modalX, modalY, modalW, modalH }, 0.03f, 16, BOARD_BODY);
        DrawRectangleRoundedLinesEx({ modalX, modalY, modalW, modalH }, 0.03f, 16, 2.0f, HINT_CYAN);

        DrawText("AI DECISION TREE & HEURISTIC TRACE", (int)modalX + 28, (int)modalY + 22, 24, RAYWHITE);
        const char* sub = stats.treeLines.empty()
            ? "No AI move recorded yet. Let the AI play a turn first!"
            : stats.algoSummary.c_str();
        DrawText(sub, (int)modalX + 28, (int)modalY + 54, 16, SKYBLUE);

        Rectangle closeBtn = { modalX + modalW - 125.0f, modalY + 20.0f, 100.0f, 38.0f };
        bool hoverClose = CheckCollisionPointRec(mouse, closeBtn);
        DrawRectangleRounded(closeBtn, 0.25f, 10, hoverClose ? RED_MAIN : SLOT_EMPTY);
        DrawRectangleRoundedLinesEx(closeBtn, 0.25f, 10, 1.5f, hoverClose ? RED_MAIN : BOARD_BORDER);
        DrawText("Close [T]", (int)closeBtn.x + 15, (int)closeBtn.y + 10, 17, RAYWHITE);
        if (hoverClose && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            closeRequested = true;
        }

        int legY = (int)modalY + 84;
        DrawCircle((int)modalX + 34, legY + 7, 5, GREEN_CHOSEN);
        DrawText("Chosen Move", (int)modalX + 46, legY, 14, GREEN_CHOSEN);

        DrawCircle((int)modalX + 175, legY + 7, 5, RED_MAIN);
        DrawText("Alpha-Beta Pruned", (int)modalX + 187, legY, 14, RED_MAIN);

        DrawCircle((int)modalX + 355, legY + 7, 5, YELLOW_MAIN);
        DrawText("Heuristic Leaf / Branch", (int)modalX + 367, legY, 14, LIGHTGRAY);

        int viewX = (int)modalX + 24;
        int viewY = (int)modalY + 112;
        int viewW = (int)modalW - 48;
        int viewH = (int)modalH - 154;

        DrawRectangleRounded({ (float)viewX, (float)viewY, (float)viewW, (float)viewH }, 0.02f, 12, SLOT_EMPTY);
        DrawRectangleRoundedLinesEx({ (float)viewX, (float)viewY, (float)viewW, (float)viewH }, 0.02f, 12, 1.5f, BOARD_BORDER);

        const float rowH = 26.0f;
        float contentH = stats.treeLines.size() * rowH + 24.0f;
        float maxScroll = std::max(0.0f, contentH - (float)viewH);

        float wheel = GetMouseWheelMove();
        if (wheel != 0.0f) scrollY -= wheel * 48.0f;
        if (IsKeyDown(KEY_DOWN)) scrollY += 8.0f;
        if (IsKeyDown(KEY_UP))   scrollY -= 8.0f;
        scrollY = std::clamp(scrollY, 0.0f, maxScroll);

        BeginScissorMode(viewX + 4, viewY + 4, viewW - 8, viewH - 8);
        for (size_t i = 0; i < stats.treeLines.size(); i++) {
            float y = viewY + 12.0f + i * rowH - scrollY;
            if (y + rowH < viewY || y > viewY + viewH) continue;

            const auto& line = stats.treeLines[i];
            int indentX = viewX + 18 + line.level * 32;

            if (line.level > 0) {
                int parentX = indentX - 18;
                DrawLine(parentX, (int)y - 4, parentX, (int)y + 11, BOARD_BORDER);
                DrawLine(parentX, (int)y + 11, indentX - 4, (int)y + 11, BOARD_BORDER);
            }

            if (line.isChosen) {
                DrawRectangleRounded({ (float)indentX - 6, y, (float)(viewW - (indentX - viewX) - 18), rowH - 2 },
                                     0.2f, 6, { 74, 222, 128, 28 });
            }

            Color textColor = RAYWHITE;
            if (line.level == 0)       textColor = HINT_CYAN;
            else if (line.isChosen)    textColor = GREEN_CHOSEN;
            else if (line.isPruned)    textColor = RED_MAIN;
            else if (line.level >= 3)  textColor = YELLOW_MAIN;

            DrawText(line.text.c_str(), indentX, (int)y + 4, 16, textColor);
        }
        EndScissorMode();

        if (maxScroll > 0.0f) {
            float barTrackH = (float)viewH - 16.0f;
            float thumbH = std::max(30.0f, barTrackH * ((float)viewH / contentH));
            float thumbY = viewY + 8.0f + (scrollY / maxScroll) * (barTrackH - thumbH);
            DrawRectangleRounded({ (float)(viewX + viewW - 10), thumbY, 5.0f, thumbH }, 0.5f, 4, BOARD_BORDER);
        }

        const char* footer = "Scroll with Mouse Wheel or [Up/Down]  |  Press [T] to Close";
        DrawText(footer, (int)(modalX + modalW / 2.0f) - MeasureText(footer, 14) / 2, (int)(modalY + modalH - 28), 14, GRAY);

        return closeRequested;
    }
}

int drawMenu(AIAlgorithm& algorithm, AIDifficulty& difficulty, int& customK) {
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();
    Vector2 mouse = GetMousePosition();
    int selectedOption = -1;

    if (difficulty == AIDifficulty::Custom) {
        if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_EQUAL))   customK = std::min(10, customK + 1);
        if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_MINUS)) customK = std::max(1, customK - 1);
    }

    bool isCustom = (difficulty == AIDifficulty::Custom);
    float containerW = 600.0f;
    float containerH = isCustom ? 720.0f : 670.0f;
    float containerX = (screenW - containerW) / 2.0f;
    float containerY = (screenH - containerH) / 2.0f;

    BeginDrawing();
    ClearBackground(BG_COLOR);

    DrawCircle(screenW / 2 - 360, screenH / 2 - 260, 190, { 239, 68, 68, 14 });
    DrawCircle(screenW / 2 + 360, screenH / 2 + 260, 210, { 245, 158, 11, 14 });

    DrawRectangleRounded({ containerX, containerY + 10, containerW, containerH }, 0.04f, 16, { 0, 0, 0, 120 });
    DrawRectangleRounded({ containerX, containerY, containerW, containerH }, 0.04f, 16, BOARD_BODY);
    DrawRectangleRoundedLinesEx({ containerX, containerY, containerW, containerH }, 0.04f, 16, 2.0f, BOARD_BORDER);

    int cx = screenW / 2;
    int headerY = (int)containerY + 30;

    drawToken(cx - 165, headerY + 20, 17.0f, RED_MAIN, RED_DARK, RED_LIGHT);
    drawToken(cx + 165, headerY + 20, 17.0f, YELLOW_MAIN, YELLOW_DARK, YELLOW_LIGHT);

    const char* title = "CONNECT 4";
    DrawText(title, cx - MeasureText(title, 42) / 2, headerY, 42, RAYWHITE);

    const char* subtitle = "Configure AI Algorithm, Depth K & Game Mode";
    DrawText(subtitle, cx - MeasureText(subtitle, 18) / 2, headerY + 50, 18, LIGHTGRAY);

    // 1. AI Algorithm Selector (Alpha-Beta vs Plain Minimax)
    float pillW = containerW - 80.0f;
    float pillH = 44.0f;
    float pillX = containerX + 40.0f;
    float algoY = containerY + 118.0f;

    DrawRectangleRounded({ pillX, algoY, pillW, pillH }, 0.25f, 12, SLOT_EMPTY);
    DrawRectangleRoundedLinesEx({ pillX, algoY, pillW, pillH }, 0.25f, 12, 1.5f, BOARD_BORDER);

    struct AlgoItem {
        const char* label;
        AIAlgorithm type;
        Color activeColor;
    };

    AlgoItem algos[2] = {
        { "Alpha-Beta Pruning", AIAlgorithm::AlphaBeta,    { 56,  189, 248, 255 } },
        { "Plain Minimax",      AIAlgorithm::PlainMinimax, { 245, 158, 11,  255 } }
    };

    float algoSegW = (pillW - 12.0f) / 2.0f;
    for (int i = 0; i < 2; i++) {
        Rectangle segRect = { pillX + 6.0f + i * algoSegW, algoY + 5.0f, algoSegW, pillH - 10.0f };
        bool active = (algorithm == algos[i].type);
        bool hovered = CheckCollisionPointRec(mouse, segRect);

        if (active) {
            DrawRectangleRounded(segRect, 0.25f, 10, algos[i].activeColor);
        } else if (hovered) {
            DrawRectangleRounded(segRect, 0.25f, 10, CARD_HOVER);
        }

        Color txtColor = active ? BG_COLOR : (hovered ? RAYWHITE : LIGHTGRAY);
        int txtW = MeasureText(algos[i].label, 18);
        DrawText(algos[i].label, (int)(segRect.x + (algoSegW - txtW) / 2.0f), (int)segRect.y + 8, 18, txtColor);

        if (hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            algorithm = algos[i].type;
        }
    }

    // 2. 4-Option Hardness / Depth K Selector
    float diffY = algoY + 56.0f;
    DrawRectangleRounded({ pillX, diffY, pillW, pillH }, 0.25f, 12, SLOT_EMPTY);
    DrawRectangleRoundedLinesEx({ pillX, diffY, pillW, pillH }, 0.25f, 12, 1.5f, BOARD_BORDER);

    struct DiffItem {
        const char* label;
        AIDifficulty level;
        Color activeColor;
    };

    DiffItem diffs[4] = {
        { "Easy (2)",   AIDifficulty::Easy,   { 34,  197, 94,  255 } },
        { "Medium (4)", AIDifficulty::Medium, { 245, 158, 11,  255 } },
        { "Hard (6)",   AIDifficulty::Hard,   { 239, 68,  68,  255 } },
        { "Custom K",   AIDifficulty::Custom, HINT_CYAN }
    };

    float diffSegW = (pillW - 12.0f) / 4.0f;
    for (int i = 0; i < 4; i++) {
        Rectangle segRect = { pillX + 6.0f + i * diffSegW, diffY + 5.0f, diffSegW, pillH - 10.0f };
        bool active = (difficulty == diffs[i].level);
        bool hovered = CheckCollisionPointRec(mouse, segRect);

        if (active) {
            DrawRectangleRounded(segRect, 0.25f, 10, diffs[i].activeColor);
        } else if (hovered) {
            DrawRectangleRounded(segRect, 0.25f, 10, CARD_HOVER);
        }

        Color txtColor = active ? BG_COLOR : (hovered ? RAYWHITE : LIGHTGRAY);
        int txtW = MeasureText(diffs[i].label, 16);
        DrawText(diffs[i].label, (int)(segRect.x + (diffSegW - txtW) / 2.0f), (int)segRect.y + 9, 16, txtColor);

        if (hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            difficulty = diffs[i].level;
        }
    }

    float nextY = diffY + 60.0f;
    if (isCustom) {
        Rectangle stepperBox = { pillX, nextY, pillW, 44.0f };
        DrawRectangleRounded(stepperBox, 0.22f, 10, SLOT_EMPTY);
        DrawRectangleRoundedLinesEx(stepperBox, 0.22f, 10, 1.5f, HINT_CYAN);

        const char* kDesc = "Tree Cutoff Depth (K):";
        DrawText(kDesc, (int)pillX + 18, (int)nextY + 13, 18, LIGHTGRAY);

        Rectangle minusBtn = { pillX + pillW - 136.0f, nextY + 6.0f, 36.0f, 32.0f };
        Rectangle plusBtn  = { pillX + pillW - 48.0f,  nextY + 6.0f, 36.0f, 32.0f };
        bool hoverMinus = CheckCollisionPointRec(mouse, minusBtn);
        bool hoverPlus  = CheckCollisionPointRec(mouse, plusBtn);

        DrawRectangleRounded(minusBtn, 0.25f, 8, hoverMinus ? CARD_HOVER : BOARD_BODY);
        DrawText("-", (int)minusBtn.x + 13, (int)minusBtn.y + 5, 22, RAYWHITE);
        if (hoverMinus && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) customK = std::max(1, customK - 1);

        const char* kValStr = TextFormat("%d", customK);
        DrawText(kValStr, (int)(pillX + pillW - 92.0f) + 18 - MeasureText(kValStr, 22) / 2, (int)nextY + 11, 22, HINT_CYAN);

        DrawRectangleRounded(plusBtn, 0.25f, 8, hoverPlus ? CARD_HOVER : BOARD_BODY);
        DrawText("+", (int)plusBtn.x + 12, (int)plusBtn.y + 5, 22, RAYWHITE);
        if (hoverPlus && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) customK = std::min(10, customK + 1);

        nextY += 56.0f;
    }

    // 3. Three Game Mode Cards
    struct MenuOption {
        const char* title;
        const char* desc;
        const char* badge;
        Color accent;
    };

    MenuOption options[3] = {
        { "Human vs Human", "Local 2-player match on the same screen", "[1]", RED_MAIN },
        { "Human vs AI",    "Challenge the selected AI algorithm",     "[2]", YELLOW_MAIN },
        { "AI vs AI",       "Watch two AI agents battle live",         "[3]", { 56, 189, 248, 255 } }
    };

    float btnW = containerW - 80.0f;
    float btnH = 88.0f;
    float btnX = containerX + 40.0f;
    float spacing = 104.0f;

    for (int i = 0; i < 3; i++) {
        Rectangle btnRect = { btnX, nextY + i * spacing, btnW, btnH };
        bool hovered = CheckCollisionPointRec(mouse, btnRect);

        DrawRectangleRounded(btnRect, 0.16f, 12, hovered ? CARD_HOVER : SLOT_EMPTY);
        DrawRectangleRoundedLinesEx(btnRect, 0.16f, 12, hovered ? 2.5f : 1.5f, hovered ? options[i].accent : BOARD_BORDER);

        DrawRectangleRounded({ btnRect.x + 12, btnRect.y + 18, 6, btnH - 36 }, 0.5f, 6, options[i].accent);

        DrawText(options[i].title, (int)btnRect.x + 34, (int)btnRect.y + 17, 25, RAYWHITE);
        DrawText(options[i].desc,  (int)btnRect.x + 34, (int)btnRect.y + 50, 17, LIGHTGRAY);

        int badgeW = MeasureText(options[i].badge, 20);
        DrawText(options[i].badge, (int)(btnRect.x + btnW - badgeW - 24), (int)btnRect.y + 34, 20, hovered ? options[i].accent : GRAY);

        if (hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            selectedOption = i;
        }
    }

    const char* footer = "Press [1-3] to Start  |  Press [ESC] to Quit";
    DrawText(footer, cx - MeasureText(footer, 16) / 2, (int)(containerY + containerH - 32), 16, GRAY);

    EndDrawing();
    return selectedOption;
}

BoardLayout getBoardLayout() {
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();

    int panelW = 330;
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

int drawGame(const GameState& g, GameMode mode, AIAlgorithm algorithm, int activeK,
             float humanTimer, const SessionScore& score, int selectedCol,
             int hintsLeft, int hintCol, const SearchStats& lastStats,
             bool showTreeOverlay, float& treeScrollY) {
    BoardLayout layout = getBoardLayout();
    bool aiTurn = isCurrentTurnAI(g.turn, mode);
    Vector2 mouse = GetMousePosition();
    int panelAction = 0;
    float pulse = 0.5f + 0.5f * std::sin((float)GetTime() * 6.0f);

    BeginDrawing();
    ClearBackground(BG_COLOR);

    // --- LEFT SIDE PANEL ---
    float cardH = (float)layout.boardH;
    Rectangle panelRect = { (float)layout.panelX, (float)layout.boardY, (float)layout.panelW, cardH };
    DrawRectangleRounded(panelRect, 0.05f, 16, BOARD_BODY);
    DrawRectangleRoundedLinesEx(panelRect, 0.05f, 16, 2.0f, BOARD_BORDER);

    int cx = layout.panelX + layout.panelW / 2;
    int topY = layout.boardY + 22;

    const char* gameTitle = "CONNECT 4";
    DrawText(gameTitle, cx - MeasureText(gameTitle, 32) / 2, topY, 32, RAYWHITE);

    const char* algoShort = (algorithm == AIAlgorithm::AlphaBeta) ? "ALPHA-BETA" : "MINIMAX";
    const char* modeLabel = (mode == GameMode::HumanVsHuman)
                            ? "HUMAN VS HUMAN"
                            : TextFormat("%s (K=%d)", algoShort, activeK);

    DrawText(modeLabel, cx - MeasureText(modeLabel, 14) / 2, topY + 36, 14, SKYBLUE);
    DrawLine(layout.panelX + 26, topY + 56, layout.panelX + layout.panelW - 26, topY + 56, BOARD_BORDER);

    // --- STATUS / WINNER SECTION ---
    int statusY = topY + 68;
    if (!g.gameOver && !g.gameDraw) {
        Color activeColor = (g.turn == 0) ? RED_MAIN : YELLOW_MAIN;
        Color rimColor    = (g.turn == 0) ? RED_DARK : YELLOW_DARK;
        Color lightColor  = (g.turn == 0) ? RED_LIGHT : YELLOW_LIGHT;

        if (aiTurn) {
            int dots = ((int)(GetTime() * 3.0)) % 4;
            const char* thinkingLabels[4] = { "AI THINKING", "AI THINKING.", "AI THINKING..", "AI THINKING..." };
            const char* label = thinkingLabels[dots];
            DrawText(label, cx - MeasureText(label, 17) / 2, statusY, 17, activeColor);
        } else {
            const char* label = "CURRENT TURN";
            DrawText(label, cx - MeasureText(label, 17) / 2, statusY, 17, LIGHTGRAY);
        }

        drawToken(cx, statusY + 56, 28.0f, activeColor, rimColor, lightColor);

        const char* playerStr = "RED";
        if (mode == GameMode::HumanVsAI) {
            playerStr = (g.turn == 0) ? "RED (YOU)" : "YELLOW (AI)";
        } else if (mode == GameMode::AIVsAI) {
            playerStr = (g.turn == 0) ? "RED (AI 1)" : "YELLOW (AI 2)";
        } else {
            playerStr = (g.turn == 0) ? "RED (P1)" : "YELLOW (P2)";
        }
        DrawText(playerStr, cx - MeasureText(playerStr, 22) / 2, statusY + 94, 22, activeColor);

        if (!aiTurn) {
            float barW = (float)layout.panelW - 52.0f;
            float barH = 9.0f;
            float barX = (float)layout.panelX + 26.0f;
            float barY = (float)statusY + 148.0f;

            float ratio = std::max(0.0f, humanTimer / HUMAN_TURN_TIME);
            Color timerColor = (humanTimer <= 5.0f) ? RED_MAIN : SKYBLUE;

            const char* timeText = TextFormat("TIME LEFT: %.1fs", std::max(0.0f, humanTimer));
            DrawText(timeText, cx - MeasureText(timeText, 15) / 2, (int)barY - 20, 15, timerColor);

            DrawRectangleRounded({ barX, barY, barW, barH }, 0.5f, 8, SLOT_EMPTY);
            if (ratio > 0.0f) {
                DrawRectangleRounded({ barX, barY, barW * ratio, barH }, 0.5f, 8, timerColor);
            }

            float hintW = (float)layout.panelW - 52.0f;
            float hintH = 34.0f;
            float hintX = (float)layout.panelX + 26.0f;
            float hintY = barY + 18.0f;
            Rectangle hintBtn = { hintX, hintY, hintW, hintH };

            bool canUseHint = (!showTreeOverlay && hintsLeft > 0 && hintCol == -1);
            bool hoverHint = canUseHint && CheckCollisionPointRec(mouse, hintBtn);

            Color btnFill   = canUseHint ? (hoverHint ? CARD_HOVER : SLOT_EMPTY) : BG_COLOR;
            Color btnBorder = (hintCol != -1) ? HINT_CYAN : (canUseHint ? (hoverHint ? HINT_CYAN : BOARD_BORDER) : BOARD_BORDER);
            Color btnText   = (hintCol != -1) ? HINT_CYAN : (canUseHint ? RAYWHITE : GRAY);

            DrawRectangleRounded(hintBtn, 0.24f, 10, btnFill);
            DrawRectangleRoundedLinesEx(hintBtn, 0.24f, 10, 1.5f, btnBorder);

            const char* hintLabel = (hintCol != -1)
                ? TextFormat("Suggested: Col %d", hintCol + 1)
                : TextFormat("Hint [H] (%d left)", hintsLeft);
            DrawText(hintLabel, cx - MeasureText(hintLabel, 16) / 2, (int)hintY + 9, 16, btnText);

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

        Rectangle resBox = { (float)layout.panelX + 24, (float)statusY, (float)layout.panelW - 48, 62.0f };
        DrawRectangleRounded(resBox, 0.16f, 12, SLOT_EMPTY);
        DrawRectangleRoundedLinesEx(resBox, 0.16f, 12, 2.0f, resultColor);
        DrawText(resultTitle, cx - MeasureText(resultTitle, 26) / 2, statusY + 18, 26, resultColor);

        float btnX = (float)layout.panelX + 24;
        float btnW = (float)layout.panelW - 48;
        float btnH = 40.0f;

        Rectangle restartBtn = { btnX, (float)statusY + 72.0f, btnW, btnH };
        bool hoverRestart = !showTreeOverlay && CheckCollisionPointRec(mouse, restartBtn);
        DrawRectangleRounded(restartBtn, 0.22f, 12, hoverRestart ? BTN_PRIM_HOVER : BTN_PRIMARY);
        const char* restartText = "Play Again  [R]";
        DrawText(restartText, cx - MeasureText(restartText, 18) / 2, (int)restartBtn.y + 11, 18, RAYWHITE);
        if (hoverRestart && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            panelAction = 1;
        }

        Rectangle menuBtn = { btnX, (float)statusY + 120.0f, btnW, btnH };
        bool hoverMenu = !showTreeOverlay && CheckCollisionPointRec(mouse, menuBtn);
        DrawRectangleRounded(menuBtn, 0.22f, 12, hoverMenu ? CARD_HOVER : SLOT_EMPTY);
        DrawRectangleRoundedLinesEx(menuBtn, 0.22f, 12, 1.5f, hoverMenu ? RAYWHITE : BOARD_BORDER);
        const char* menuText = "Main Menu  [M]";
        DrawText(menuText, cx - MeasureText(menuText, 18) / 2, (int)menuBtn.y + 11, 18, RAYWHITE);
        if (hoverMenu && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            panelAction = 2;
        }
    }

    // --- AI BENCHMARK METRICS & TREE BUTTON ---
    if (mode != GameMode::HumanVsHuman) {
        float statW = (float)layout.panelW - 48.0f;
        float statH = 68.0f;
        float statX = (float)layout.panelX + 24.0f;
        float statY = (float)(layout.boardY + layout.boardH) - 208.0f;

        DrawRectangleRounded({ statX, statY, statW, statH }, 0.16f, 10, SLOT_EMPTY);
        DrawRectangleRoundedLinesEx({ statX, statY, statW, statH }, 0.16f, 10, 1.0f, BOARD_BORDER);

        const char* perfStr = (lastStats.nodesExpanded > 0)
            ? TextFormat("Nodes: %lld  |  %.2f ms", lastStats.nodesExpanded, lastStats.timeMs)
            : "Nodes: -  |  Time: -";
        DrawText(perfStr, cx - MeasureText(perfStr, 14) / 2, (int)statY + 10, 14, LIGHTGRAY);

        Rectangle treeBtn = { statX + 10.0f, statY + 32.0f, statW - 20.0f, 28.0f };
        bool hasTree = !lastStats.treeLines.empty();
        bool hoverTree = (!showTreeOverlay && hasTree && CheckCollisionPointRec(mouse, treeBtn));

        DrawRectangleRounded(treeBtn, 0.25f, 8, hoverTree ? CARD_HOVER : BOARD_BODY);
        DrawRectangleRoundedLinesEx(treeBtn, 0.25f, 8, 1.2f, hasTree ? HINT_CYAN : BOARD_BORDER);
        const char* treeLbl = hasTree ? "View Decision Tree  [T]" : "Tree Ready After AI Move";
        DrawText(treeLbl, cx - MeasureText(treeLbl, 14) / 2, (int)treeBtn.y + 7, 14, hasTree ? HINT_CYAN : GRAY);

        if (hoverTree && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            panelAction = 4;
        }
    }

    // --- SESSION SCOREBOARD CARD ---
    float sbW = (float)layout.panelW - 48.0f;
    float sbH = 68.0f;
    float sbX = (float)layout.panelX + 24.0f;
    float sbY = (float)(layout.boardY + layout.boardH) - 128.0f;

    DrawRectangleRounded({ sbX, sbY, sbW, sbH }, 0.18f, 12, SLOT_EMPTY);
    DrawRectangleRoundedLinesEx({ sbX, sbY, sbW, sbH }, 0.18f, 12, 1.5f, BOARD_BORDER);

    float colW = sbW / 3.0f;
    const char* sbHeaders[3] = { "RED", "DRAW", "YELLOW" };
    int sbValues[3]          = { score.redWins, score.draws, score.yellowWins };
    Color sbColors[3]        = { RED_MAIN, LIGHTGRAY, YELLOW_MAIN };

    for (int i = 0; i < 3; i++) {
        int subCx = (int)(sbX + colW * i + colW / 2.0f);
        DrawText(sbHeaders[i], subCx - MeasureText(sbHeaders[i], 12) / 2, (int)sbY + 10, 12, sbColors[i]);
        const char* valStr = TextFormat("%d", sbValues[i]);
        DrawText(valStr, subCx - MeasureText(valStr, 24) / 2, (int)sbY + 30, 24, RAYWHITE);

        if (i < 2) {
            int divX = (int)(sbX + colW * (i + 1));
            DrawLine(divX, (int)sbY + 12, divX, (int)(sbY + sbH - 12), BOARD_BORDER);
        }
    }

    const char* navHint = "[1-7 / Arrows] Drop  |  [M] Menu";
    DrawText(navHint, cx - MeasureText(navHint, 15) / 2, layout.boardY + layout.boardH - 40, 15, GRAY);

    // --- RIGHT SIDE: BOARD CHASSIS ---
    Rectangle boardRect = {
        (float)layout.boardX - 10, (float)layout.boardY - 10,
        (float)layout.boardW + 20, (float)layout.boardH + 20
    };
    DrawRectangleRounded({ boardRect.x, boardRect.y + 8, boardRect.width, boardRect.height }, 0.04f, 16, { 0, 0, 0, 110 });
    DrawRectangleRounded(boardRect, 0.04f, 16, BOARD_BODY);
    DrawRectangleRoundedLinesEx(boardRect, 0.04f, 16, 2.0f, BOARD_BORDER);

    for (int c = 0; c < COLS; c++) {
        int colX = layout.boardX + c * layout.cellSize;
        int colCenterX = colX + layout.cellSize / 2;
        bool isSelected = (!g.gameOver && !g.gameDraw && !aiTurn && c == selectedCol);
        bool isHinted   = (!g.gameOver && !g.gameDraw && !aiTurn && c == hintCol);

        const char* colNum = TextFormat("%d", c + 1);
        Color numColor = isHinted ? HINT_CYAN : (isSelected ? RAYWHITE : GRAY);
        DrawText(colNum, colCenterX - MeasureText(colNum, 16) / 2, layout.boardY - 52, 16, numColor);

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

    if (showTreeOverlay) {
        if (drawTreeModal(lastStats, treeScrollY)) {
            panelAction = 4;
        }
    }

    EndDrawing();
    return panelAction;
}