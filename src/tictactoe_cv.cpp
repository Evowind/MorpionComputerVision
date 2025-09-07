#include "tictactoe_cv.h"
#include <iostream>

using namespace cv;
using namespace std;

TicTacToeCV::TicTacToeCV() : gridSize(3), gridDetected(false), playerTurn(true), drawing(false) {
    canvas = Mat::zeros(600, 800, CV_8UC3);
    canvas.setTo(paperColor);
    background = canvas.clone();
    
    // Ajouter texture papier
    addPaperTexture();
    
    gameState = vector<vector<int>>(gridSize, vector<int>(gridSize, 0));
    gridCells = vector<vector<Point>>(gridSize, vector<Point>(gridSize));
    
    cout << "=== JEU DE MORPION AVEC COMPUTER VISION ===" << endl;
    cout << "1. Dessinez une grille avec la souris" << endl;
    cout << "2. Cliquez dans une case pour dessiner un X" << endl;
    cout << "3. L'ordinateur dessinera automatiquement un O" << endl;
    cout << "Appuyez sur 'r' pour recommencer, 'q' pour quitter" << endl;
}

void TicTacToeCV::onMouse(int event, int x, int y, int flags, void* userdata) {
    TicTacToeCV* game = static_cast<TicTacToeCV*>(userdata);
    game->handleMouse(event, x, y, flags);
}

void TicTacToeCV::handleMouse(int event, int x, int y, int flags) {
    Point2f currentPos(x, y);
    
    if (event == EVENT_LBUTTONDOWN) {
        drawing = true;
        lastMousePos = currentPos;
        
        if (gridDetected && playerTurn) {
            // Vérifier si on clique dans une case
            int row, col;
            if (getCellFromPoint(currentPos, row, col)) {
                if (gameState[row][col] == 0) {
                    drawX(row, col);
                    gameState[row][col] = 1;
                    playerTurn = false;
                    
                    if (!checkGameEnd()) {
                        // Tour de l'ordinateur après un petit délai
                        playComputer();
                    }
                }
            }
        }
    } else if (event == EVENT_MOUSEMOVE && drawing) {
        if (!gridDetected) {
            // Dessiner la grille à main levée
            drawHandDrawnLine(lastMousePos, currentPos);
            lastMousePos = currentPos;
        }
    } else if (event == EVENT_LBUTTONUP) {
        drawing = false;
        if (!gridDetected) {
            detectGrid();
        }
    }
}

bool TicTacToeCV::checkGameEnd() {
    if (checkWin(1)) {
        putText(canvas, "VOUS GAGNEZ!", Point(200, 50), 
               FONT_HERSHEY_SIMPLEX, 1.5, Scalar(0, 150, 0), 3);
        cout << "Felicitations! Vous avez gagne!" << endl;
        return true;
    }
    
    if (checkWin(2)) {
        putText(canvas, "L'ORDI GAGNE!", Point(200, 50), 
               FONT_HERSHEY_SIMPLEX, 1.5, Scalar(0, 0, 200), 3);
        cout << "L'ordinateur a gagne!" << endl;
        return true;
    }
    
    // Vérifier match nul
    bool full = true;
    for (int i = 0; i < gridSize; i++) {
        for (int j = 0; j < gridSize; j++) {
            if (gameState[i][j] == 0) {
                full = false;
                break;
            }
        }
        if (!full) break;
    }
    
    if (full) {
        putText(canvas, "MATCH NUL!", Point(250, 50), 
               FONT_HERSHEY_SIMPLEX, 1.5, Scalar(100, 100, 100), 3);
        cout << "Match nul!" << endl;
        return true;
    }
    
    return false;
}

void TicTacToeCV::reset() {
    canvas = background.clone();
    gridDetected = false;
    playerTurn = true;
    gameState = vector<vector<int>>(3, vector<int>(3, 0));
    gridCells = vector<vector<Point>>(3, vector<Point>(3));
    gridSize = 3;
    
    cout << "\n=== NOUVEAU JEU ===" << endl;
    cout << "Dessinez votre grille!" << endl;
}

void TicTacToeCV::run() {
    namedWindow("Morpion Computer Vision", WINDOW_NORMAL);
    setMouseCallback("Morpion Computer Vision", onMouse, this);
    
    while (true) {
        // Ajouter instructions sur l'image
        Mat display = canvas.clone();

        if (!gridDetected) {
            putText(display, "Dessinez une grille avec la souris", 
                   Point(20, 30), FONT_HERSHEY_SIMPLEX, 0.7, inkColor, 2);
        } else if (playerTurn) {
            putText(display, "Votre tour - Cliquez dans une case", 
                   Point(20, 30), FONT_HERSHEY_SIMPLEX, 0.7, inkColor, 2);
        } else {
            putText(display, "Tour de l'ordinateur...", 
                   Point(20, 30), FONT_HERSHEY_SIMPLEX, 0.7, inkColor, 2);
        }
        
        imshow("Morpion Computer Vision", display);
        
        char key = waitKey(30);
        if (key == 'q' || key == 27) break; // Quitter
        if (key == 'r') reset(); // Reset
    }
    
    destroyAllWindows();
}