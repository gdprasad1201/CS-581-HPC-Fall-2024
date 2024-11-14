#ifndef BOARD
#define BOARD

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>

#define ROOT 0
#define ALIVE 1
#define DEAD 0

int N, maxIterations;
char directory[100];

void generateBoard(int* board) {
    // Use a fixed seed for reproducibility
    srand(0);
    for (int row = 0; row < N; row++) {
        for (int col = 0; col < N; col++) {
            board[row * N + col] = rand() % 2;
        }
    }
}

void printBoard(int* board, int localRows, int comm_size) {
    FILE* finalBoardFIle = fopen(directory, "w");
    if (finalBoardFIle == NULL) {
        printf("Error: could not open file\n");
        return;
    }

    for (int row = 0; row < localRows; row++) {
        for (int col = 0; col < N; col++) {
            fprintf(finalBoardFIle, "%d ", board[row * N + col]);
        }
        fprintf(finalBoardFIle, "\n");
    }

    fclose(finalBoardFIle);
}

int getTopNeighbor(int rank) {
    if (rank == ROOT) {
        return MPI_PROC_NULL;
    } 
    else {
        return (rank - 1);
    }
}

int getBottomNeighbor(int rank, int size) {
    if (rank == (size - 1)) {
        return MPI_PROC_NULL;
    } 
    else {
        return (rank + 1);
    }
}

int countLiveNeighbors(int* board, int row, int col, int localRows, int rank, int size) {
    int neighbors = 0, top = 1, bottom = 1;

    // Very top row of the board has no top neighbor
    if (rank == ROOT && row == 1) {
        top = 0;
    }

    // Very bottom row of the board has no bottom neighbor
    if (rank == (size - 1) && row == localRows) {
        bottom = 0;
    } 

    // check all 8 neighbors
    if (top && ((col - 1) >= 0) && board[(row - 1) * N + (col - 1)]) {
        neighbors++;
    }

    if (top && board[(row - 1) * N + col]) {
        neighbors++;
    }

    if (top && ((col + 1) < N) && board[(row - 1) * N + (col + 1)]) {
        neighbors++;
    }

    if (((col + 1) < N) && board[row * N + (col + 1)]) {
        neighbors++;
    }
    
    if (bottom && ((col + 1) < N) && board[(row + 1) * N + (col + 1)]) {
        neighbors++;
    }

    if (bottom && board[(row + 1) * N + col]) {
        neighbors++;
    }

    if (bottom && ((col - 1) >= 0) && board[(row + 1) * N + (col - 1)]) {
        neighbors++;
    }

    if (((col - 1) >= 0) && board[row * N + (col - 1)]) {
        neighbors++;
    }

    return neighbors;
}

bool updateBoard(int* board, int localRows, int rank, int size) {
    int* newBoard = (int *)malloc(localRows * N * sizeof(int));
    bool flag = false;

    for (int row = 1; row < localRows + 1; row++) {
        for (int col = 0; col < N; col++) {
            int neighbors = countLiveNeighbors(board, row, col, localRows, rank, size);

            if (board[row * N + col]) {
                if (neighbors < 2 || neighbors > 3) {
                    newBoard[(row - 1) * N + col] = DEAD;
                    flag = true;
                }
                else {
                    newBoard[(row - 1) * N + col] = ALIVE;
                }
            }
            else {
                if (neighbors == 3) {
                    newBoard[(row - 1) * N + col] = ALIVE;
                    flag = true;
                }
                else {
                    newBoard[(row - 1) * N + col] = DEAD;
                }
            }
        }
    }

    if (flag) {
        for (int row = 1; row < localRows + 1; row++) {
            for (int col = 0; col < N; col++) {
                board[row * N + col] = newBoard[(row - 1) * N + col];
            }
        }
    }

    free(newBoard);

    return flag;
}

#endif