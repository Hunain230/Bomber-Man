#include <iostream>
#include <windows.h>
#include <conio.h>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
using namespace std;

const int ROWS = 16;
const int COLS = 32;
const int MAX_DESTRUCT_OBSTACLES = 40;
const int BOMB_TIMER = 14;
const int EXPLOSION_RANGE = 2;
int powerUpDuration = 0;
int bombRadius = EXPLOSION_RANGE; // Initial radius from the constant
const string SCORES_FILE = "Text.txt";

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

bool powerUpPresent[ROWS][COLS] = { false };  // Power-up presence on the grid

void hideCursor();
int displayMenu();
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
void generatePowerUps();
void collectPowerUp(int playerRow, int playerCol);

int main() {
    char ch;
    int menuChoice = displayMenu();

    if (menuChoice == 3) {
        cout << "Exiting the game. Goodbye!\n";
        return 0;  // Exit the program
    }

    // Start the game
    cout << "Starting the game...\n";

    int Row = 1, Col = 1;

    transitionToStage(1);
    spawnEnemies();
    generateDestructibleObstacles();
    generatePowerUps();  // Generate power-ups
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
            }

            if (key == ' ' && !bombPlanted) {
                plantBomb(Row, Col);
            }
        }

        // Collect Power-up
        collectPowerUp(Row, Col);

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
                generatePowerUps();  // Generate power-ups for the next stage
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

    return 0;
}

void generatePowerUps() {
    // Generate a few power-ups randomly on the grid
    int count = 0;
    while (count < 3) {  // Limit the number of power-ups
        int r = rand() % (ROWS - 2) + 1;
        int c = rand() % (COLS - 2) + 1;

        if (!isObstacle(r, c) && !destructibleObstacles[r][c] && !powerUpPresent[r][c]) {
            powerUpPresent[r][c] = true;  // Mark this cell as a power-up
            count++;
        }
    }
}

void collectPowerUp(int playerRow, int playerCol) {
    if (powerUpPresent[playerRow][playerCol]) {
        powerUpPresent[playerRow][playerCol] = false;  // Remove the power-up from the grid
        bombRadius = 3;  // Increase explosion range when power-up is collected
        powerUpDuration = 2;  // Power-up lasts for the next two bombs
        cout << "Power-up collected! Bomb radius increased!" << endl;
    }
}
void displayGrid(int Row, int Col, int enemyRow[], int enemyCol[]) {
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            if (i == Row && j == Col) {
                // Highlight player in green
                cout << "\033[32mP\033[0m";
            }
            else {
                bool enemyDisplayed = false;
                for (int k = 0; k < currentMaxEnemies; k++) {
                    if (i == enemyRow[k] && j == enemyCol[k] && enemyAlive[k]) {
                        // Highlight enemy in red
                        cout << "\033[31mE\033[0m";
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
                        cout << "\033[33mB\033[0m"; // bomb
                    else if (powerUpPresent[i][j])  // Display power-up
                        cout << "\033[34m+\033[0m";  // Power-up in blue
                    else
                        cout << ' ';
                }
            }
        }
        cout << endl;
    }

    // Display the score below the grid
    for (int i = 0; i < COLS; i++) cout << "=";
    cout << endl << "Score: " << playerScore << endl;
}
void hideCursor() {
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(consoleHandle, &cursorInfo);
    cursorInfo.bVisible = false;
    SetConsoleCursorInfo(consoleHandle, &cursorInfo);
}

int displayMenu() {
    int choice;
    do {
        system("cls"); // Clear screen before showing the menu
        cout << "=====================\n";
        cout << "  BOMBERMAN MENU\n";
        cout << "=====================\n";
        cout << "1. Start Game\n";
        cout << "2. View Instructions\n";
        cout << "3. Exit\n";
        cout << "=====================\n";
        cout << "Enter your choice (1-3): ";
        cin >> choice;

        if (choice == 2) {
            system("cls"); // Clear screen before showing instructions
            cout << "\n========== INSTRUCTIONS ==========\n";
            cout << "1. Use arrow keys to move the player (P).\n";
            cout << "2. Plant bombs (B) to eliminate enemies (E).\n";
            cout << "3. Avoid contact with enemies and obstacles.\n";
            cout << "4. Reach the goal to win!\n";
            cout << "===================================\n\n";
            system("pause"); // Wait for the user to read instructions
        }
        else if (choice < 1 || choice > 3) {
            system("cls"); // Clear screen for invalid input
            cout << "Invalid choice! Please select 1, 2, or 3.\n";
        }
    } while (choice != 1 && choice != 3);

    system("cls"); // Clear screen before starting the game
    return choice;
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

    if (powerUpDuration > 0) {
        // Apply increased bomb radius while the power-up is active
        cout << "Bomb planted! Bomb radius increased to " << bombRadius << ". "<<
          endl << powerUpDuration << " bomb(s) left with increased radius." << endl;
        powerUpDuration--;  // Decrease the power-up duration after each bomb
    }
    else {
        // Default bomb radius after the power-up duration ends
        bombRadius = EXPLOSION_RANGE;
        cout << endl;
        cout << "Bomb planted! Bomb radius: " << bombRadius << "." << endl;
    }

    // If the power-up duration ends, reset bomb radius to the default value
    if (powerUpDuration == 0) {
        bombRadius = EXPLOSION_RANGE;  // Reset bomb radius to the default value
    }
}

void explodeBomb(int bombRow, int bombCol, int playerRow, int playerCol) {
    PlaySound(TEXT("bomb_explode.wav"), NULL, SND_FILENAME | SND_ASYNC);
    for (int i = -bombRadius; i <= bombRadius; i++) {
        for (int j = -bombRadius; j <= bombRadius; j++) {
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
    system("cls"); // Clear the screen
    cout << "Transitioning to Stage " << stage << "..." << endl;
    currentMaxEnemies = (stage == 2) ? MAX_ENEMIES_STAGE2 : MAX_ENEMIES_STAGE1;
    Sleep(2000);  // Simulate transition time
}
