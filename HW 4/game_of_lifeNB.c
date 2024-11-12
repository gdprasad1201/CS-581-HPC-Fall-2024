/* 
   Name: Gowtham Prasad
   Email: gdprasad@crimson.ua.edu
   Course Section: CS 581 
   Homework #: 4
   To Compile: mpicc -g -Wall -o game_of_lifeNB game_of_lifeNB.c
   To Run: mpirun -n <number of processes> game_of_lifeNB <matrix size> <number of iterations> <output directory>
*/

#include "board.h"

int main(int argc, char **argv) {
    N = atoi(argv[1]);
    maxIterations = atoi(argv[2]);
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

    char sizeChar[2];
    sprintf(sizeChar, "%d", size);
    
    strcpy(directory, argv[3]);
    strcat(directory, "/output.");
    // strcpy(directory, "output.");
    strcat(directory, argv[1]);
    strcat(directory, ".");
    strcat(directory, argv[2]);
    strcat(directory, ".");
    strcat(directory, sizeChar);
    strcat(directory, ".txt");

    // track the time it takes for the program to fully run
    double startTime, endTime;
    
    // Use a 1D array to represent the 2D board
    int* board = (int *)malloc(N * N * sizeof(int));

    // setup the initial board
    if (rank == ROOT) {
        generateBoard(board);
        startTime = MPI_Wtime();
        #ifdef DEBUG1
            printf("Initial Board:\n");
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < N; j++) {
                    printf("%d ", board[i * N + j]);
                }
                printf("\n");
            }
        #endif
    }

    int remainingRows = N % size;
    int rowsPerCore = N / size;
    // If the number of rows is not evenly divisible by the number of processes, then the last process will have the remaining rows
    if (rank == size - 1) {
        rowsPerCore += remainingRows;
    }

    // Use a 1D array to represent the 2D board
    int totalRowsPerCore = rowsPerCore + 2;  // +2 for the top & bottom neighbor rows
    int* localBoard = (int *)malloc(totalRowsPerCore * N * sizeof(int));

    int localSize = rowsPerCore * N;
    int topNeighbor = getTopNeighbor(rank);
    int bottomNeighbor = getBottomNeighbor(rank, size);

    int* send_counts = (int *)malloc(size * sizeof(int));
    int* displs = (int *)malloc(size * sizeof(int));
    for (int i = 0; i < size; i++) {
        send_counts[i] = rowsPerCore * N;
        displs[i] = i * rowsPerCore * N;
    }
    send_counts[size - 1] += remainingRows * N;

    // send each process their rows
    if (remainingRows == 0) {
        MPI_Scatterv(board, send_counts, displs, MPI_INT, localBoard + N, localSize, MPI_INT, ROOT, MPI_COMM_WORLD);
    }
    else {
        MPI_Scatterv(board, send_counts, displs, MPI_INT, localBoard + N, localSize, MPI_INT, ROOT, MPI_COMM_WORLD);  
    } 

    MPI_Status recv_stat, send_stat; // status for MPI_Recv and MPI_Send
    MPI_Request sreq1, sreq2, rreq1, rreq2; // request for MPI_Isend and MPI_Irecv

    for (int iteration = 0; iteration < maxIterations; iteration++) {
        // process sends its top row to the previous process & receives the bottom neighbor from the next process
        if (remainingRows == 0) {
            MPI_Isend(localBoard + N, N, MPI_INT, topNeighbor, 0, MPI_COMM_WORLD, &sreq1);
            MPI_Irecv(localBoard + (totalRowsPerCore - 1) * N, N, MPI_INT, bottomNeighbor, 0, MPI_COMM_WORLD, &rreq1);
        }
        else {
            MPI_Isend(localBoard + N, N, MPI_INT, topNeighbor, 0, MPI_COMM_WORLD, &sreq1);
            MPI_Irecv(localBoard + (totalRowsPerCore - 1) * N, N, MPI_INT, bottomNeighbor, 0, MPI_COMM_WORLD, &rreq1);
        }
        
        // process sends its bottom row to the next process & receives the top neighbor from the previous process
        if (remainingRows == 0) {
            MPI_Isend(localBoard + (totalRowsPerCore - 2) * N, N, MPI_INT, bottomNeighbor, 0, MPI_COMM_WORLD, &sreq2);
            MPI_Irecv(localBoard, N, MPI_INT, topNeighbor, 0, MPI_COMM_WORLD, &rreq2);
        }
        else {
            MPI_Isend(localBoard + (totalRowsPerCore - 2) * N, N, MPI_INT, bottomNeighbor, 0, MPI_COMM_WORLD, &sreq2);
            MPI_Irecv(localBoard, N, MPI_INT, topNeighbor, 0, MPI_COMM_WORLD, &rreq2);
        }

        // wait for the sends and receives to complete
        MPI_Wait(&sreq1, &send_stat);
        MPI_Wait(&rreq1, &recv_stat);
        MPI_Wait(&sreq2, &send_stat);
        MPI_Wait(&rreq2, &recv_stat);

        // update the board
        flag = updateBoard(localBoard, rowsPerCore, rank, size);

        bool reduction_flag = false;
        MPI_Allreduce(&flag, &reduction_flag, 1, MPI_C_BOOL, MPI_LOR, MPI_COMM_WORLD);

        if (!reduction_flag) {
            break;
        }
    }

    int* recv_counts = (int *)malloc(size * sizeof(int));
    int* recv_displs = (int *)malloc(size * sizeof(int));
    for (int i = 0; i < size; i++) {
        recv_counts[i] = rowsPerCore * N;
        recv_displs[i] = i * rowsPerCore * N;
    }
    recv_counts[size - 1] += remainingRows * N;

    // gather the final results of all the rows into root
    if (remainingRows == 0) {
        MPI_Gatherv(localBoard + N, localSize, MPI_INT, board, recv_counts, recv_displs, MPI_INT, ROOT, MPI_COMM_WORLD);
    }
    else {
        MPI_Gatherv(localBoard + N, localSize, MPI_INT, board, recv_counts, recv_displs, MPI_INT, ROOT, MPI_COMM_WORLD);
    }
    
    // print the final board
    if (rank == ROOT) {
        endTime = MPI_Wtime();
        #ifdef DEBUG1
            printf("\nFinal Board:\n");
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < N; j++) {
                    printf("%d ", board[i * N + j]);
                }
                printf("\n");
            }
        #endif
        printBoard(board, N);
        printf("Matrix of size %d x %d with %d processes and %d maximum iterations:\n", N, N, size, maxIterations);
        printf("\t\tWall clock time: %fs\n\n", endTime - startTime);
    }

    free(board);
    free(localBoard);
    free(send_counts);
    free(displs);
    free(recv_counts);
    free(recv_displs);

    MPI_Finalize();
    return 0;
}