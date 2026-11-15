# Connect 4 🎮

A modern, modular C++ implementation of the classic Connect Four game built with the [Raylib](https://www.raylib.com/) library. Originally written as a single-file C project, it has been refactored into a clean multi-file procedural architecture featuring an intelligent **Minimax AI with Alpha-Beta Pruning**, a responsive fullscreen UI, blitz turn timers, and procedural sound effects.

## 🚀 Features

- **Three Game Modes:**
  - **Human vs. Human:** Local 2-player match on the same screen.
  - **Human vs. AI:** Challenge an intelligent AI engine with natural thinking delays.
  - **AI vs. AI:** Watch two Minimax engines battle each other live.
- **Minimax AI Engine with Alpha-Beta Pruning:**
  - Evaluates all 69 horizontal, vertical, and diagonal 4-cell windows using a custom heuristic function.
  - Uses center-first column ordering and in-place backtracking for fast lookahead search.
- **Modern Fullscreen UI & Home Menu:**
  - Interactive start menu with mode selection cards and keyboard shortcuts (`[1]–[3]`).
  - Dynamic fullscreen board scaling with a side dashboard displaying turn status, end-of-game results, and clickable **Play Again** / **Main Menu** buttons.
  - Column hover highlighting with a floating ghost token preview.
- **15-Second Blitz Turn Timer:** Keeps matches fast-paced with a visual countdown bar that automatically drops a piece in an open column if time expires.
- **Procedural Audio:** In-memory synthesized sound effects for piece drops, victories, and draws with zero external audio asset dependencies.

## 📁 Project Structure

```text
Connect4/
├── include/
│   ├── ai.h          # Minimax, Alpha-Beta pruning, and board heuristic declarations
│   ├── audio.h       # Procedural sound effect declarations
│   ├── config.h      # Grid dimensions, screen/mode enums, and timer constants
│   ├── game.h        # GameState struct and core board rules
│   ├── input.h       # Mouse column click detection
│   └── render.h      # Responsive layout calculation, menu, and board rendering
├── src/
│   ├── ai.cpp
│   ├── audio.cpp
│   ├── game.cpp
│   ├── input.cpp
│   ├── main.cpp      # Main loop, timers, and state transitions
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

   > Update the `-I` and `-L` paths if your Raylib installation is located elsewhere.

   ```powershell
   g++ -O2 (Get-ChildItem src/*.cpp) -Iinclude -I"C:/Users/MyLap/Downloads/raylib-5.5_win64_mingw-w64/include" -L"C:/Users/MyLap/Downloads/raylib-5.5_win64_mingw-w64/lib" -lraylib -lopengl32 -lgdi32 -lwinmm -o Connect4.exe
   ```

3. **Run the game:**

   ```powershell
   .\Connect4.exe
   ```

## 🎮 How to Play

1. Launch the game and select a mode on the home screen by clicking a card or pressing `1`, `2`, or `3`.
2. On your turn, hover over any column and **Left-Click** to drop your token before the 15-second timer runs out.
3. Connect 4 tokens horizontally, vertically, or diagonally to win!

### Controls

| Key / Button | Action |
|---|---|
| `R` (or **Play Again** button) | Restart the match after a win or draw |
| `M` (or **Main Menu** button) | Return to the home screen |
| `ESC` | Exit the game |

## 👨‍💻 Author

**Badr Ashraf**
Computer and Systems Engineering Student, Faculty of Engineering, Alexandria University