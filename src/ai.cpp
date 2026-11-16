#include "ai.h"
#include "game.h"
#include <algorithm>
#include <chrono>
#include <climits>
#include <iostream>
#include <string>
#include <vector>

namespace {
    constexpr int COL_ORDER[COLS] = { 3, 2, 4, 1, 5, 0, 6 };
    constexpr int MAX_PRINT_DEPTH = 3;

    struct TreeNode {
        std::string label;
        int score = 0;
        bool pruned = false;
        int moveCol = -1;
        std::vector<TreeNode> children;
    };

    void undoPiece(char board[ROWS][COLS], int col) {
        for (int r = 0; r < ROWS; r++) {
            if (board[r][col] != '.') {
                board[r][col] = '.';
                return;
            }
        }
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

    int minimax(char board[ROWS][COLS], int depth, int treeLevel, int alpha, int beta,
                bool isMaximizing, char aiPiece, bool usePruning, GameRules rules,
                long long& nodes, TreeNode* node) {
        nodes++;
        char oppPiece = (aiPiece == 'X') ? 'O' : 'X';

        // In Classic FirstFour mode, stop immediately on the first 4-in-a-row
        if (rules == GameRules::FirstFour) {
            if (countConnectFours(board, aiPiece) > 0) {
                int val = 100000 + depth;
                if (node) node->score = val;
                return val;
            }
            if (countConnectFours(board, oppPiece) > 0) {
                int val = -100000 - depth;
                if (node) node->score = val;
                return val;
            }
        }

        // Board full terminal state
        if (checkDraw(board) == 1) {
            if (rules == GameRules::FullBoard) {
                int aiFours  = countConnectFours(board, aiPiece);
                int oppFours = countConnectFours(board, oppPiece);
                int val = (aiFours - oppFours) * 10000;
                if (node) node->score = val;
                return val;
            }
            if (node) node->score = 0;
            return 0;
        }

        // Heuristic cutoff at depth K
        if (depth == 0) {
            int val = evaluateBoard(board, aiPiece);
            if (node) node->score = val;
            return val;
        }

        bool recordChildren = (node != nullptr && treeLevel < MAX_PRINT_DEPTH);

        if (isMaximizing) {
            int maxEval = INT_MIN;
            for (int col : COL_ORDER) {
                if (board[0][col] == '.') {
                    TreeNode* childPtr = nullptr;
                    if (recordChildren) {
                        node->children.push_back({ "MAX Col " + std::to_string(col + 1), 0, false, col, {} });
                        childPtr = &node->children.back();
                    }

                    dropPiece(board, col, aiPiece);
                    int eval = minimax(board, depth - 1, treeLevel + 1, alpha, beta, false, aiPiece, usePruning, rules, nodes, childPtr);
                    undoPiece(board, col);

                    maxEval = std::max(maxEval, eval);
                    if (usePruning) {
                        alpha = std::max(alpha, eval);
                        if (beta <= alpha) {
                            if (recordChildren) {
                                node->children.push_back({ "PRUNED (beta <= alpha)", 0, true, -1, {} });
                            }
                            break;
                        }
                    }
                }
            }
            if (node) node->score = maxEval;
            return maxEval;
        } else {
            int minEval = INT_MAX;
            for (int col : COL_ORDER) {
                if (board[0][col] == '.') {
                    TreeNode* childPtr = nullptr;
                    if (recordChildren) {
                        node->children.push_back({ "MIN Col " + std::to_string(col + 1), 0, false, col, {} });
                        childPtr = &node->children.back();
                    }

                    dropPiece(board, col, oppPiece);
                    int eval = minimax(board, depth - 1, treeLevel + 1, alpha, beta, true, aiPiece, usePruning, rules, nodes, childPtr);
                    undoPiece(board, col);

                    minEval = std::min(minEval, eval);
                    if (usePruning) {
                        beta = std::min(beta, eval);
                        if (beta <= alpha) {
                            if (recordChildren) {
                                node->children.push_back({ "PRUNED (beta <= alpha)", 0, true, -1, {} });
                            }
                            break;
                        }
                    }
                }
            }
            if (node) node->score = minEval;
            return minEval;
        }
    }

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

    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c <= COLS - 4; c++) {
            for (int i = 0; i < 4; i++) window[i] = board[r][c + i];
            score += evaluateWindow(window, aiPiece, oppPiece);
        }
    }

    for (int r = 0; r <= ROWS - 4; r++) {
        for (int c = 0; c < COLS; c++) {
            for (int i = 0; i < 4; i++) window[i] = board[r + i][c];
            score += evaluateWindow(window, aiPiece, oppPiece);
        }
    }

    for (int r = 0; r <= ROWS - 4; r++) {
        for (int c = 0; c <= COLS - 4; c++) {
            for (int i = 0; i < 4; i++) window[i] = board[r + i][c + i];
            score += evaluateWindow(window, aiPiece, oppPiece);
        }
    }

    for (int r = 3; r < ROWS; r++) {
        for (int c = 0; c <= COLS - 4; c++) {
            for (int i = 0; i < 4; i++) window[i] = board[r - i][c + i];
            score += evaluateWindow(window, aiPiece, oppPiece);
        }
    }

    return score;
}

int getBestMove(char board[ROWS][COLS], char aiPiece, int maxDepth,
                AIAlgorithm algo, GameRules rules, SearchStats* outStats, bool printTree) {
    int bestScore = INT_MIN;
    int bestCol = -1;
    int alpha = INT_MIN;
    int beta = INT_MAX;
    long long nodesExpanded = 0;
    bool usePruning = (algo == AIAlgorithm::AlphaBeta);

    auto startTime = std::chrono::high_resolution_clock::now();

    TreeNode root{ "ROOT (MAX Turn, K=" + std::to_string(maxDepth) + ")", 0, false, -1, {} };
    TreeNode* rootPtr = (printTree || outStats != nullptr) ? &root : nullptr;

    for (int col : COL_ORDER) {
        if (board[0][col] == '.') {
            if (bestCol == -1) bestCol = col;

            TreeNode* childPtr = nullptr;
            if (rootPtr) {
                rootPtr->children.push_back({ "MAX Col " + std::to_string(col + 1), 0, false, col, {} });
                childPtr = &rootPtr->children.back();
            }

            dropPiece(board, col, aiPiece);
            int score = minimax(board, maxDepth - 1, 1, alpha, beta, false, aiPiece, usePruning, rules, nodesExpanded, childPtr);
            undoPiece(board, col);

            if (score > bestScore) {
                bestScore = score;
                bestCol = col;
            }
            if (usePruning) {
                alpha = std::max(alpha, bestScore);
            }
        }
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    double elapsedMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();

    const char* algoName = usePruning ? "Minimax + Alpha-Beta Pruning" : "Plain Minimax (No Pruning)";
    const char* ruleName = (rules == GameRules::FullBoard) ? "Full Board" : "First 4";
    root.score = bestScore;

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