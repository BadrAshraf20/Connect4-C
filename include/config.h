#pragma once

constexpr int ROWS = 6;
constexpr int COLS = 7;
constexpr int CELL_SIZE = 100;
constexpr int PADDING = 50;
constexpr float HUMAN_TURN_TIME = 15.0f;

enum class ScreenState {
    Menu,
    Playing
};

enum class GameMode {
    HumanVsHuman,
    HumanVsAI,
    AIVsAI
};

enum class AIDifficulty {
    Easy = 2,
    Medium = 4,
    Hard = 6
};

struct SessionScore {
    int redWins = 0;
    int yellowWins = 0;
    int draws = 0;
};