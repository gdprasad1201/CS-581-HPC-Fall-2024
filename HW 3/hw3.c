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

int** allocarray(int P, int Q) {
    int i, *p, **a;

    p = (int *)malloc(P * Q * sizeof(int));
    a = (int **)malloc(P * sizeof(int *));
    for (i = 0; i < P; i++) a[i] = &p[i * Q];

    return a;
}

// Free the memory allocated to the board
void freearray(int** a) {
    free(&a[0][0]);
    free(a);
}

void swap(int*** a, int*** b) {
    int** temp = *a;
    *a = *b;
    *b = temp;
}

int main(int argc, char** argv) {
    if (argc != 5) {
        printf("Usage: %s <N> <maxium iterations> <number of threads> <output text file directory>\n", argv[0]);
        exit(1);
    }

    int N = atoi(argv[1]), maxGenerations = atoi(argv[2]), numThreads = atoi(argv[3]), i, j, generation, recordGen = 0;
    bool change;

    int** board = allocarray(N + 2, N + 2);
    int** newBoard = allocarray(N + 2, N + 2);

    generateBoard(board, N);
    generateBoard(newBoard, N);

    double start = omp_get_wtime();

    // Update the board in parallel using OpenMP threads
    #pragma omp parallel default(none) shared(recordGen, change, numThreads, board, newBoard, maxGenerations, N) private(i, j) num_threads(numThreads) reduction(+:generation)
    {
        int tid = omp_get_thread_num();
        int nthreads = omp_get_num_threads();
        int iStart = (N / nthreads) * tid + 1;
        int iEnd = (N / nthreads) * (tid + 1);

        if (tid == nthreads - 1) {
            iEnd = N;
        }

        #ifdef DEBUG
            printf("tid=%d iStart=%d iEnd=%d\n", tid, iStart, iEnd);
        #endif

        for (generation = 0; generation < maxGenerations; generation+=1) {
            bool localChange = false;
            change = false;

            for (i = iStart; i <= iEnd; i++) {
                for (j = 1; j <= N; j++) {
                    int liveNeighbors = board[i - 1][j - 1] + board[i - 1][j] + board[i - 1][j + 1] + board[i][j - 1] + board[i][j + 1] + board[i + 1][j - 1] + board[i + 1][j] + board[i + 1][j + 1];
                    if (board[i][j]) {
                        if (liveNeighbors < 2 || liveNeighbors > 3) {
                            newBoard[i][j] = DEAD;
                            localChange = true;
                        } else {
                            newBoard[i][j] = ALIVE;
                        }
                    } else if (!board[i][j]) {
                        if (liveNeighbors == 3) {
                            newBoard[i][j] = ALIVE;
                            localChange = true;
                        } else {
                            newBoard[i][j] = DEAD;
                        }
                    }
                }
            }
            
            #pragma omp critical
            {
                // if (localChange) {
                //     change = true;
                // }
               change = change || localChange;
            }

            #pragma omp barrier 
            
            #pragma omp single
            {
                if (!change) {
                    recordGen = generation;
                    generation = maxGenerations;
                }

                swap(&board, &newBoard);
                #ifdef DEBUG
                    if (generation != maxGenerations) {
                        printf("Generation %d:\n", generation);
                        printBoard(board, N); 
                    }
                    
                #endif
            }   
        }
    }
    

    double end = omp_get_wtime();

    freearray(newBoard);
    #ifdef DEBUG
        printf("Final board:\n");
        printBoard(board, N);  // Print the final board

        if (recordGen) {
            printf("The Game of Life ends at iteration %d\n", recordGen);
        }
        else {
            printf("The Game of Life has reached the maximum number of iterations of %d.\n", atoi(argv[2]));
        }
    #endif
        
    printf("%d x %d board computed with %d maximum iterations and %d threads:\n", N, N, maxGenerations, numThreads);
    printf("\t\tTime taken: %lf seconds\n\n", end - start);  // Print the time taken
    

    char outputFileDirectory[150];
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
    
    freearray(board);

    return 0;
}