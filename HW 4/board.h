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

void printBoard(int* board, int M_local, int comm_size) {
    FILE* finalBoardFIle = fopen(directory, "w");
    if (finalBoardFIle == NULL) {
        printf("Error: could not open file\n");
        return;
    }

    for (int row = 0; row < M_local; row++) {
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
        return (rank - 1);
    }
}

int getBottomNeighbor(int rank, int size) {
    // a bottom neighbor doesn't exist
    if (rank == (size - 1)) {
        return MPI_PROC_NULL;
    } 
    else {
        return (rank + 1);
    }
}

int countLiveNeighbors(int* board, int row, int col, int M_local, int rank, int size) {
    int neighbors = 0;
    // edges of the board
    int top = 1;
    // very top row of the board DOESN'T have a top neighbor
    if (rank == 0 && row == 1) {
        top = 0;
    }
    int bottom = 1;
    // very bottom row of the board DOESN'T have a bottom neighbor
    if (rank == (size - 1) && row == M_local) {
        bottom = 0;
    }
    int right = (col + 1) < N;  // check if rightmost column
    int left = (col - 1) >= 0;  // check if leftmost column

    // check that the cell position exists and then check if the cell is alive or dead
    if (top && left && board[(row - 1) * N + (col - 1)]) {
        neighbors++;
    }
    if (top && board[(row - 1) * N + col]) {
        neighbors++;
    }
    if (top && right && board[(row - 1) * N + (col + 1)]) {
        neighbors++;
    }
    if (right && board[row * N + (col + 1)]) {
        neighbors++;
    }
    if (bottom && right && board[(row + 1) * N + (col + 1)]) {
        neighbors++;
    }
    if (bottom && board[(row + 1) * N + col]) {
        neighbors++;
    }
    if (bottom && left && board[(row + 1) * N + (col - 1)]) {
        neighbors++;
    }
    if (left && board[row * N + (col - 1)]) {
        neighbors++;
    }

    return neighbors;
}

bool updateBoard(int* board, int M_local, int rank, int size) {
    int neighbors = 0; 
    int* newBoard = (int *)malloc(M_local * N * sizeof(int));
    bool flag = false;

    for (int row = 1; row < M_local + 1; row++) {
        for (int col = 0; col < N; col++) {
            neighbors = countLiveNeighbors(board, row, col, M_local, rank, size);

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
        for (int row = 1; row < M_local + 1; row++) {
            for (int col = 0; col < N; col++) {
                board[row * N + col] = newBoard[(row - 1) * N + col];
            }
        }
    }

    free(newBoard);

    return flag;
}

#endif