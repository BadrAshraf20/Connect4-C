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