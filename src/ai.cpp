#include "tictactoe_cv.h"
#include <random>
#include <iostream>

using namespace cv;
using namespace std;

void TicTacToeCV::playComputer() {
    // Stratégie simple de l'IA
    int bestRow = -1, bestCol = -1;
    
    // 1. Essayer de gagner
    if (findWinningMove(2, bestRow, bestCol)) {
        gameState[bestRow][bestCol] = 2;
        drawO(bestRow, bestCol);
        playerTurn = true;
        checkGameEnd();
        return;
    }
    
    // 2. Bloquer le joueur
    if (findWinningMove(1, bestRow, bestCol)) {
        gameState[bestRow][bestCol] = 2;
        drawO(bestRow, bestCol);
        playerTurn = true;
        checkGameEnd();
        return;
    }
    
    // 3. Jouer au centre si disponible
    if (gridSize % 2 == 1) {
        int center = gridSize / 2;
        if (gameState[center][center] == 0) {
            gameState[center][center] = 2;
            drawO(center, center);
            playerTurn = true;
            checkGameEnd();
            return;
        }
    }
    
    // 4. Jouer aléatoirement
    vector<pair<int, int>> emptyCells;
    for (int i = 0; i < gridSize; i++) {
        for (int j = 0; j < gridSize; j++) {
            if (gameState[i][j] == 0) {
                emptyCells.push_back({i, j});
            }
        }
    }
    
    if (!emptyCells.empty()) {
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<> dis(0, emptyCells.size() - 1);
        auto move = emptyCells[dis(gen)];
        
        gameState[move.first][move.second] = 2;
        drawO(move.first, move.second);
    }
    
    playerTurn = true;
    checkGameEnd();
}

bool TicTacToeCV::findWinningMove(int player, int& bestRow, int& bestCol) {
    for (int i = 0; i < gridSize; i++) {
        for (int j = 0; j < gridSize; j++) {
            if (gameState[i][j] == 0) {
                gameState[i][j] = player;
                if (checkWin(player)) {
                    bestRow = i;
                    bestCol = j;
                    gameState[i][j] = 0; // Annuler le mouvement test
                    return true;
                }
                gameState[i][j] = 0; // Annuler le mouvement test
            }
        }
    }
    return false;
}

bool TicTacToeCV::checkWin(int player) {
    // Vérifier lignes et colonnes
    for (int i = 0; i < gridSize; i++) {
        bool winRow = true, winCol = true;
        for (int j = 0; j < gridSize; j++) {
            if (gameState[i][j] != player) winRow = false;
            if (gameState[j][i] != player) winCol = false;
        }
        if (winRow || winCol) return true;
    }
    
    // Vérifier diagonales
    bool winDiag1 = true, winDiag2 = true;
    for (int i = 0; i < gridSize; i++) {
        if (gameState[i][i] != player) winDiag1 = false;
        if (gameState[i][gridSize - 1 - i] != player) winDiag2 = false;
    }
    
    return winDiag1 || winDiag2;
}