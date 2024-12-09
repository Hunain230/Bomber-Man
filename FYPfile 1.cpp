#include <iostream>
#include <windows.h>
#include <conio.h>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
using namespace std;

const int ROWS = 14;
const int COLS = 28;
const int MAX_DESTRUCT_OBSTACLES = 40;
const int BOMB_TIMER = 14;
const int EXPLOSION_RANGE = 2;
const string SCORES_FILE = ".high_scores.txt";

const int MAX_ENEMIES_STAGE1 = 2;
const int MAX_ENEMIES_STAGE2 = 5;  // Max enemies for stage 2 can be 5
int currentMaxEnemies = MAX_ENEMIES_STAGE1;

int enemyRows[5] = { -1, -1, -1, -1, -1 };
int enemyCols[5] = { -1, -1, -1, -1, -1 };
bool enemyAlive[5] = { true, true, true, true, true };

bool destructibleObstacles[ROWS][COLS] = { false };
bool bombPlanted = false;
int bombRow = -1, bombCol = -1;
int bombTimer = 0;
int playerScore = 0;

void hideCursor();
void displayGrid(int Row, int Col, int enemyRow[], int enemyCol[]);
bool isObstacle(int row, int col);
void spawnEnemies();
void moveEnemies(int playerRow, int playerCol);
bool checkCollision(int playerRow, int playerCol);
void generateDestructibleObstacles();
void plantBomb(int playerRow, int playerCol);
void explodeBomb(int bombRow, int bombCol, int playerRow, int playerCol);
void readHighScores(int scores[], int& count);
void saveHighScore(int score);
void displayHighScores();
void transitionToStage(int stage);

