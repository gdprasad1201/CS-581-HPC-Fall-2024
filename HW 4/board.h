#ifndef BOARD_H
#define BOARD_H

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
    for (int row = 0; row < N; row++) {
        srand(54321 | (row + 1));
        for (int col = 0; col < N; col++) {
            if (drand48() < 0.5) {
                board[row * N + col] = ALIVE;
            }
            else {
                board[row * N + col] = DEAD;
            }
        }
    }
}

void printBoard(int* board, int localRows) {
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
    // a top neighbor doesn't exist
    if (rank == ROOT) {
        return MPI_PROC_NULL;
    } 
    else {
        return rank - 1;
    }
}

int getBottomNeighbor(int rank, int size) {
    // a bottom neighbor doesn't exist
    if (rank == size - 1) {
        return MPI_PROC_NULL;
    } 
    else {
        return rank + 1;
    }
}

int countLiveNeighbors(int* board, int row, int col, int localRows, int rank, int size) {
    // edges of the board
    int top = 1;

    // very top row of the board doesn't have a top neighbor
    if (rank == ROOT && row == 1) {
        top = 0;
    }

    int bottom = 1;

    // very bottom row of the board doesn't have a bottom neighbor
    if (rank == size - 1 && row == localRows) {
        bottom = 0;
    }

    // check that the cell position exists and then check if the cell is alive or dead

    return (top && col - 1 >= 0 && board[(row - 1) * N + (col - 1)]) +
           (top && board[(row - 1) * N + col]) +
           (top && col + 1 < N && board[(row - 1) * N + (col + 1)]) +
           (col + 1 < N && board[row * N + (col + 1)]) +
           (bottom && col + 1 < N && board[(row + 1) * N + (col + 1)]) +
           (bottom && board[(row + 1) * N + col]) +
           (bottom && col - 1 >= 0 && board[(row + 1) * N + (col - 1)]) +
           (col - 1 >= 0 && board[row * N + (col - 1)]);
}

bool updateBoard(int* board, int localRows, int rank, int size) {
    int* newBoard = (int *)malloc(localRows * N * sizeof(int));
    bool flag = false;

    for (int row = 1; row < localRows + 1; row++) {
        for (int col = 0; col < N; col++) {
            int neighbors = countLiveNeighbors(board, row, col, localRows, rank, size);

            if (board[row * N + col] && (neighbors < 2 || neighbors > 3)) {
                newBoard[(row - 1) * N + col] = DEAD;
                flag = true;
            }
            else if (!board[row * N + col] && neighbors == 3) {
                newBoard[(row - 1) * N + col] = ALIVE;
                flag = true;
            }
            else {
                newBoard[(row - 1) * N + col] = board[row * N + col];
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