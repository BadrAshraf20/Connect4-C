#include "raylib.h"
#include "input.h"
#include "render.h"
#include "config.h"

int getClickedColumn() {
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        BoardLayout layout = getBoardLayout();
        int mouseX = GetMouseX();
        int mouseY = GetMouseY();

        if (mouseX >= layout.boardX && mouseX < layout.boardX + layout.boardW &&
            mouseY >= layout.boardY && mouseY < layout.boardY + layout.boardH) {
            return (mouseX - layout.boardX) / layout.cellSize;
        }
    }
    return -1;
}