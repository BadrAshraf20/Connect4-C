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
int drawMenu(GameRules& rules, AIAlgorithm& algorithm, AIDifficulty& difficulty, int& customK);
int drawGame(const GameState& g, GameMode mode, GameRules rules, AIAlgorithm algorithm, int activeK,
             float humanTimer, const SessionScore& score, int selectedCol,
             int hintsLeft, int hintCol, const SearchStats& lastStats,
             bool showTreeOverlay, float& treeScrollY);