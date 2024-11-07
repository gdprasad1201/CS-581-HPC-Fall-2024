/* 
   Name: Gowtham Prasad
   Email: gdprasad@crimson.ua.edu
   Course Section: CS 581 
   Homework #: 4
   To Compile: mpicc -g -Wall -o game_of_lifeB game_of_lifeB.c
   To Run: mpirun -n <number of processes> game_of_lifeB <matrix size> <number of iterations> <output directory>
*/

#include "board.h"

int main(int argc, char **argv) {
    if (argc != 3) {
        printf("Usage: %s <matrix size> <number of iterations> <output directory>\n", argv[0]);
        return 1;
    }

    N = atoi(argv[1]);
    maxIterations = atoi(argv[2]);
    char processAMT[2];
    bool flag;
    
    MPI_Init(&argc, &argv);

    int rank = 0, size = 0;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size == 0) {
        printf("Error: number of processes must be greater than 0\n");
        MPI_Finalize();
        return 1;
    }

    strcpy(directory, argv[3]);
    strcat(directory, "/output.");
    // strcpy(directory, "output.");
    strcat(directory, argv[1]);
    strcat(directory, ".");
    strcat(directory, argv[2]);
    strcat(directory, ".");
    sprintf(processAMT, "%d", size);
    strcat(directory, processAMT);
    strcat(directory, ".txt");

    // track the time it takes for the program to fully run
    double startTime, endTime;
    startTime = MPI_Wtime();
    
    // Use a 1D array to represent the 2D board
    int* board = (int *)malloc(N * N * sizeof(int));

    // setup the initial board
    if (rank == ROOT) {
        generateBoard(board);
    }

    int remaining_rows = N % size;
    int rows_per_process = N / size;
    // If the number of rows is not evenly divisible by the number of processes, then the last process will have the remaining rows
    if (rank == (size - 1)) {
        rows_per_process += remaining_rows;
    }

    // Use a 1D array to represent the 2D board
    int total_rpp = rows_per_process + 2;  // +2 for the top & bottom neighbor rows
    int* local_board = (int *)malloc(total_rpp * N * sizeof(int));

    int local_board_size = rows_per_process * N;
    int top_neighbor = getTopNeighbor(rank);
    int bottom_neighbor = getBottomNeighbor(rank, size);

    int* counts = (int *)malloc(size * sizeof(int));
    int* displacements = (int *)malloc(size * sizeof(int));
    for (int i = 0; i < size; i++) {
        counts[i] = rows_per_process * N;
        displacements[i] = i * rows_per_process * N;
    }
    counts[size - 1] += remaining_rows * N;

    // send each process their rows
    if (remaining_rows == 0) {
        MPI_Scatter(board, local_board_size, MPI_INT, local_board + N, local_board_size, MPI_INT, ROOT, MPI_COMM_WORLD);
    } 
    else {
        MPI_Scatterv(board, counts, displacements, MPI_INT, local_board + N, local_board_size, MPI_INT, ROOT, MPI_COMM_WORLD);
    }
    
    for (int iteration = 0; iteration < maxIterations; iteration++) {
        // process sends its top row to the previous process & receives the bottom neighbor from the next process
        if (remaining_rows == 0) {
            MPI_Sendrecv(local_board + N, N, MPI_INT, top_neighbor, 0, local_board + (total_rpp - 1) * N, N, MPI_INT, bottom_neighbor, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        else {
            MPI_Sendrecv(local_board + N, N, MPI_INT, top_neighbor, 0, local_board + (total_rpp - 1) * N, N, MPI_INT, bottom_neighbor, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        // process sends its bottom row to the next process & receives the top neighbor from the previous process
        if (remaining_rows == 0) {
            MPI_Sendrecv(local_board + (total_rpp - 2) * N, N, MPI_INT, bottom_neighbor, 0, local_board, N, MPI_INT, top_neighbor, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        else {
            MPI_Sendrecv(local_board + (total_rpp - 2) * N, N, MPI_INT, bottom_neighbor, 0, local_board, N, MPI_INT, top_neighbor, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        
        // update the board
        flag = updateBoard(local_board, rows_per_process, rank, size);

        bool reduction_flag = false;
        MPI_Allreduce(&flag, &reduction_flag, 1, MPI_C_BOOL, MPI_LOR, MPI_COMM_WORLD);

        if (!reduction_flag) {
            break;
        }
    }

    // gather the final results of all the rows into root
    if (remaining_rows == 0) {
        MPI_Gather(local_board + N, local_board_size, MPI_INT, board, local_board_size, MPI_INT, ROOT, MPI_COMM_WORLD);
    }
    else {
        MPI_Gatherv(local_board + N, local_board_size, MPI_INT, board, counts, displacements, MPI_INT, ROOT, MPI_COMM_WORLD);
    }
    // print the final board
    if (rank == ROOT) {
        endTime = MPI_Wtime();
        printBoard(board, N, size);
        printf("Matrix of size %d x %d with %d processes and %d maximum iterations", N, N, size, maxIterations);
        printf("\nWall clock time: %fs\n", endTime - startTime);
    }

    free(board);
    free(local_board);
    free(counts);
    free(displacements);

    MPI_Finalize();
    return 0;
}