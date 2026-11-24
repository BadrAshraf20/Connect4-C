#include "ai.h"
#include "game.h"
#include <algorithm>
#include <chrono>
#include <climits>
#include <iostream>
#include <string>
#include <vector>

namespace {
    // Search the center first: better move ordering gives more alpha-beta cutoffs
    constexpr int COL_ORDER[COLS] = { 3, 2, 4, 1, 5, 0, 6 };
    // Limit visual/console tree logging to the top 3 levels to avoid massive output
    constexpr int MAX_PRINT_DEPTH = 3;

    // Internal node structure used to build the decision tree for console and GUI display
    struct TreeNode {
        std::string label;
        int score = 0;
        bool pruned = false;
        int moveCol = -1;
        std::vector<TreeNode> children;
    };

    // Remove the topmost piece of a column (exact inverse of dropPiece)
    void undoPiece(char board[ROWS][COLS], int col) {
        for (int r = 0; r < ROWS; r++) {
            if (board[r][col] != '.') {
                board[r][col] = '.';
                return;
            }
        }
    }

    // Row of the topmost piece in a column (i.e. where the last piece landed)
    int topRow(const char board[ROWS][COLS], int col) {
        for (int r = 0; r < ROWS; r++) {
            if (board[r][col] != '.') return r;
        }
        return -1;
    }

    // FIX: checks only the 4 lines through the last-played cell,
    // instead of scanning the whole board twice per node.
    bool lastMoveWins(const char board[ROWS][COLS], int row, int col) {
        char p = board[row][col];
        const int dirs[4][2] = { {0, 1}, {1, 0}, {1, 1}, {-1, 1} };

        for (const auto& d : dirs) {
            int count = 1;  // the piece itself
            for (int sign = -1; sign <= 1; sign += 2) {
                int r = row + sign * d[0];
                int c = col + sign * d[1];
                while (r >= 0 && r < ROWS && c >= 0 && c < COLS && board[r][c] == p) {
                    count++;
                    r += sign * d[0];
                    c += sign * d[1];
                }
            }
            if (count >= 4) return true;
        }
        return false;
    }

    // FIX: used to show "PRUNED" only when columns were really skipped
    bool hasRemainingMove(const char board[ROWS][COLS], int fromIdx) {
        for (int i = fromIdx; i < COLS; i++) {
            if (board[0][COL_ORDER[i]] == '.') return true;
        }
        return false;
    }

    int evaluateWindow(const char w[4], char aiPiece, char oppPiece) {
        int aiCount = 0;
        int oppCount = 0;
        int emptyCount = 0;

        for (int i = 0; i < 4; i++) {
            if (w[i] == aiPiece) aiCount++;
            else if (w[i] == oppPiece) oppCount++;
            else emptyCount++;
        }

        // Completed 4-in-a-row sequences
        if (aiCount == 4) return 1000;
        if (oppCount == 4) return -1050;

        // Candidate threat windows
        if (aiCount == 3 && emptyCount == 1) return 25;
        if (aiCount == 2 && emptyCount == 2) return 5;

        if (oppCount == 3 && emptyCount == 1) return -28;
        if (oppCount == 2 && emptyCount == 2) return -5;

        return 0;
    }

