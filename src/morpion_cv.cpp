#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <cmath>

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
    TicTacToeCV() : gridSize(3), gridDetected(false), playerTurn(true), drawing(false) {
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
    
    void addPaperTexture() {
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
    
    static void onMouse(int event, int x, int y, int flags, void* userdata) {
        TicTacToeCV* game = static_cast<TicTacToeCV*>(userdata);
        game->handleMouse(event, x, y, flags);
    }
    
    void handleMouse(int event, int x, int y, int flags) {
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
    
    void drawHandDrawnLine(Point2f start, Point2f end) {
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
    
    void detectGrid() {
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

            cout << "Grille détectée: " << gridSize << "x" << gridSize << endl;
            cout << "Cliquez dans une case pour jouer!" << endl;
        }
    }
}

    void calculateCellCenters(const vector<Vec4i>& hLines, const vector<Vec4i>& vLines) {
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
    
    bool getCellFromPoint(Point2f point, int& row, int& col) {
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
    
    void drawX(int row, int col) {
        Point center = gridCells[row][col];
        int size = 20;
        
        // Dessiner X avec style gribouillage
        drawHandDrawnSymbol(center, size, 'X');
    }
    
    void drawO(int row, int col) {
        Point center = gridCells[row][col];
        int size = 20;
        
        // Dessiner O avec style gribouillage
        drawHandDrawnSymbol(center, size, 'O');
    }
    
    void drawHandDrawnSymbol(Point center, int size, char symbol) {
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
    
    void playComputer() {
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
    
    bool findWinningMove(int player, int& bestRow, int& bestCol) {
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
    
    bool checkWin(int player) {
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
    
    bool checkGameEnd() {
        if (checkWin(1)) {
            putText(canvas, "VOUS GAGNEZ!", Point(200, 50), 
                   FONT_HERSHEY_SIMPLEX, 1.5, Scalar(0, 150, 0), 3);
            cout << "Félicitations! Vous avez gagné!" << endl;
            return true;
        }
        
        if (checkWin(2)) {
            putText(canvas, "L'ORDI GAGNE!", Point(200, 50), 
                   FONT_HERSHEY_SIMPLEX, 1.5, Scalar(0, 0, 200), 3);
            cout << "L'ordinateur a gagné!" << endl;
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
    
    void reset() {
        canvas = background.clone();
        gridDetected = false;
        playerTurn = true;
        gameState = vector<vector<int>>(3, vector<int>(3, 0));
        gridCells = vector<vector<Point>>(3, vector<Point>(3));
        gridSize = 3;
        
        cout << "\n=== NOUVEAU JEU ===" << endl;
        cout << "Dessinez votre grille!" << endl;
    }
    
    void run() {
        namedWindow("Morpion Computer Vision", WINDOW_AUTOSIZE);
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
};

int main() {
    try {
        TicTacToeCV game;
        game.run();
    } catch (const Exception& e) {
        cerr << "Erreur OpenCV: " << e.what() << endl;
        return -1;
    }
    
    return 0;
}