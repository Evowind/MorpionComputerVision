#include "tictactoe_cv.h"
#include <random>
#include <cmath>

using namespace cv;
using namespace std;

void TicTacToeCV::addPaperTexture() {
    RNG rng(12345);
    for (int i = 0; i < canvas.rows; i++) {
        for (int j = 0; j < canvas.cols; j++) {
            Vec3b& pixel = canvas.at<Vec3b>(i, j);
            int noise = rng.uniform(-5, 5);
            for (int c = 0; c < 3; c++) {
                pixel[c] = saturate_cast<uchar>(pixel[c] + noise);
            }
        }
    }
    
    // Ajouter quelques lignes de cahier subtiles
    for (int y = 50; y < canvas.rows; y += 30) {
        line(canvas, Point(50, y), Point(canvas.cols - 50, y), 
             Scalar(230, 230, 240), 1, LINE_AA);
    }
    
    background = canvas.clone();
}

void TicTacToeCV::drawHandDrawnLine(Point2f start, Point2f end) {
    // Simuler un trait tremblé à la main
    vector<Point2f> points;
    float distance = norm(end - start);
    int numPoints = max(2, (int)(distance / 3));
    
    for (int i = 0; i <= numPoints; i++) {
        float t = (float)i / numPoints;
        Point2f point = start + t * (end - start);
        
        // Ajouter du tremblement
        RNG rng(getTickCount() + i);
        point.x += rng.uniform(-1.5f, 1.5f);
        point.y += rng.uniform(-1.5f, 1.5f);
        
        points.push_back(point);
    }
    
    // Dessiner la ligne avec plusieurs traits pour effet d'encre
    for (size_t i = 1; i < points.size(); i++) {
        line(canvas, points[i-1], points[i], inkColor, 2, LINE_AA);
        line(canvas, Point(points[i-1].x + 1, points[i-1].y), 
             Point(points[i].x + 1, points[i].y), inkColor, 1, LINE_AA);
    }
}

void TicTacToeCV::drawX(int row, int col) {
    Point center = gridCells[row][col];
    int size = 20;
    
    // Dessiner X avec style gribouillage
    drawHandDrawnSymbol(center, size, 'X');
}

void TicTacToeCV::drawO(int row, int col) {
    Point center = gridCells[row][col];
    int size = 20;
    
    // Dessiner O avec style gribouillage
    drawHandDrawnSymbol(center, size, 'O');
}

void TicTacToeCV::drawHandDrawnSymbol(Point center, int size, char symbol) {
    RNG rng(getTickCount());
    
    if (symbol == 'X') {
        // Dessiner les deux diagonales du X
        for (int pass = 0; pass < 2; pass++) {
            vector<Point2f> points;
            Point2f start, end;
            
            if (pass == 0) {
                start = Point2f(center.x - size, center.y - size);
                end = Point2f(center.x + size, center.y + size);
            } else {
                start = Point2f(center.x + size, center.y - size);
                end = Point2f(center.x - size, center.y + size);
            }
            
            // Créer une ligne tremblée
            int numPoints = 15;
            for (int i = 0; i <= numPoints; i++) {
                float t = (float)i / numPoints;
                Point2f point = start + t * (end - start);
                point.x += rng.uniform(-2.0f, 2.0f);
                point.y += rng.uniform(-2.0f, 2.0f);
                points.push_back(point);
            }
            
            // Dessiner la ligne
            for (size_t i = 1; i < points.size(); i++) {
                line(canvas, points[i-1], points[i], inkColor, 3, LINE_AA);
            }
        }
    } else if (symbol == 'O') {
        // Dessiner un cercle tremblé
        vector<Point2f> points;
        int numPoints = 30;
        
        for (int i = 0; i <= numPoints; i++) {
            float angle = 2 * CV_PI * i / numPoints;
            float radius = size + rng.uniform(-3.0f, 3.0f);
            Point2f point;
            point.x = center.x + radius * cos(angle) + rng.uniform(-1.5f, 1.5f);
            point.y = center.y + radius * sin(angle) + rng.uniform(-1.5f, 1.5f);
            points.push_back(point);
        }
        
        // Dessiner le cercle
        for (size_t i = 1; i < points.size(); i++) {
            line(canvas, points[i-1], points[i], inkColor, 3, LINE_AA);
        }
        // Fermer le cercle
        line(canvas, points.back(), points[0], inkColor, 3, LINE_AA);
    }
}