    // lastRow/lastCol = cell of the piece that was just dropped (used for the fast win check)
    int minimax(char board[ROWS][COLS], int depth, int treeLevel, int alpha, int beta,
                bool isMaximizing, char aiPiece, bool usePruning, GameRules rules,
                long long& nodes, TreeNode* node, int lastRow, int lastCol) {
        nodes++;
        char oppPiece = (aiPiece == 'X') ? 'O' : 'X';

        // FirstFour: the game ends as soon as the last move makes a 4-in-a-row
        if (rules == GameRules::FirstFour && lastMoveWins(board, lastRow, lastCol)) {
            // Earlier win (larger depth left) scores higher; earlier loss scores lower
            int val = (board[lastRow][lastCol] == aiPiece) ? (100000 + depth) : (-100000 - depth);
            if (node) node->score = val;
            return val;
        }

        // Board full terminal state
        if (checkDraw(board) == 1) {
            if (rules == GameRules::FullBoard) {
                // Final result = difference in number of connected fours
                int aiFours  = countConnectFours(board, aiPiece);
                int oppFours = countConnectFours(board, oppPiece);
                int val = (aiFours - oppFours) * 10000;
                if (node) node->score = val;
                return val;
            }
            if (node) node->score = 0;
            return 0;
        }

        // Heuristic cutoff when reaching search depth limit K
        if (depth == 0) {
            int val = evaluateBoard(board, aiPiece);
            if (rules == GameRules::FullBoard) {
                // FIX: evaluateBoard already counts each four as +1000 / -1050.
                // Top it up so a four is worth 10000, the same as at a real terminal state.
                val += countConnectFours(board, aiPiece) * 9000
                     - countConnectFours(board, oppPiece) * 8950;
            }
            if (node) node->score = val;
            return val;
        }

        bool recordChildren = (node != nullptr && treeLevel < MAX_PRINT_DEPTH);

        if (isMaximizing) {
            // AI's turn (MAX): maximize score and update alpha
            int maxEval = INT_MIN;
            for (int i = 0; i < COLS; i++) {
                int col = COL_ORDER[i];
                if (board[0][col] != '.') continue;  // column full

                TreeNode* childPtr = nullptr;
                if (recordChildren) {
                    node->children.push_back({ "MAX Col " + std::to_string(col + 1), 0, false, col, {} });
                    childPtr = &node->children.back();
                }

                dropPiece(board, col, aiPiece);
                int row = topRow(board, col);
                int eval = minimax(board, depth - 1, treeLevel + 1, alpha, beta, false, aiPiece,
                                   usePruning, rules, nodes, childPtr, row, col);
                undoPiece(board, col);  // restore board

                maxEval = std::max(maxEval, eval);
                if (usePruning) {
                    alpha = std::max(alpha, eval);
                    if (beta <= alpha) {
                        // FIX: only mark PRUNED if some columns were actually skipped
                        if (recordChildren && hasRemainingMove(board, i + 1)) {
                            node->children.push_back({ "PRUNED (beta <= alpha)", 0, true, -1, {} });
                        }
                        break;
                    }
                }
            }
            // FIX: safety net if no move was possible (should not happen after checkDraw)
            if (maxEval == INT_MIN) maxEval = 0;
            if (node) node->score = maxEval;
            return maxEval;
        } else {
            // Opponent's turn (MIN): minimize score and update beta
            int minEval = INT_MAX;
            for (int i = 0; i < COLS; i++) {
                int col = COL_ORDER[i];
                if (board[0][col] != '.') continue;  // column full

                TreeNode* childPtr = nullptr;
                if (recordChildren) {
                    node->children.push_back({ "MIN Col " + std::to_string(col + 1), 0, false, col, {} });
                    childPtr = &node->children.back();
                }

                dropPiece(board, col, oppPiece);
                int row = topRow(board, col);
                int eval = minimax(board, depth - 1, treeLevel + 1, alpha, beta, true, aiPiece,
                                   usePruning, rules, nodes, childPtr, row, col);
                undoPiece(board, col);  // restore board

                minEval = std::min(minEval, eval);
                if (usePruning) {
                    beta = std::min(beta, eval);
                    if (beta <= alpha) {
                        // FIX: only mark PRUNED if some columns were actually skipped
                        if (recordChildren && hasRemainingMove(board, i + 1)) {
                            node->children.push_back({ "PRUNED (beta <= alpha)", 0, true, -1, {} });
                        }
                        break;
                    }
                }
            }
            // FIX: safety net if no move was possible (should not happen after checkDraw)
            if (minEval == INT_MAX) minEval = 0;
            if (node) node->score = minEval;
            return minEval;
        }
    }

    // Traverses the recorded search tree to print to stdout and populate GUI overlay lines
    void collectAndPrintTree(const TreeNode& node, int currentLevel, int chosenRootCol,
                             bool printToConsole, std::vector<TreeDisplayLine>* guiLines) {
        std::string indent(currentLevel * 4, ' ');
        bool chosenBranch = (currentLevel == 1 && node.moveCol == chosenRootCol);

        if (printToConsole) {
            if (node.pruned) {
                std::cout << indent << "|-- [X] " << node.label << "\n";
            } else {
                std::cout << indent << "|-- " << node.label << "  =>  Score: " << node.score
                          << (chosenBranch ? "  <-- CHOSEN" : "") << "\n";
            }
        }

        if (guiLines) {
            std::string displayStr = node.pruned
                ? ("[X] " + node.label)
                : (node.label + "   =>   Score: " + std::to_string(node.score) + (chosenBranch ? "   [CHOSEN]" : ""));
            guiLines->push_back({ currentLevel, displayStr, node.score, node.pruned, chosenBranch });
        }

        if (node.pruned) return;

        for (const auto& child : node.children) {
            collectAndPrintTree(child, currentLevel + 1, chosenRootCol, printToConsole, guiLines);
        }
    }
}

