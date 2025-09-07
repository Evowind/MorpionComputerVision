#include "tictactoe_cv.h"
#include <iostream>

using namespace std;

int main() {
    try {
        TicTacToeCV game;
        game.run();
    } catch (const cv::Exception& e) {
        cerr << "Erreur OpenCV: " << e.what() << endl;
        return -1;
    }
    
    return 0;
}