/*
   Name: Gowtham Prasad
   Email: gdprasad@crimson.ua.edu
   Course Section: CS 581
   Homework #: 3
   Instructions to compile the program: gcc -Wall -o hw3 hw3.c -fopenmp
   Instructions to compile the program with debugging: gcc -Wall -o hw3 hw3.c -fopenmp -DDEBUG
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

// Print the board
void printBoard(int** board, int N) {
    for (int i = 1; i <= N; i++) {
        for (int j = 1; j <= N; j++) {
            printf("%d ", board[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

// Generate the board with random values of 0 and 1 and set the boundaries to 0
void generateBoard(int** board, int N) {
    for (int k = 0; k < N + 2; k++) {
        board[0][k] = board[k][0] = board[N + 1][k] = board[k][N + 1] = DEAD;
    }

    for (int i = 1; i <= N; i++) {  // Initialize the board with random values of 0 and 1
        srand(54321 | i);
        for (int j = 1; j <= N; j++) {
            if (drand48() < 0.5) {
                board[i][j] = ALIVE;
            } else {
                board[i][j] = DEAD;
            }
        }
    }
}

int **allocarray(int P, int Q) {
    int i, *p, **a;

    p = (int *)malloc(P * Q * sizeof(int));
    a = (int **)malloc(P * sizeof(int *));
    for (i = 0; i < P; i++) a[i] = &p[i * Q];

    return a;
}

// Free the memory allocated to the board
void freearray(int **a) {
    free(&a[0][0]);
    free(a);
}

int main(int argc, char** argv) {
    if (argc != 5) {
        printf("Usage: %s <N> <maxium iterations> <number of threads> <output text file directory>\n", argv[0]);
        exit(1);
    }

    int N = atoi(argv[1]), maxGenerations = atoi(argv[2]), numThreads = atoi(argv[3]), i, j, tid, generation;
    bool change;

    int** board = allocarray(N + 2, N + 2);
    int** newBoard = allocarray(N + 2, N + 2);
    int** temp;

    generateBoard(board, N);
    generateBoard(newBoard, N);

    double start = omp_get_wtime();

    // Compute the next generation of the board
    for (generation = 1; generation <= maxGenerations; generation++) {
       change = false;

        // Update the board in parallel using OpenMP threads
        #pragma omp parallel num_threads(numThreads) private(i, j, tid) shared(generation, board, newBoard, change, maxGenerations, N)
        {
            tid = omp_get_thread_num(); // Get the thread ID
            int start = tid * (N / numThreads) + 1; // Calculate the start and end indices for each thread
            int end = (tid + 1) * (N / numThreads); // Calculate the start and end indices for each thread

            // Update the start and end indices for the last thread
            if (tid == numThreads - 1) {
                end += (N % numThreads);
            }

            for (i = start; i <= end; i++) {
                for (j = 1; j <= N; j++) {
                    int liveNeighbors = board[i - 1][j - 1] + board[i - 1][j] + board[i - 1][j + 1] + board[i][j - 1] + board[i][j + 1] + board[i + 1][j - 1] + board[i + 1][j] + board[i + 1][j + 1];
                    if (board[i][j]) {
                        if (liveNeighbors < 2 || liveNeighbors > 3) {
                            newBoard[i][j] = DEAD;
                            change = true;
                        } else {
                            newBoard[i][j] = ALIVE;
                        }
                    } else if (!board[i][j]) {
                        if (liveNeighbors == 3) {
                            newBoard[i][j] = ALIVE;
                            change = true;
                        } else {
                            newBoard[i][j] = DEAD;
                        }
                    }
                }
            }
        }

        #pragma omp barrier 
        {
            if (!change) {
                break;
            }
            temp = board;
            board = newBoard;
            newBoard = temp;
        }
    }

    double end = omp_get_wtime();

    freearray(newBoard);

    // printf("Final board:\n");
    // printBoard(board, N);  // Print the final board

    printf("%d x %d board computed with %d maximum iterations and %d threads:\n", N, N, maxGenerations, numThreads);
    printf("\t\tTime taken: %lf seconds\n\n", end - start);  // Print the time taken
    

    char* outputFileDirectory = (char*)malloc(150 * sizeof(char));
    strcpy(outputFileDirectory, argv[4]);

    strcat(outputFileDirectory, "/output.");
    for (int k = 1; k <= 3; k++) {
        strcat(outputFileDirectory, argv[k]);
        if (k <= 2) {
            strcat(outputFileDirectory, ".");
        }
        else {
            strcat(outputFileDirectory, ".txt");
        }
    }

    FILE* output = fopen(outputFileDirectory, "w");
    for (int k = 1; k <= N; k++) {
        for (int l = 1; l <= N; l++) {
            fprintf(output, "%d ", board[k][l]);
        }
        fprintf(output, "\n");
    }
    fclose(output);
    
    free(outputFileDirectory);
    freearray(board);

    return 0;
}