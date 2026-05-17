# Connect 4 🎮

A classic Connect Four game implemented in C using the [Raylib](https://www.raylib.com/) library. This project demonstrates foundational game logic, 2D array manipulation, and graphical user interface (GUI) handling.

## 🚀 Features
* **Interactive GUI:** Clean graphics and smooth mouse controls.
* **Two-Player Local Mode:** Play against a friend (Red vs. Yellow).
* **Robust Win Detection:** Real-time checking for vertical, horizontal, and both diagonal connections with optimized boundary checks.
* **Draw Detection:** Automatically recognizes when the board is full with no winner.
* **Clean Logic:** Avoids out-of-bounds memory errors and magic numbers.

## 🛠️ Technologies Used
* **Language:** C
* **Graphics Library:** Raylib 5.5

## ⚙️ How to Compile and Run (Windows)

This project is configured for Windows using MinGW. Ensure you have a C compiler (GCC) and the Raylib library installed.

Run the following command in your terminal to compile the game:

## 🎮 How to Play
1. Launch the game.
2. Players take turns clicking on the column where they want to drop their piece.
3. The first player to connect 4 pieces horizontally, vertically, or diagonally wins!

## 👨‍💻 Author
**Badr Ashraf**
*Computer and Systems Engineering Student*

```bash
gcc Connect4.c -o connect4 -lraylib -lopengl32 -lgdi32 -lwinmm
