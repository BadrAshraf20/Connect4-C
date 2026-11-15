#pragma once

// Updates `selectedCol` from mouse movement or arrow keys,
// and returns the chosen column (0 to COLS - 1) if confirmed, or -1 otherwise.
int getSelectedColumn(int& selectedCol);