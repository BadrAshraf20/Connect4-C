#pragma once
#include <string>
#include <vector>

constexpr int ROWS = 6;
constexpr int COLS = 7;
constexpr int CELL_SIZE = 100;
constexpr int PADDING = 50;
constexpr float HUMAN_TURN_TIME = 30.0f;
constexpr int MAX_HINTS_PER_PLAYER = 3;

enum class ScreenState {
    Menu,
    Playing
};

enum class GameMode {
    HumanVsHuman,
    HumanVsAI,
    AIVsAI
};

enum class GameRules {
    FullBoard, // Lab 2 requirement: play until board is full, highest connected-fours wins
    FirstFour  // Classic Connect 4: game ends on the first 4-in-a-row
};

enum class AIAlgorithm {
    AlphaBeta,
    PlainMinimax
};

enum class AIDifficulty {
    Easy = 2,
    Medium = 4,
    Hard = 6,
    Custom = 0
};

struct SessionScore {
    int redWins = 0;
    int yellowWins = 0;
    int draws = 0;
};

struct TreeDisplayLine {
    int level = 0;
    std::string text;
    int score = 0;
    bool isPruned = false;
    bool isChosen = false;
};

struct SearchStats {
    long long nodesExpanded = 0;
    double timeMs = 0.0;
    int chosenCol = -1;
    int bestScore = 0;
    std::string algoSummary;
    std::vector<TreeDisplayLine> treeLines;
};