// Heuristic board evaluation combining center control and all 69 4-cell windows
int evaluateBoard(char board[ROWS][COLS], char aiPiece) {
    char oppPiece = (aiPiece == 'X') ? 'O' : 'X';
    int score = 0;

    // 1. Center & inner-column positional control
    int centerCol = COLS / 2;
    for (int r = 0; r < ROWS; r++) {
        if (board[r][centerCol] == aiPiece) score += 6;
        else if (board[r][centerCol] == oppPiece) score -= 6;

        if (board[r][centerCol - 1] == aiPiece || board[r][centerCol + 1] == aiPiece) score += 3;
        if (board[r][centerCol - 1] == oppPiece || board[r][centerCol + 1] == oppPiece) score -= 3;
    }

    // 2. Evaluate all 69 four-cell windows (includes completed 4-in-a-rows + candidate threats)
    char window[4];

    // Horizontal
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c <= COLS - 4; c++) {
            for (int i = 0; i < 4; i++) window[i] = board[r][c + i];
            score += evaluateWindow(window, aiPiece, oppPiece);
        }
    }

    // Vertical
    for (int r = 0; r <= ROWS - 4; r++) {
        for (int c = 0; c < COLS; c++) {
            for (int i = 0; i < 4; i++) window[i] = board[r + i][c];
            score += evaluateWindow(window, aiPiece, oppPiece);
        }
    }

    // Diagonal down-right
    for (int r = 0; r <= ROWS - 4; r++) {
        for (int c = 0; c <= COLS - 4; c++) {
            for (int i = 0; i < 4; i++) window[i] = board[r + i][c + i];
            score += evaluateWindow(window, aiPiece, oppPiece);
        }
    }

    // Diagonal up-right
    for (int r = 3; r < ROWS; r++) {
        for (int c = 0; c <= COLS - 4; c++) {
            for (int i = 0; i < 4; i++) window[i] = board[r - i][c + i];
            score += evaluateWindow(window, aiPiece, oppPiece);
        }
    }

    return score;
}

// Entry point for the AI: evaluates all valid root moves, tracks time/nodes, and returns best column
int getBestMove(char board[ROWS][COLS], char aiPiece, int maxDepth,
                AIAlgorithm algo, GameRules rules, SearchStats* outStats, bool printTree) {
    // FIX: depth must be at least 1, otherwise the search never reaches depth 0
    maxDepth = std::max(1, maxDepth);

    int bestScore = INT_MIN;
    int bestCol = -1;
    int alpha = INT_MIN;
    int beta = INT_MAX;
    long long nodesExpanded = 0;
    bool usePruning = (algo == AIAlgorithm::AlphaBeta);

    auto startTime = std::chrono::high_resolution_clock::now();

    TreeNode root{ "ROOT (MAX Turn, K=" + std::to_string(maxDepth) + ")", 0, false, -1, {} };
    TreeNode* rootPtr = (printTree || outStats != nullptr) ? &root : nullptr;

    // Root = MAX layer: try each column and keep the best score
    for (int col : COL_ORDER) {
        if (board[0][col] == '.') {
            if (bestCol == -1) bestCol = col;  // fallback: first legal column

            TreeNode* childPtr = nullptr;
            if (rootPtr) {
                rootPtr->children.push_back({ "MAX Col " + std::to_string(col + 1), 0, false, col, {} });
                childPtr = &rootPtr->children.back();
            }

            dropPiece(board, col, aiPiece);
            int row = topRow(board, col);
            int score = minimax(board, maxDepth - 1, 1, alpha, beta, false, aiPiece,
                                usePruning, rules, nodesExpanded, childPtr, row, col);
            undoPiece(board, col);  // restore board

            // Strict '>' keeps the earliest (most central) column on ties
            if (score > bestScore) {
                bestScore = score;
                bestCol = col;
            }
            if (usePruning) {
                alpha = std::max(alpha, bestScore);
            }
            // NOTE: with pruning, non-chosen root scores are only bounds, so they
            // can differ from plain minimax. The chosen column and score are the same.
        }
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    double elapsedMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();

    const char* algoName = usePruning ? "Minimax + Alpha-Beta Pruning" : "Plain Minimax (No Pruning)";
    const char* ruleName = (rules == GameRules::FullBoard) ? "Full Board" : "First 4";
    root.score = bestScore;

    // Populate search statistics for the side panel and tree overlay
    if (outStats) {
        outStats->nodesExpanded = nodesExpanded;
        outStats->timeMs = elapsedMs;
        outStats->chosenCol = bestCol;
        outStats->bestScore = bestScore;
        outStats->algoSummary = std::string(algoName) + "  |  " + ruleName +
                                "  |  K = " + std::to_string(maxDepth) +
                                "  |  Chosen Col " + std::to_string(bestCol + 1);
        outStats->treeLines.clear();
    }

    // Print summary header and decision tree to console if enabled
    if (printTree) {
        std::cout << "\n======================================================\n";
        std::cout << "Algorithm: " << algoName << " | Rules: " << ruleName << " | Depth K = " << maxDepth << "\n";
        std::cout << "Chosen Column: " << (bestCol + 1) << " | Best Score: " << bestScore
                  << " | Nodes Expanded: " << nodesExpanded
                  << " | Time: " << elapsedMs << " ms\n";
        std::cout << "------------------------------------------------------\n";
    }

    if (printTree || outStats) {
        collectAndPrintTree(root, 0, bestCol, printTree, outStats ? &outStats->treeLines : nullptr);
    }

    if (printTree) {
        std::cout << "======================================================\n";
    }

    return bestCol;
}
