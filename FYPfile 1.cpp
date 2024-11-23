#include <iostream>
#include <windows.h>
#include <conio.h>
#include <cstdlib>
#include <ctime>
using namespace std;

const int ROWS = 10;
const int COLS = 30;

void hideCursor();
void displayGrid(int Row, int Col, int enemyRow, int enemyCol);
void printObstacles(int i, int j);
bool isObstacle(int row, int col);
void spawnEnemy(int& enemyRow, int& enemyCol);
void moveEnemy(int& enemyRow, int& enemyCol, int playerRow, int playerCol);
bool checkCollision(int playerRow, int playerCol, int enemyRow, int enemyCol);

int main() {
    char ch;

    cout << "NOTE: Use (WASD) for Movement!" << endl;
    cout << "Do you want to start the game?" << endl;
    cout << "(y / n) : ";
    cin >> ch;

    if (ch == 'y' || ch == 'Y') {
        int Row = 1, Col = 1;  
        int enemyRow = 5, enemyCol = 5;  
        spawnEnemy(enemyRow, enemyCol);
        hideCursor();

        srand(time(0));

        while (true) {
            cout << "\033[H";  
            displayGrid(Row, Col, enemyRow, enemyCol);

            
            if (_kbhit()) {
                char key = _getch();
                int newRow = Row;
                int newCol = Col;

                switch (key) {
                case 'w':
                    newRow--;  // Move up
                    break;
                case 's':
                    newRow++;  // Move down
                    break;
                case 'a':
                    newCol--;  // Move left
                    break;
                case 'd':
                    newCol++;  // Move right
                    break;
                }

                if (newRow >= 1 && newRow < ROWS - 1 && newCol >= 1 && newCol < COLS - 1 && !isObstacle(newRow, newCol)) {
                    Row = newRow;
                    Col = newCol;
                }
            }

            moveEnemy(enemyRow, enemyCol, Row, Col);

            if (checkCollision(Row, Col, enemyRow, enemyCol)) {
                cout << "YOU LOST LOOSER HEHE" << endl;
                break;  
            }

            Sleep(100);  
        }
    }

    return 0;
}


void hideCursor() {
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(consoleHandle, &cursorInfo);
    cursorInfo.bVisible = false;
    SetConsoleCursorInfo(consoleHandle, &cursorInfo);
}

void displayGrid(int Row, int Col, int enemyRow, int enemyCol) {
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            if (i == Row && j == Col)
                cout << 'P';  // Player
            else if (i == enemyRow && j == enemyCol)
                cout << 'E';  // Enemy
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
    return false;  // No obstacle
}

void spawnEnemy(int& enemyRow, int& enemyCol) {
   
    do {
        enemyRow = rand() % (ROWS - 2) + 1;  
        enemyCol = rand() % (COLS - 2) + 1;  
    } while (isObstacle(enemyRow, enemyCol));  // Ensure enemy is not spawned on an obstacle
}

void moveEnemy(int& enemyRow, int& enemyCol, int playerRow, int playerCol) {
    int moveChoice = rand() % 10;  // Random choice 

    // 70% chance to move towards player, 30% chance to move randomly
    if (moveChoice < 2) {
        
        if (enemyRow < playerRow) {
            enemyRow++;
        }
        else if (enemyRow > playerRow) {
            enemyRow--;
        }

        if (enemyCol < playerCol) {
            enemyCol++;
        }
        else if (enemyCol > playerCol) {
            enemyCol--;
        }
    }
    else {
        
        int direction = rand() % 4;  // Randomly select a direction (0 = up, 1 = down, 2 = left, 3 = right)

        switch (direction) {
        case 0:  // Move up
            enemyRow--;
            break;
        case 1:  // Move down
            enemyRow++;
            break;
        case 2:  // Move left
            enemyCol--;
            break;
        case 3:  // Move right
            enemyCol++;
            break;
        }
    }

    
    if (enemyRow < 1) enemyRow = 1;
    if (enemyRow >= ROWS - 1) enemyRow = ROWS - 2;
    if (enemyCol < 1) enemyCol = 1;
    if (enemyCol >= COLS - 1) enemyCol = COLS - 2;

    
    if (isObstacle(enemyRow, enemyCol)) {
        moveEnemy(enemyRow, enemyCol, playerRow, playerCol);
    }
}

bool checkCollision(int playerRow, int playerCol, int enemyRow, int enemyCol) {
    return playerRow == enemyRow && playerCol == enemyCol;
}