int main() {
    char ch;

    cout << "Do you want to start the game?" << endl;
    cout << "(y / n): ";
    cin >> ch;

    if (ch == 'y' || ch == 'Y') {
        int Row = 1, Col = 1;

        transitionToStage(1);
        spawnEnemies();
        generateDestructibleObstacles();
        hideCursor();

        srand(time(0));

        while (true) {
            cout << "\033[H";  // Refresh the grid
            displayGrid(Row, Col, enemyRows, enemyCols);

            // Player movement and bomb planting
            if (_kbhit()) {
                char key = _getch();
                int newRow = Row, newCol = Col;

                if (key == -32) {
                    key = _getch();
                    switch (key) {
                    case 72: newRow--; break;  // Up
                    case 80: newRow++; break;  // Down
                    case 75: newCol--; break;  // Left
                    case 77: newCol++; break;  // Right
                    }
                }

                if (newRow >= 1 && newRow < ROWS - 1 && newCol >= 1 && newCol < COLS - 1 &&
                    !isObstacle(newRow, newCol) && !destructibleObstacles[newRow][newCol]) {
                    Row = newRow;
                    Col = newCol;
                    playerScore++;
                }

                if (key == ' ' && !bombPlanted) {
                    plantBomb(Row, Col);
                }
            }

            // Move enemies
            moveEnemies(Row, Col);

            // Check for collision with enemies
            if (checkCollision(Row, Col)) {
                cout << "YOU LOST!" << endl;
                saveHighScore(playerScore);
                displayHighScores();
                Sleep(3000);
                break;
            }

            // Bomb explosion logic
            if (bombPlanted) {
                bombTimer++;
                if (bombTimer >= BOMB_TIMER) {
                    explodeBomb(bombRow, bombCol, Row, Col);
                    bombPlanted = false;
                    bombTimer = 0;
                }
            }

            // Check if all enemies are defeated to move to the next stage
            bool allEnemiesDefeated = true;
            for (int k = 0; k < currentMaxEnemies; k++) {
                if (enemyAlive[k]) {
                    allEnemiesDefeated = false;
                    break;
                }
            }

            if (allEnemiesDefeated) {
                if (currentMaxEnemies == MAX_ENEMIES_STAGE1) {
                    cout << "Stage 1 Complete! Moving to Stage 2!" << endl;
                    transitionToStage(2);
                    Row = 1; Col = 1;
                    spawnEnemies();
                    generateDestructibleObstacles();
                }
                else {
                    cout << "Congratulations! You completed the game!" << endl;
                    saveHighScore(playerScore);
                    displayHighScores();
                    Sleep(3000);
                    break;
                }
            }

            Sleep(100);  // Game speed control
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

void displayGrid(int Row, int Col, int enemyRow[], int enemyCol[]) {
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            if (i == Row && j == Col)
                cout << 'P';  // Player
            else {
                bool enemyDisplayed = false;
                for (int k = 0; k < currentMaxEnemies; k++) {
                    if (i == enemyRow[k] && j == enemyCol[k] && enemyAlive[k]) {
                        cout << 'E';  // Enemy
                        enemyDisplayed = true;
                        break;
                    }
                }
                if (!enemyDisplayed) {
                    if (isObstacle(i, j))
                        cout << '*';  // Fixed or destructible obstacle
                    else if (destructibleObstacles[i][j])
                        cout << 'D'; // Destructible obstacle
                    else if (i == 0)
                        cout << '_';  // Top boundary
                    else if (i == ROWS - 1)
                        cout << '-';  // Bottom boundary
                    else if (j == 0 || j == COLS - 1)
                        cout << '|';  // Left and right boundaries
                    else if (bombPlanted && i == bombRow && j == bombCol)
                        cout << 'B'; // Display Bomb
                    else
                        cout << ' ';
                }
            }
        }
        cout << endl;
    }

    // Display the score below the grid
    for (int j = 0; j < COLS; j++) cout << '-';  // Separator line
    cout << endl;
    cout << "Score: " << playerScore << endl;
}


bool isObstacle(int row, int col) {
    if (row == 0 || col == 0) return false;  // No obstacles in the first row/column
    return row % 2 == 0 && col % 2 == 0;
}

void spawnEnemies() {
    // Dynamically spawn the enemies based on currentMaxEnemies for the stage
    for (int k = 0; k < currentMaxEnemies; k++) {
        do {
            enemyRows[k] = rand() % (ROWS - 2) + 1;
            enemyCols[k] = rand() % (COLS - 2) + 1;
        } while (isObstacle(enemyRows[k], enemyCols[k]) || destructibleObstacles[enemyRows[k]][enemyCols[k]]);
        enemyAlive[k] = true;
    }
}

void moveEnemies(int playerRow, int playerCol) {
    for (int k = 0; k < currentMaxEnemies; k++) {
        if (!enemyAlive[k]) continue;

        int moveChoice = rand() % 4;
        int newRow = enemyRows[k];
        int newCol = enemyCols[k];

        if (moveChoice == 0) newRow--;
        else if (moveChoice == 1) newRow++;
        else if (moveChoice == 2) newCol--;
        else if (moveChoice == 3) newCol++;

        if (newRow >= 1 && newRow < ROWS - 1 && newCol >= 1 && newCol < COLS - 1 &&
            !isObstacle(newRow, newCol) && !destructibleObstacles[newRow][newCol]) {
            enemyRows[k] = newRow;
            enemyCols[k] = newCol;
        }
    }
}

bool checkCollision(int playerRow, int playerCol) {
    for (int k = 0; k < currentMaxEnemies; k++) {
        if (enemyAlive[k] && playerRow == enemyRows[k] && playerCol == enemyCols[k]) {
            return true;
        }
    }
    return false;
}

void generateDestructibleObstacles() {
    int count = 0;
    while (count < MAX_DESTRUCT_OBSTACLES) {
        int r = rand() % (ROWS - 2) + 1;
        int c = rand() % (COLS - 2) + 1;

        if (!isObstacle(r, c) && !destructibleObstacles[r][c]) {
            destructibleObstacles[r][c] = true;
            count++;
        }
    }
}

void plantBomb(int playerRow, int playerCol) {
    bombPlanted = true;
    bombRow = playerRow;
    bombCol = playerCol;
    PlaySound(TEXT("bomb_plant.wav"), NULL, SND_FILENAME | SND_ASYNC);
}

void explodeBomb(int bombRow, int bombCol, int playerRow, int playerCol) {
    PlaySound(TEXT("bomb_explode.wav"), NULL, SND_FILENAME | SND_ASYNC);
    for (int i = -EXPLOSION_RANGE; i <= EXPLOSION_RANGE; i++) {
        for (int j = -EXPLOSION_RANGE; j <= EXPLOSION_RANGE; j++) {
            int targetRow = bombRow + i;
            int targetCol = bombCol + j;

            if (targetRow >= 1 && targetRow < ROWS - 1 && targetCol >= 1 && targetCol < COLS - 1) {
                // Check if the player is in the explosion range
                if (targetRow == playerRow && targetCol == playerCol) {
                    cout << "YOU LOST!" << endl;
                    saveHighScore(playerScore);
                    displayHighScores();
                    Sleep(3000);
                    exit(0);
                }

                // Check if there is a destructible obstacle
                if (destructibleObstacles[targetRow][targetCol]) {
                    destructibleObstacles[targetRow][targetCol] = false;
                    playerScore += 10;  // Add 10 points for destroying the obstacle
                }

                // Check if there is an enemy
                for (int k = 0; k < currentMaxEnemies; k++) {
                    if (enemyAlive[k] && targetRow == enemyRows[k] && targetCol == enemyCols[k]) {
                        enemyAlive[k] = false;  // Enemy is eliminated
                        playerScore += 50;     // Add 50 points for eliminating the enemy
                    }
                }
            }
        }
    }
}


void readHighScores(int scores[], int& count) {
    ifstream inputFile(SCORES_FILE);
    while (inputFile >> scores[count]) {
        count++;
    }
    inputFile.close();
}

void saveHighScore(int score) {
    int scores[10];
    int count = 0;
    readHighScores(scores, count);

    scores[count] = score;
    count++;

    // Sort scores
    for (int i = 0; i < count - 1; i++) {
        for (int j = i + 1; j < count; j++) {
            if (scores[i] < scores[j]) {
                swap(scores[i], scores[j]);
            }
        }
    }

    // Save top 3 scores
    ofstream outputFile(SCORES_FILE);
    for (int i = 0; i < 3 && i < count; i++) {
        outputFile << scores[i] << endl;
    }
    outputFile.close();
}

void displayHighScores() {
    int scores[10];
    int count = 0;
    readHighScores(scores, count);

    cout << "High Scores: " << endl;
    for (int i = 0; i < count; i++) {
        cout << scores[i] << endl;
    }
}

void transitionToStage(int stage) {
    cout << "Transitioning to Stage " << stage << "..." << endl;
    currentMaxEnemies = (stage == 2) ? MAX_ENEMIES_STAGE2 : MAX_ENEMIES_STAGE1;
    Sleep(2000);  // Simulate transition time
}
