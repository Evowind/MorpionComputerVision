#include "tictactoe_cv.h"
#include <algorithm>
#include <cmath>

using namespace cv;
using namespace std;

void TicTacToeCV::detectGrid() {
    Mat gray, edges;
    cvtColor(canvas, gray, COLOR_BGR2GRAY);
    GaussianBlur(gray, gray, Size(3, 3), 0);

    // Canny + fermeture morphologique pour combler les trous
    Canny(gray, edges, 50, 150);
    Mat kernel = getStructuringElement(MORPH_RECT, Size(3, 3));
    morphologyEx(edges, edges, MORPH_CLOSE, kernel);

    vector<Vec4i> lines;
    HoughLinesP(edges, lines, 1, CV_PI/180, 40, 40, 50);

    vector<Vec4i> horizontalLines, verticalLines;

    // Classer par orientation
    for (const auto& line : lines) {
        Point p1(line[0], line[1]);
        Point p2(line[2], line[3]);
        double angle = atan2(p2.y - p1.y, p2.x - p1.x) * 180.0 / CV_PI;
        angle = fmod(fabs(angle), 180.0);

        if (angle < 20 || angle > 160) {
            horizontalLines.push_back(line);
        } else if (angle > 70 && angle < 110) {
            verticalLines.push_back(line);
        }
    }

    // Fonction pour fusionner les segments proches
    auto mergeLines = [](vector<Vec4i>& lines, bool horizontal) {
        vector<Vec4i> merged;
        if (lines.empty()) return merged;

        // Trier les lignes par coordonnée dominante (y pour horizontales, x pour verticales)
        sort(lines.begin(), lines.end(), [horizontal](const Vec4i& a, const Vec4i& b) {
            if (horizontal) return (a[1] + a[3]) / 2 < (b[1] + b[3]) / 2;
            else return (a[0] + a[2]) / 2 < (b[0] + b[2]) / 2;
        });

        // Grouper les lignes proches
        Vec4i current = lines[0];
        for (size_t i = 1; i < lines.size(); i++) {
            Vec4i l = lines[i];
            if (horizontal) {
                int y1 = (current[1] + current[3]) / 2;
                int y2 = (l[1] + l[3]) / 2;
                if (abs(y1 - y2) < 15) { 
                    // Fusionner horizontalement
                    current[0] = min(current[0], min(l[0], l[2]));
                    current[2] = max(current[2], max(l[0], l[2]));
                } else {
                    merged.push_back(current);
                    current = l;
                }
            } else {
                int x1 = (current[0] + current[2]) / 2;
                int x2 = (l[0] + l[2]) / 2;
                if (abs(x1 - x2) < 15) {
                    // Fusionner verticalement
                    current[1] = min(current[1], min(l[1], l[3]));
                    current[3] = max(current[3], max(l[1], l[3]));
                } else {
                    merged.push_back(current);
                    current = l;
                }
            }
        }
        merged.push_back(current);
        return merged;
    };

    horizontalLines = mergeLines(horizontalLines, true);
    verticalLines   = mergeLines(verticalLines, false);

    if (horizontalLines.size() >= 2 && verticalLines.size() >= 2) {
        sort(horizontalLines.begin(), horizontalLines.end(),
             [](const Vec4i& a, const Vec4i& b) {
                 return (a[1] + a[3]) / 2 < (b[1] + b[3]) / 2;
             });
        sort(verticalLines.begin(), verticalLines.end(),
             [](const Vec4i& a, const Vec4i& b) {
                 return (a[0] + a[2]) / 2 < (b[0] + b[2]) / 2;
             });

        int rows = horizontalLines.size() + 1;
        int cols = verticalLines.size() + 1;

        if (rows <= 5 && cols <= 5 && rows >= 2 && cols >= 2) {
            gridSize = max(rows, cols);
            gameState = vector<vector<int>>(gridSize, vector<int>(gridSize, 0));
            gridCells = vector<vector<Point>>(gridSize, vector<Point>(gridSize));

            calculateCellCenters(horizontalLines, verticalLines);
            gridDetected = true;

            cout << "Grille detectee: " << gridSize << "x" << gridSize << endl;
            cout << "Cliquez dans une case pour jouer!" << endl;
        }
    }
}

void TicTacToeCV::calculateCellCenters(const vector<Vec4i>& hLines, const vector<Vec4i>& vLines) {
    vector<int> hPositions, vPositions;
    
    // Positions des lignes horizontales
    hPositions.push_back(50); // Bord supérieur
    for (const auto& line : hLines) {
        hPositions.push_back((line[1] + line[3]) / 2);
    }
    hPositions.push_back(canvas.rows - 50); // Bord inférieur
    
    // Positions des lignes verticales
    vPositions.push_back(50); // Bord gauche
    for (const auto& line : vLines) {
        vPositions.push_back((line[0] + line[2]) / 2);
    }
    vPositions.push_back(canvas.cols - 50); // Bord droit
    
    // Calculer les centres des cases
    for (int i = 0; i < gridSize; i++) {
        for (int j = 0; j < gridSize; j++) {
            int centerX = (vPositions[j] + vPositions[j + 1]) / 2;
            int centerY = (hPositions[i] + hPositions[i + 1]) / 2;
            gridCells[i][j] = Point(centerX, centerY);
        }
    }
}

bool TicTacToeCV::getCellFromPoint(Point2f point, int& row, int& col) {
    float minDist = 1000;
    int bestRow = -1, bestCol = -1;
    
    for (int i = 0; i < gridSize; i++) {
        for (int j = 0; j < gridSize; j++) {
            float dist = norm(point - Point2f(gridCells[i][j]));
            if (dist < minDist && dist < 60) { // Seuil de proximité
                minDist = dist;
                bestRow = i;
                bestCol = j;
            }
        }
    }
    
    if (bestRow != -1) {
        row = bestRow;
        col = bestCol;
        return true;
    }
    return false;
}