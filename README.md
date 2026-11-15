# Connect 4 🎮

A modern, modular C++ implementation of the classic Connect Four game built with the [Raylib](https://www.raylib.com/) library. Originally written as a single-file C project, it has been refactored into a clean multi-file procedural architecture featuring an intelligent **Minimax AI with Alpha-Beta Pruning**, selectable difficulty levels, a responsive fullscreen UI, blitz turn timers, and procedural sound effects.

## 🚀 Features

- **Three Game Modes & AI Difficulty Selector:**
  - **Human vs. Human:** Local 2-player match on the same screen.
  - **Human vs. AI:** Challenge an intelligent AI engine with natural thinking delays.
  - **AI vs. AI:** Watch two Minimax engines battle each other live.
  - **Selectable AI Difficulty:** Toggle between **Easy** (depth 2), **Medium** (depth 4), and **Hard** (depth 6) directly on the home screen.
- **Minimax AI Engine with Alpha-Beta Pruning:**
  - Evaluates all 69 horizontal, vertical, and diagonal 4-cell windows using a custom heuristic function.
  - Uses center-first column ordering and in-place backtracking for fast lookahead search.
- **Modern Fullscreen UI & Session Scoreboard:**
  - Interactive start menu with mode selection cards, difficulty pill toggle, and keyboard shortcuts (`1`–`3`).
  - Dynamic fullscreen board scaling with a side dashboard displaying turn status, persistent **Red / Draw / Yellow** session score counters, end-of-game results, and clickable **Play Again** / **Main Menu** buttons.
  - **Winning Line Highlight:** Pulsing highlight rings and a connecting line across the 4 winning tokens.
  - Column number headers (`1`–`7`) and hover/selection highlighting with a floating ghost token preview.
- **15-Second Blitz Turn Timer:** Keeps matches fast-paced with a visual countdown bar that automatically drops a piece in an open column if time expires.
- **Flexible Mouse & Keyboard Controls:** Play with mouse clicks, number keys (`1`–`7`), or `Left`/`Right` arrows + `Enter`/`Space`.
- **Procedural Audio:** In-memory synthesized sound effects for piece drops, final 5-second countdown ticks, victories, and draws, with zero external audio asset dependencies.

## 📁 Project Structure

```text
Connect4/
├── include/
│   ├── ai.h          # Minimax, Alpha-Beta pruning, and board heuristic declarations
│   ├── audio.h       # Procedural sound effect declarations (drop, tick, win, draw)
│   ├── config.h      # Grid dimensions, screen/mode/difficulty enums, and SessionScore
│   ├── game.h        # GameState struct (including winning cells) and core board rules
│   ├── input.h       # Mouse and keyboard column selection logic
│   └── render.h      # Responsive layout calculation, menu, scoreboard, and board rendering
├── src/
│   ├── ai.cpp
│   ├── audio.cpp
│   ├── game.cpp
│   ├── input.cpp
│   ├── main.cpp      # Main loop, timers, score tracking, and state transitions
│   └── render.cpp
└── README.md
```

## 🛠️ Technologies Used

- **Language:** C++17
- **Graphics & Audio Library:** Raylib 5.5
- **Algorithms:** Minimax Search, Alpha-Beta Pruning, Sliding-Window Heuristic Evaluation

## ⚙️ How to Compile and Run (Windows)

Ensure you have a C++ compiler (`g++` via MinGW/MSYS2) and Raylib 5.5 installed.

1. **Clone the repository:**

   ```bash
   git clone https://github.com/BadrAshraf20/Connect4-C.git
   cd Connect4-C
   ```

2. **Compile all source files (PowerShell):**

   Update the `-I` and `-L` paths to match where Raylib is installed on your machine.

   ```powershell
   g++ -O2 -std=c++17 (Get-ChildItem src/*.cpp) -Iinclude `
       -I"C:/path/to/raylib-5.5_win64_mingw-w64/include" `
       -L"C:/path/to/raylib-5.5_win64_mingw-w64/lib" `
       -lraylib -lopengl32 -lgdi32 -lwinmm -o Connect4.exe
   ```

3. **Run the game:**

   ```powershell
   .\Connect4.exe
   ```

## 🎮 How to Play

1. Launch the game, choose an AI difficulty (**Easy**, **Medium**, or **Hard**), and select a mode on the home screen by clicking a card or pressing `1`, `2`, or `3`.
2. On your turn, select a column and drop your token before the 15-second timer runs out.
3. Connect 4 tokens horizontally, vertically, or diagonally to win!

### Controls

| Key / Input                            | Action                                                       |
| -------------------------------------- | ------------------------------------------------------------ |
| `1` – `3` (on Home Menu)               | Select game mode (Human vs Human, Human vs AI, AI vs AI)     |
| Left-click on a column                 | Drop token into the clicked column                           |
| `1` – `7` (in game)                    | Immediately drop token into columns 1 through 7              |
| `Left` / `Right` arrows (or `A` / `D`) | Move column selector left or right                           |
| `Enter` / `Space`                      | Drop token into the currently selected column                |
| `R` (or **Play Again** button)         | Restart the match after a win or draw                        |
| `M` (or **Main Menu** button)          | Return to the home screen (resets session scoreboard)        |
| `ESC`                                  | Exit the game                                                |

## 👨‍💻 Author

**Badr Ashraf**
Computer and Systems Engineering Student, Faculty of Engineering, Alexandria University