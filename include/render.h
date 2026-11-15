#pragma once
#include "config.h"
#include "game.h"

struct BoardLayout {
    int boardX;
    int boardY;
    int boardW;
    int boardH;
    int cellSize;
    int panelX;
    int panelW;
};

BoardLayout getBoardLayout();
int drawMenu(); // Returns 0 (PvP), 1 (PvAI), 2 (AIvAI), or -1 (none)
int drawGame(const GameState& g, GameMode mode, float humanTimer); // Returns 1 (Restart), 2 (Main Menu), or 0 (none)