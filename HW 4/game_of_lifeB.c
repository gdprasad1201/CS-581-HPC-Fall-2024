/* 
   Name: Gowtham Prasad
   Email: gdprasad@crimson.ua.edu
   Course Section: CS 581 
   Homework #: 4
   To Compile: mpicc -g -Wall -std=c99 -o game_of_lifeB game_of_lifeB.c
   To Run: mpirun -n <number of processes> game_of_lifeB <matrix size> <number of iterations> <output directory>
*/

#include "board.h"

int main(int argc, char **argv) {
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
    
    // Use a 1D array to represent the 2D board
    int* board = (int *)malloc(N * N * sizeof(int));

    // setup the initial board
    if (rank == ROOT) {
        generateBoard(board);
        startTime = MPI_Wtime();
    }

    int remainingRows = N % size;
    int rowsPerCore = N / size;
    
    // If the number of rows is not evenly divisible by the number of processes, then the last process will have the remaining rows
    if (rank == (size - 1)) {
        rowsPerCore += remainingRows;
    }

    int totalRowsPerCore = rowsPerCore + 2;  // +2 for the top & bottom neighbor rows
    int* localBoard = (int *)malloc(totalRowsPerCore * N * sizeof(int));

    int localSize = rowsPerCore * N;
    int topNeighbor = getTopNeighbor(rank);
    int bottomNeighbor = getBottomNeighbor(rank, size);

    int* counts = (int *)malloc(size * sizeof(int));
    int* displacements = (int *)malloc(size * sizeof(int));
    for (int i = 0; i < size; i++) {
        counts[i] = rowsPerCore * N;
        displacements[i] = i * rowsPerCore * N;
    }
    counts[size - 1] += remainingRows * N;

    // send each process their rows
    if (remainingRows == 0) {
        MPI_Scatter(board, localSize, MPI_INT, localBoard + N, localSize, MPI_INT, ROOT, MPI_COMM_WORLD);
    } 
    else {
        MPI_Scatterv(board, counts, displacements, MPI_INT, localBoard + N, localSize, MPI_INT, ROOT, MPI_COMM_WORLD);
    }
    
    for (int iteration = 0; iteration < maxIterations; iteration++) {
        // process sends its top row to the previous process & receives the bottom neighbor from the next process
        if (remainingRows == 0) {
            MPI_Sendrecv(localBoard + N, N, MPI_INT, topNeighbor, 0, localBoard + (totalRowsPerCore - 1) * N, N, MPI_INT, bottomNeighbor, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        else {
            MPI_Sendrecv(localBoard + N, N, MPI_INT, topNeighbor, 0, localBoard + (totalRowsPerCore - 1) * N, N, MPI_INT, bottomNeighbor, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        // process sends its bottom row to the next process & receives the top neighbor from the previous process
        if (remainingRows == 0) {
            MPI_Sendrecv(localBoard + (totalRowsPerCore - 2) * N, N, MPI_INT, bottomNeighbor, 0, localBoard, N, MPI_INT, topNeighbor, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        else {
            MPI_Sendrecv(localBoard + (totalRowsPerCore - 2) * N, N, MPI_INT, bottomNeighbor, 0, localBoard, N, MPI_INT, topNeighbor, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        
        // update the board
        flag = updateBoard(localBoard, rowsPerCore, rank, size);

        bool reduction_flag = false;
        MPI_Allreduce(&flag, &reduction_flag, 1, MPI_C_BOOL, MPI_LOR, MPI_COMM_WORLD);

        if (!reduction_flag) {
            break;
        }
    }

    // gather the final results of all the rows into root
    if (remainingRows == 0) {
        MPI_Gather(localBoard + N, localSize, MPI_INT, board, localSize, MPI_INT, ROOT, MPI_COMM_WORLD);
    }
    else {
        MPI_Gatherv(localBoard + N, localSize, MPI_INT, board, counts, displacements, MPI_INT, ROOT, MPI_COMM_WORLD);
    }
    // print the final board
    if (rank == ROOT) {
        endTime = MPI_Wtime();
    
        printBoard(board, N, size);
        printf("Matrix of size %d x %d with %d processes and %d maximum iterations:", N, N, size, maxIterations);
        printf("\n\t\tWall clock time: %fs\n\n", endTime - startTime);
    }

    free(board);
    free(localBoard);
    free(counts);
    free(displacements);

    MPI_Finalize();
    return 0;
}