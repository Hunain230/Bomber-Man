#include <iostream>
#include <windows.h>
#include <conio.h>
using namespace std;

const int ROWS = 10;
const int COLS = 30;

void hideCursor();
void displayGrid(int Row, int Col);
void printObstacles(int i, int j);
bool isObstacle(int row, int col);

int main() {
    char ch;
    cout << "NOTE: Use (WASD) for Movement!" << endl;
    cout << "Do you want to start the game?" << endl;
    cout << "(y / n) : ";
    cin >> ch;

    if (ch == 'y' || ch == 'Y') {
        int Row = 1, Col = 1;

        hideCursor();

        while (true) {
            cout << "\033[H";
            displayGrid(Row, Col);

            char key = _getch();

            if (key == 27) // escape key 
                break;

            int newRow = Row;
            int newCol = Col;

           
            switch (key) {
            case 'w':
                newRow--; //  up
                break;
            case 's':
                newRow++; // down
                break;
            case 'a':
                newCol--; // left
                break;
            case 'd':
                newCol++; // right
                break;
            }

           
            if (newRow >= 1 && newRow < ROWS - 1 && newCol >= 1 && newCol < COLS - 1 && !isObstacle(newRow, newCol)) {
                Row = newRow;  
                Col = newCol;
            }

            Sleep(10);
        }

        return 0;
    }
}

void hideCursor() {
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(consoleHandle, &cursorInfo);
    cursorInfo.bVisible = false;
    SetConsoleCursorInfo(consoleHandle, &cursorInfo);
}

void displayGrid(int Row, int Col) {
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            if (i == Row && j == Col)
                cout << 'P';  // Player
            else if (i == 0)
                cout << '_';
            else if (i == ROWS - 1)
                cout << '-';
            else if (j == 0 || j == COLS - 1)
                cout << '|';
            else {
                printObstacles(i, j);
            }
        }
        cout << endl;
    }
}

void printObstacles(int i, int j) {
    if (i % 2 == 0 && j % 2 == 0 && ((i / 2 + j / 2) % 2 == 0))
        cout << '*';  // Obstacle
    else
        cout << ' ';  // Empty space
}

bool isObstacle(int row, int col) {
   
    if (row % 2 == 0 && col % 2 == 0 && ((row / 2 + col / 2) % 2 == 0)) {
        return true;  
    }
    return false;  
}
