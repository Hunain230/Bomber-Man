#include <iostream>
#include <windows.h>
#include <conio.h>
#include <cstdlib>
#include <ctime>
using namespace std;

const int ROWS = 10;
const int COLS = 30;
const int MAX_DESTRUCT_OBSTACLES = 20;
const int BOMB_TIMER = 8; // Time for bomb to explode in seconds
const int EXPLOSION_RANGE = 3; // Bomb explosion range

void hideCursor();
void displayGrid(int Row, int Col, int enemyRow, int enemyCol);
bool isObstacle(int row, int col);
void spawnEnemy(int& enemyRow, int& enemyCol);
void moveEnemy(int& enemyRow, int& enemyCol, int playerRow, int playerCol);
bool checkCollision(int playerRow, int playerCol, int enemyRow, int enemyCol);
void generateDestructibleObstacles();
void plantBomb(int playerRow, int playerCol);
void explodeBomb(int bombRow, int bombCol, int& enemyRow, int& enemyCol);

bool destructibleObstacles[ROWS][COLS] = { false };
bool bombPlanted = false;
int bombRow = -1, bombCol = -1; // Position of the planted bomb
int bombTimer = 0;
bool enemyAlive = true; // Flag to check if the enemy is alive

int main() {
    char ch;

    cout << "Do you want to start the game?" << endl;
    cout << "(y / n) : ";
    cin >> ch;

    if (ch == 'y' || ch == 'Y') {
        int Row = 1, Col = 1;
        int enemyRow = 5, enemyCol = 5;

        spawnEnemy(enemyRow, enemyCol);
        generateDestructibleObstacles();
        hideCursor();

        srand(time(0));

        while (true) {
            cout << "\033[H"; // Clear the screen and reset cursor
            displayGrid(Row, Col, enemyRow, enemyCol);

            if (_kbhit()) {
                char key = _getch();
                int newRow = Row;
                int newCol = Col;

                if (key == -32) {  // Arrow key detection
                    key = _getch();  // Capture the second byte of the arrow key sequence
                    switch (key) {
                    case 72:  // Up arrow
                        newRow--;
                        break;
                    case 80:  // Down arrow
                        newRow++;
                        break;
                    case 75:  // Left arrow
                        newCol--;
                        break;
                    case 77:  // Right arrow
                        newCol++;
                        break;
                    }
                }

                // Prevent the player from moving outside the grid or into fixed obstacles or destructible obstacles
                if (newRow >= 1 && newRow < ROWS - 1 && newCol >= 1 && newCol < COLS - 1 && !isObstacle(newRow, newCol) && !destructibleObstacles[newRow][newCol]) {
                    Row = newRow;
                    Col = newCol;
                }

                // Plant the bomb if 'B' is pressed
                if (key == ' ' && !bombPlanted) {  // Spacebar check
                    plantBomb(Row, Col); // Plant the bomb
                }
            }

            if (enemyAlive) {
                moveEnemy(enemyRow, enemyCol, Row, Col);
                if (checkCollision(Row, Col, enemyRow, enemyCol)) {
                    cout << "YOU LOST LOOSER HEHE" << endl;
                    break;
                }
            }

            if (bombPlanted) {
                bombTimer++;
                if (bombTimer >= BOMB_TIMER) {
                    explodeBomb(bombRow, bombCol, enemyRow, enemyCol);
                    bombPlanted = false; // Bomb exploded
                    bombTimer = 0; // Reset timer
                }
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
            else if (i == enemyRow && j == enemyCol && enemyAlive)
                cout << 'E';  // Enemy
            else if (isObstacle(i, j))
                cout << '*';  // Fixed or destructible obstacle
            else if (destructibleObstacles[i][j])
                cout << "D"; // Destructible obstacle
            else if (i == 0)
                cout << '_';
            else if (i == ROWS - 1)
                cout << '-';
            else if (j == 0 || j == COLS - 1)
                cout << '|';
            else if (bombPlanted && i == bombRow && j == bombCol)
                cout << "B"; // Display Bomb
            else
                cout << ' ';
        }
        cout << endl;
    }
}

bool isObstacle(int row, int col) {
    if (row == 0 || col == 0) {
        return false; // Ensure no obstacles on the first row
    }
    if (row % 2 == 0 && col % 2 == 0 && ((row / 2 + col / 2) % 2 == 0)) {
        return true; // Fixed obstacles
    }
    return false;  // No obstacle
}

void spawnEnemy(int& enemyRow, int& enemyCol) {
    do {
        enemyRow = rand() % (ROWS - 2) + 1;
        enemyCol = rand() % (COLS - 2) + 1;
    } while (isObstacle(enemyRow, enemyCol) || destructibleObstacles[enemyRow][enemyCol]);  // Ensure no overlap
}

void moveEnemy(int& enemyRow, int& enemyCol, int playerRow, int playerCol) {
    if (!enemyAlive) return;  // If the enemy is dead, skip moving

    static int moveCounter = 0; // Static counter to track moves
    const int moveThreshold = 5; // Adjust this value to slow down movement (higher = slower)

    moveCounter++;
    if (moveCounter < moveThreshold) {
        return; // Skip moving the enemy until the counter reaches the threshold
    }

    moveCounter = 0; // Reset counter after moving

    int moveChoice = rand() % 10;
    int newRow = enemyRow;
    int newCol = enemyCol;

    if (moveChoice < 2) {
        // Move towards player
        if (enemyRow < playerRow) {
            newRow++;
        }
        else if (enemyRow > playerRow) {
            newRow--;
        }

        if (enemyCol < playerCol) {
            newCol++;
        }
        else if (enemyCol > playerCol) {
            newCol--;
        }
    }
    else {
        // Random movement in one of the four directions
        int direction = rand() % 4; // Randomly select a direction (0 = up, 1 = down, 2 = left, 3 = right)

        switch (direction) {
        case 0:  // Move up
            newRow--;
            break;
        case 1:  // Move down
            newRow++;
            break;
        case 2:  // Move left
            newCol--;
            break;
        case 3:  // Move right
            newCol++;
            break;
        }
    }

    // Check if the new position is valid (not out of bounds and not an obstacle)
    if (newRow >= 1 && newRow < ROWS - 1 && newCol >= 1 && newCol < COLS - 1 &&
        !isObstacle(newRow, newCol) && !destructibleObstacles[newRow][newCol]) {
        enemyRow = newRow;
        enemyCol = newCol;
    }
    else {
        // If blocked, don't update position (enemy stays in the same place)
        moveEnemy(enemyRow, enemyCol, playerRow, playerCol); // Recurse to try again
    }
}


bool checkCollision(int playerRow, int playerCol, int enemyRow, int enemyCol) {
    return playerRow == enemyRow && playerCol == enemyCol;
}

void generateDestructibleObstacles() {
    int count = 0;
    while (count < MAX_DESTRUCT_OBSTACLES) {
        int r = rand() % (ROWS - 2) + 1;
        int c = rand() % (COLS - 2) + 1;

        // Ensure no overlap with fixed obstacles or initial positions
        if (!isObstacle(r, c) && !destructibleObstacles[r][c] && !(r == 1 && c == 1)) {
            destructibleObstacles[r][c] = true;
            count++;
        }
    }
}

void plantBomb(int playerRow, int playerCol) {
    bombPlanted = true;
    bombRow = playerRow;
    bombCol = playerCol;
    cout << "Bomb planted at (" << bombRow << ", " << bombCol << ")" << endl;
}

void explodeBomb(int bombRow, int bombCol, int& enemyRow, int& enemyCol) {
    cout << "Boom! The bomb exploded!" << endl;

    // Check the explosion range and affect obstacles
    for (int i = -EXPLOSION_RANGE; i <= EXPLOSION_RANGE; i++) {
        for (int j = -EXPLOSION_RANGE; j <= EXPLOSION_RANGE; j++) {
            int r = bombRow + i;
            int c = bombCol + j;

            // Check if the position is within bounds and destroy destructible obstacles
            if (r >= 1 && r < ROWS - 1 && c >= 1 && c < COLS - 1) {
                if (destructibleObstacles[r][c]) {
                    destructibleObstacles[r][c] = false;  // Destroy destructible obstacles
                }
            }

            // Check if the enemy is within the explosion range
            if (r == enemyRow && c == enemyCol) {
                cout << "The enemy was caught in the explosion and has been destroyed!" << endl;
                enemyAlive = false; // Mark enemy as dead
                enemyRow = -1;
                enemyCol = -1;
            }
        }
    }
}
