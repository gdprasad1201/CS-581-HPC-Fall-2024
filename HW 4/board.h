#ifndef BOARD
#define BOARD

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>

#define ALIVE 1
#define DEAD 0

int N, N, T;
char directory[100];

void init_board(int* board) {
    for (int row = 0; row < N; row++) {
        srand(54321 | row);
        for (int col = 0; col < N; col++) {
            // random value of 0 or 1
            if (drand48() < 0.5) {
                board[row * N + col] = ALIVE;
            } else {
                board[row * N + col] = DEAD;
            }
        }
    }
}

void print_board(int* board, int M_local, int comm_size) {
    FILE* output_file = fopen(directory, "w");
    if (output_file == NULL) {
        printf("Error: could not open file\n");
        return;
    }

    for (int row = 0; row < M_local; row++) {
        for (int col = 0; col < N; col++) {
            fprintf(output_file, "%d ", board[row * N + col]);
        }
        fprintf(output_file, "\n");
    }

    fclose(output_file);
}

int get_top_neighbor(int rank) {
    int neighbor_rank = 0;

    // a top neighbor doesn't exist
    if (rank == 0) {
        neighbor_rank = MPI_PROC_NULL;
    } 
    else {
        neighbor_rank = rank - 1;
    }

    return neighbor_rank;
}

int get_bottom_neighbor(int rank, int size) {
    int neighbor_rank = 0;

    // a bottom neighbor doesn't exist
    if (rank == (size - 1)) {
        neighbor_rank = MPI_PROC_NULL;
    } else {
        neighbor_rank = rank + 1;
    }

    return neighbor_rank;
}

int count_neighbors(int* board, int row, int col, int M_local, int rank, int size) {
    int neighbors = 0;
    // edges of the board
    int top = 1;
    // very top row of the board DOESN'T have a top neighbor
    if (rank == 0 && row == 1) {
        top = 0;
    }
    int bottom = 1;
    // very bottom row of the board DOESN'T have a bottom neighbor
    if (rank == (size - 1) && row == (M_local)) {
        bottom = 0;
    }
    int right = (col + 1) < N;  // check if rightmost column
    int left = (col - 1) >= 0;  // check if leftmost column

    // check that the cell position exists and then check if the cell is alive
    // or dead
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

bool update_board(int* board, int M_local, int rank, int size) {
    int neighbors = 0; 
    int* new_board = (int *)malloc(M_local * N * sizeof(int));
    int final_row = M_local + 1;
    bool flag = false;

    for (int row = 1; row < final_row; row++) {
        for (int col = 0; col < N; col++) {
            neighbors = count_neighbors(board, row, col, M_local, rank, size);

            if (board[row * N + col]) {
                if (neighbors < 2 || neighbors > 3) {
                    new_board[(row - 1) * N + col] = 0;
                    flag = true;
                }
                else {
                    new_board[(row - 1) * N + col] = 1;
                }
            }
            else {
                if (neighbors == 3) {
                    new_board[(row - 1) * N + col] = 1;
                    flag = true;
                }
                // cell remains dead
                else {
                    new_board[(row - 1) * N + col] = 0;
                }
            }
        }
    }

    for (int row = 1; row < final_row; row++) {
        for (int col = 0; col < N; col++) {
            board[row * N + col] = new_board[(row - 1) * N + col];
        }
    }

    free(new_board);

    return flag;
}

#endif