#ifndef TICTACTOE_CV_H
#define TICTACTOE_CV_H

#include <opencv2/opencv.hpp>
#include <vector>

using namespace cv;
using namespace std;

class TicTacToeCV {
private:
    Mat canvas;
    Mat background;
    vector<vector<Point>> gridCells;
    vector<vector<int>> gameState; // 0: vide, 1: X (joueur), 2: O (ordi)
    int gridSize;
    bool gridDetected;
    bool playerTurn;
    Point2f lastMousePos;
    bool drawing;
    
    // Couleurs style cahier
    Scalar paperColor = Scalar(248, 248, 240);
    Scalar inkColor = Scalar(40, 40, 120);
    Scalar gridColor = Scalar(180, 180, 200);

public:
    TicTacToeCV();
    
    // Mouse handling
    static void onMouse(int event, int x, int y, int flags, void* userdata);
    void handleMouse(int event, int x, int y, int flags);
    
    // Game logic
    void run();
    void reset();
    bool checkGameEnd();
    
    // Grid detection methods
    void detectGrid();
    void calculateCellCenters(const vector<Vec4i>& hLines, const vector<Vec4i>& vLines);
    bool getCellFromPoint(Point2f point, int& row, int& col);
    
    // Drawing methods
    void addPaperTexture();
    void drawHandDrawnLine(Point2f start, Point2f end);
    void drawX(int row, int col);
    void drawO(int row, int col);
    void drawHandDrawnSymbol(Point center, int size, char symbol);
    
    // AI methods
    void playComputer();
    bool findWinningMove(int player, int& bestRow, int& bestCol);
    bool checkWin(int player);
};

#endif