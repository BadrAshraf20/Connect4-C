#include "input.h"
#include "render.h"
#include "config.h"
#include "raylib.h"
#include <algorithm>

int getSelectedColumn(int& selectedCol) {
    BoardLayout layout = getBoardLayout();
    Vector2 mouseDelta = GetMouseDelta();
    int mouseX = GetMouseX();
    int mouseY = GetMouseY();

    bool mouseInsideBoard = (mouseX >= layout.boardX && mouseX < layout.boardX + layout.boardW &&
                             mouseY >= layout.boardY && mouseY < layout.boardY + layout.boardH);

    // Sync selected column when mouse moves over the board
    if ((mouseDelta.x != 0.0f || mouseDelta.y != 0.0f) && mouseInsideBoard) {
        selectedCol = (mouseX - layout.boardX) / layout.cellSize;
    }

    // Arrow / A-D navigation
    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) {
        selectedCol = (selectedCol - 1 + COLS) % COLS;
    }
    if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) {
        selectedCol = (selectedCol + 1) % COLS;
    }

    selectedCol = std::clamp(selectedCol, 0, COLS - 1);

    // Instant drop via number keys 1..7
    for (int i = 0; i < COLS; i++) {
        if (IsKeyPressed(KEY_ONE + i)) {
            selectedCol = i;
            return i;
        }
    }

    // Confirm drop via Enter/Space or Left-Click inside board
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
        return selectedCol;
    }
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && mouseInsideBoard) {
        return (mouseX - layout.boardX) / layout.cellSize;
    }

    return -1;
}