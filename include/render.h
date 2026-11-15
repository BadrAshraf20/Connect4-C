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
int drawMenu(AIDifficulty& difficulty);
int drawGame(const GameState& g, GameMode mode, AIDifficulty difficulty, float humanTimer,
             const SessionScore& score, int selectedCol);