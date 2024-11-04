/* 
   Name: Gowtham Prasad
   Email: gdprasad@crimson.ua.edu
   Course Section: CS 581 
   Homework #: 4
   To Compile: mpicc -g -Wall -o game_of_lifeNB game_of_lifeNB.c
   To Run: mpirun -n <number of processes> ./game_of_lifeNB <matrix size> <number of iterations> <output directory>
*/

#include "board.h"

// root process
#define ROOT 0

int main(int argc, char **argv) {
    M = atoi(argv[1]);
    N = atoi(argv[1]);
    T = atoi(argv[2]);
    char processAMT[2];
    bool flag;
    
    MPI_Init(&argc, &argv);

    int rank = 0, size = 0;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    strcpy(directory, argv[3]);
    strcat(directory, "/output.");
    strcat(directory, argv[1]);
    strcat(directory, ".");
    strcat(directory, argv[2]);
    strcat(directory, ".");
    sprintf(processAMT, "%d", size);
    strcat(directory, processAMT);
    strcat(directory, ".txt");

    // track the time it takes for the program to fully run
    double startwtime, endwtime;
    startwtime = MPI_Wtime();
    
    // Use a 1D array to represent the 2D board
    int* board = (int *)malloc(M * N * sizeof(int));

    // setup the initial board
    if (rank == ROOT) {
        init_board(board);
    }

    int remaining_rows = M % size;
    int rows_per_process = M / size;
    // If the number of rows is not evenly divisible by the number of processes,
    // then the last process will have the remaining rows
    if (rank == (size - 1)) {
        rows_per_process += remaining_rows;
    }

    // Use a 1D array to represent the 2D board
    int total_rpp = rows_per_process + 2;  // +2 for the top & bottom neighbor rows
    int* local_board = (int *)malloc(total_rpp * N * sizeof(int));

    int local_board_size = rows_per_process * N;
    int top_neighbor = get_top_neighbor(rank);
    int bottom_neighbor = get_bottom_neighbor(rank, size);

    int* send_counts = (int *)malloc(size * sizeof(int));
    int* displs = (int *)malloc(size * sizeof(int));
    for (int i = 0; i < size; i++) {
        send_counts[i] = rows_per_process * N;
        displs[i] = i * rows_per_process * N;
    }
    send_counts[size - 1] += remaining_rows * N;

    // send each process their rows
    if (remaining_rows == 0) {
        MPI_Scatterv(board, send_counts, displs, MPI_INT, local_board + N, local_board_size, MPI_INT, ROOT, MPI_COMM_WORLD);
    }
    else {
        MPI_Scatterv(board, send_counts, displs, MPI_INT, local_board + N, local_board_size, MPI_INT, ROOT, MPI_COMM_WORLD);
        
    } 

    MPI_Status recv_stat, send_stat;
    MPI_Request sreq1, sreq2, rreq1, rreq2;

    // every process updates their rows T number of times
    for (int step = 0; step < T; step++) {
        // process sends its top row to the previous process & receives the
        // bottom neighbor from the next process

        if (remaining_rows == 0) {
            MPI_Isend(local_board + N, N, MPI_INT, top_neighbor, 0, MPI_COMM_WORLD, &sreq1);
            MPI_Irecv(local_board + (total_rpp - 1) * N, N, MPI_INT, bottom_neighbor, 0, MPI_COMM_WORLD, &rreq1);
        }
        else {
            MPI_Isend(local_board + N, N, MPI_INT, top_neighbor, 0, MPI_COMM_WORLD, &sreq1);
            MPI_Irecv(local_board + (total_rpp - 1) * N, N, MPI_INT, bottom_neighbor, 0, MPI_COMM_WORLD, &rreq1);
        }
        
        // process sends its bottom row to the next process & receives the top
        // neighbor from the previous process

        if (remaining_rows == 0) {
            MPI_Isend(local_board + (total_rpp - 2) * N, N, MPI_INT, bottom_neighbor, 0, MPI_COMM_WORLD, &sreq2);
            MPI_Irecv(local_board, N, MPI_INT, top_neighbor, 0, MPI_COMM_WORLD, &rreq2);
        }
        else {
            MPI_Isend(local_board + (total_rpp - 2) * N, N, MPI_INT, bottom_neighbor, 0, MPI_COMM_WORLD, &sreq2);
            MPI_Irecv(local_board, N, MPI_INT, top_neighbor, 0, MPI_COMM_WORLD, &rreq2);
        }

        // wait for the sends and receives to complete
        MPI_Wait(&sreq1, &send_stat);
        MPI_Wait(&rreq1, &recv_stat);
        MPI_Wait(&sreq2, &send_stat);
        MPI_Wait(&rreq2, &recv_stat);

        // update the board
        flag = update_board(local_board, rows_per_process, rank, size);

        bool reduction_flag = false;
        MPI_Allreduce(&flag, &reduction_flag, 1, MPI_C_BOOL, MPI_LOR, MPI_COMM_WORLD);

        if (!reduction_flag) {
            break;
        }
    }

    int* recv_counts = (int *)malloc(size * sizeof(int));
    int* recv_displs = (int *)malloc(size * sizeof(int));
    for (int i = 0; i < size; i++) {
        recv_counts[i] = rows_per_process * N;
        recv_displs[i] = i * rows_per_process * N;
    }
    recv_counts[size - 1] += remaining_rows * N;

    // gather the final results of all the rows into root
    if (remaining_rows == 0) {
        MPI_Gatherv(local_board + N, local_board_size, MPI_INT, board, recv_counts, recv_displs, MPI_INT, ROOT, MPI_COMM_WORLD);
    }
    else {
        MPI_Gatherv(local_board + N, local_board_size, MPI_INT, board, recv_counts, recv_displs, MPI_INT, ROOT, MPI_COMM_WORLD);
    }
    
    // print the final board
    if (rank == ROOT) {
        endwtime = MPI_Wtime();
        print_board(board, M, size);
        printf("Matrix of size %d x %d with %d processes and %d maximum iterations", M, N, size, T);
        printf("\nWall clock time: %fs\n", endwtime - startwtime);
    }

    free(board);
    free(local_board);
    free(send_counts);
    free(displs);
    free(recv_counts);
    free(recv_displs);

    MPI_Finalize();
    return 0;
}