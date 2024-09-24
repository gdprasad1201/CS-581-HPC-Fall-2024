#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/time.h>
#include <time.h>

double gettime() {
    struct timeval tval;

    gettimeofday(&tval, NULL);

    return ((double)tval.tv_sec + (double)tval.tv_usec / 1000000.0);
}

// Function to print the board
void printBoard(int** board, int N) {
    for (int i = 1; i <= N; i++) {
        for (int j = 1; j <= N; j++) {
            if (board[i][j]) {
                printf(" %d ", board[i][j]);
            } else {
                printf(" %d ", board[i][j]);
            }
        }
        printf("\n");
    }
}

void generateBoard(int** board, int N) {
    for (int i = 0; i < N + 2; i++) {
        board[0][i] = board[i][0] = board[N + 1][i] = board[i][N + 1] = 0;
    }

    srand(time(0));
    for (int i = 1; i <= N; i++) {  // Initialize the board with random values of 0 and 1
        for (int j = 1; j <= N; j++) {
            board[i][j] = rand() % 2;
        }
    }
}

// Function to count the number of live neighbors
int countLiveNeighbors(int** board, int i, int j) {
    return board[i - 1][j - 1] + board[i - 1][j] + board[i - 1][j + 1] + board[i][j - 1] + board[i][j + 1] + board[i + 1][j - 1] + board[i + 1][j] + board[i + 1][j + 1];
}

// Function to simulate the game of board
void gameOfLife(int** board, int N, int maxGenerations) {
    int** newBoard = board;  // Create a new board to store the next generation
    int generation = 0;      // Generation counter

    while (generation < maxGenerations) {  // Run the simulation for the specified number of generations
        bool change = false;

        for (int i = 1; i <= N; i++) {
            for (int j = 1; j <= N; j++) {
                int liveNeighbors = countLiveNeighbors(newBoard, i, j);
                if (board[i][j]) {
                    if (liveNeighbors < 2 || liveNeighbors > 3) {
                        newBoard[i][j] = 0;
                        change = true;
                    } else {
                        newBoard[i][j] = 1;
                    }
                } else if (!board[i][j]) {
                    if (liveNeighbors == 3) {
                        newBoard[i][j] = 1;
                        change = true;
                    } else {
                        newBoard[i][j] = 0;
                    }
                }
            }
        }

        if (!change) {
            break;
        }

        board = newBoard;  // Update the board with the new generation
        generation++;  // Increment the generation counter
    }

    board = newBoard;
}

int main(int argc, char **argv) {
    if (argc != 3) {
        printf("Usage: %s <size of board> <max generations>\n", argv[0]);
        return 1;
    }

    int N = atoi(argv[1]), maxGenerations = atoi(argv[2]);     // Size of the board and maximum number of generations
    double timeTaken[3];  // Array to store the time taken for each run

    int** board = malloc((N + 2) * sizeof(int*));
    for (int i = 0; i < N + 2; i++) {
        board[i] = malloc((N + 2) * sizeof(int));
    }

    for (int i = 0; i < 3; i++) { // Run the simulation 3 times
        generateBoard(board, N);

        double start = gettime(); // Start the timer
        gameOfLife(board, N, maxGenerations); // Run the game of board
        double end = gettime(); // End the timer

        timeTaken[i] = end - start; // Calculate the time taken for the run
    }

    printf("Time taken: %f seconds\n", (timeTaken[0] + timeTaken[1] + timeTaken[2]) / 3); // Print the average time taken for the 3 runs

    for (int i = 0; i < N + 2; i++) {
        free(board[i]);
    }
    free(board);

    return 0;
}