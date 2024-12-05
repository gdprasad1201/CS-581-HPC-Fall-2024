#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

#define ALIVE 1
#define DEAD 0

double gettime() {
    struct timeval tval;

    gettimeofday(&tval, NULL);

    return ((double)tval.tv_sec + (double)tval.tv_usec / 1000000.0);
}

void printGrid(int* grid, int rows, int cols) {
    for (int i = 1; i < rows - 1; i++) {
        for (int j = 1; j < cols - 1; j++) {
            printf("%d ", grid[i * cols + j]);
        }
        printf("\n");
    }
}

int main(int agrc, char** argv) {
    int rows = atoi(argv[1]) + 2, cols = atoi(argv[1]) + 2;
    int maxGenerations = atoi(argv[2]);

    char outputDir[150];
    sprintf(outputDir, "%s/output.%s.%s.CPU.txt", argv[3], argv[1], argv[2]);

    int* grid = (int*)malloc(rows * cols * sizeof(int));
    int* newGrid = (int*)malloc(rows * cols * sizeof(int));

    // Initialize the grid
    for (int i = 0; i < rows; i++) {
        srand(54321 | i);
        for (int j = 0; j < cols; j++) {
            if (i == 0 || i == rows - 1 || j == 0 || j == cols - 1) {
                grid[i * cols + j] = DEAD;
            } 
            else {
                if (drand48() < 0.5) 
                    grid[i * cols + j] = ALIVE;
                else
                    grid[i * cols + j] = DEAD;
            }
        }
    }

    #ifdef DEBUG
        printf("Initial grid:\n");
        printGrid(grid, rows, cols);
    #endif

    double startTime = gettime();

    for (int generation = 0; generation < maxGenerations; generation++) {
        int flag = 0;
        for (int i = 1; i < rows - 1; i++) {
            for (int j = 1; j < cols - 1; j++) {
                int count = grid[(i - 1) * cols + j - 1] + grid[(i - 1) * cols + j] + grid[(i - 1) * cols + j + 1] + grid[i * cols + j - 1] + grid[i * cols + j + 1] + grid[(i + 1) * cols + j - 1] + grid[(i + 1) * cols + j] + grid[(i + 1) * cols + j + 1];

                if (grid[i * cols + j] == ALIVE) {
                    if (count < 2 || count > 3) {
                        newGrid[i * cols + j] = DEAD;
                        flag = 1;
                    } 
                    else {
                        newGrid[i * cols + j] = ALIVE;
                    }
                } 
                else {
                    if (count == 3) {
                        newGrid[i * cols + j] = ALIVE;
                        flag = 1;
                    } 
                    else {
                        newGrid[i * cols + j] = DEAD;
                    }
                }
            }
        }

        if (!flag) {
            printf("Converged at generation %d\n", generation + 1);
            break;
        }

        int* temp = grid;
        grid = newGrid;
        newGrid = temp;
    }

    double endTime = gettime();

    #ifdef DEBUG
        // printf("Final grid:\n");
        // printGrid(grid, rows, cols);
    #endif

    FILE* file = fopen(outputDir, "w");
    for (int i = 1; i < rows - 1; i++) {
        for (int j = 1; j < cols - 1; j++) {
            fprintf(file, "%d ", grid[i * cols + j]);
        }
        fprintf(file, "\n");
    }
    fclose(file);

    printf("CPU Game of Life: Matrix of size %d x %d with %d generations computed in %f sec\n\n", rows - 2, cols - 2, maxGenerations, endTime - startTime);

    free(grid);
    free(newGrid);

    return 0;
}