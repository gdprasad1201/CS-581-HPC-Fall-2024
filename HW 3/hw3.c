/*
   Name: Gowtham Prasad
   Email: gdprasad@crimson.ua.edu
   Course Section: CS 581
   Homework #: 3
   Instructions to compile the program: gcc hw3.c -o hw3
   Instructions to run the program: ./hw3 <size of board> <max iterations> <number of threads> <output file directory>
*/

#include <omp.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

#define ALIVE 1
#define DEAD 0

double gettime() {
    struct timeval tval;

    gettimeofday(&tval, NULL);

    return ((double)tval.tv_sec + (double)tval.tv_usec / 1000000.0);
}


void printBoard(int** board, int N) {
    for (int i = 1; i <= N; i++) {
        for (int j = 1; j <= N; j++) {
            printf("%d ", board[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

void generateBoard(int** board, int N) {
    for (int i = 0; i < N + 2; i++) {
        board[0][i] = board[i][0] = board[N + 1][i] = board[i][N + 1] = DEAD;
    }

    srand(time(NULL));
    for (int i = 1; i <= N; i++) {  // Initialize the board with random values of 0 and 1
        for (int j = 1; j <= N; j++) {
            board[i][j] = rand() % 2;
        }
    }
}

int main(int argc, char** argv) {
    if (argc != 5) {
        printf("Usage: %s <N> <maxGenerations> <numThreads> <outputFile>\n", argv[0]);
        exit(1);
    }

    int N = atoi(argv[1]), maxGenerations = atoi(argv[2]), numThreads = atoi(argv[3]), i, j;
    double timeTaken[3];
    bool change;

    char* outputFileDirectory = malloc(200 * sizeof(char));
    strcpy(outputFileDirectory, argv[4]);

    int** board = malloc((N + 2) * sizeof(int*));
    for (int k = 0; k < N + 2; k++) {
        board[k] = malloc((N + 2) * sizeof(int));
    }

    for (int test = 0; test < 3; test++) {
        generateBoard(board, N);

        // printf("Initial board:\n");
        // printBoard(board, N);  

        double start = gettime();

        // Create a parallel region only once before the iteration loop
        #pragma omp parallel for num_threads(numThreads) default(none) private(i, j) shared(change, board, N, maxGenerations)
        for (int generation = 0; generation < maxGenerations; generation++) {
            change = false;
            int** newBoard = board;

            for (i = 1; i <= N; i++) {
                for (j = 1; j <= N; j++) {
                    int liveNeighbors = board[i - 1][j - 1] + board[i - 1][j] + board[i - 1][j + 1] + board[i][j - 1] + board[i][j + 1] + board[i + 1][j - 1] + board[i + 1][j] + board[i + 1][j + 1];
                    if (board[i][j] == ALIVE) {
                        if (liveNeighbors < 2 || liveNeighbors > 3) {
                            newBoard[i][j] = DEAD;
                            change = true;
                        } else {
                            newBoard[i][j] = ALIVE;
                        }
                    } else if (board[i][j] == DEAD) {
                        if (liveNeighbors == 3) {
                            newBoard[i][j] = ALIVE;
                            change = true;
                        } else {
                            newBoard[i][j] = DEAD;
                        }
                    }
                }
            }

            // After each thread has finished, update the board
            if (!change) {
                maxGenerations = generation;
                continue;
            }

            board = newBoard;
        }

        double end = gettime();

        timeTaken[test] = end - start;

        // printf("Final board:\n");
        // printBoard(board, N);  // Print the final board

        // printf("Time taken: %lf seconds\n", end - start);  // Print the time taken
        printf("Test %d) Time taken: %lf seconds\n", test + 1, timeTaken[test]);  // Print the time taken
    // }

    double averageTime = (timeTaken[0] + timeTaken[1] + timeTaken[2]) / 3;
    printf("Average time taken: %lf seconds\n", averageTime);  // Print the average time taken

    strcat(outputFileDirectory, "/output.");

    for (int i = 1; i <= 3; i++) {
        strcat(outputFileDirectory, argv[i]);
        if (i <= 2) {
            strcat(outputFileDirectory, ".");
        }
        else {
            strcat(outputFileDirectory, ".txt");
        }
    }
    
    FILE* output = fopen(outputFileDirectory, "w");
    fprintf(output, "%d x %d board with %d maximum iterations and %d threads\n", N, N, maxGenerations, numThreads);
    fprintf(output, "\t\t\tTest 1) Time taken: %lf seconds\n", timeTaken[0]);
    fprintf(output, "\t\t\tTest 2) Time taken: %lf seconds\n", timeTaken[1]);
    fprintf(output, "\t\t\tTest 3) Time taken: %lf seconds\n", timeTaken[2]);
    fprintf(output, "\t\t\tAverage time taken: %lf seconds\n", averageTime);
    fclose(output);

    for (int k = 0; k < N + 2; k++) {
        free(board[k]);
    }
    free(board);
    free(outputFileDirectory);

    return 0;
}