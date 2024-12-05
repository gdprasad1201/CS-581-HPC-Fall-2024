#include <stdio.h>
#include <stdlib.h>
#include <cuda.h>
#include <cuda_runtime.h>

#define ALIVE 1
#define DEAD 0

__global__ void updateBoard(int* grid, int* nextGrid, int* flag, int N) {
    extern __shared__ int sharedGrid[];

    // Global indices
    int global_x = blockIdx.x * blockDim.x + threadIdx.x, global_y = blockIdx.y * blockDim.y + threadIdx.y;

    // Shared memory dims
    int shared_size = blockDim.x + 2;

    // Load current cell
    if (global_x < N && global_y < N) {
        sharedGrid[(threadIdx.y + 1) * shared_size + (threadIdx.x + 1)] = grid[global_y * N + global_x];

        // Load ghost cells, top row
        if (threadIdx.y == DEAD) {
            if (global_y)
                sharedGrid[threadIdx.y * shared_size + (threadIdx.x + 1)] = grid[(global_y - 1) * N + global_x];

            // Load ghost cells, top left corner
            if (threadIdx.x == DEAD && global_x && global_y) 
                sharedGrid[threadIdx.y * shared_size + threadIdx.x] = grid[(global_y - 1) * N + (global_x - 1)];

            // Load ghost cells, top right corner
            if (threadIdx.x == blockDim.x - 1 && global_y && global_x < N - 1)
                sharedGrid[threadIdx.y * shared_size + (threadIdx.x + 2)] = grid[(global_y - 1) * N + (global_x + 1)];
        }

        // Load ghost cells, bottom row
        if (threadIdx.y == blockDim.y - 1) {
            if (global_y < N - 1)
                sharedGrid[(threadIdx.y + 2) * shared_size + (threadIdx.x + 1)] = grid[(global_y + 1) * N + global_x];

            // Load ghost cells, bottom left corner
            if (threadIdx.x == DEAD && global_x && global_y < N - 1) 
                sharedGrid[(threadIdx.y + 2) * shared_size + threadIdx.x] = grid[(global_y + 1) * N + (global_x - 1)];

            // Load ghost cells, bottom right corner
            if (threadIdx.x == blockDim.x - 1 && global_x < N - 1 && global_y < N - 1) 
                sharedGrid[(threadIdx.y + 2) * shared_size + (threadIdx.x + 2)] = grid[(global_y + 1) * N + (global_x + 1)];
        }

        // Load ghost cells, left column
        if (threadIdx.x == DEAD && global_x)
            sharedGrid[(threadIdx.y + 1) * shared_size + threadIdx.x] = grid[global_y * N + (global_x - 1)];

        // Load ghost cells, right column
        if (threadIdx.x == blockDim.x - 1 && global_x < N - 1)
            sharedGrid[(threadIdx.y + 1) * shared_size + (threadIdx.x + 2)] = grid[global_y * N + (global_x + 1)];
    }

    __syncthreads();

    // Update current cell
    if (global_x && global_x < N - 1 && global_y && global_y < N - 1) {
        int neighbors = sharedGrid[threadIdx.y * shared_size + threadIdx.x] + sharedGrid[threadIdx.y * shared_size + (threadIdx.x + 1)] + sharedGrid[threadIdx.y * shared_size + (threadIdx.x + 2)] + sharedGrid[(threadIdx.y + 1) * shared_size + threadIdx.x] + sharedGrid[(threadIdx.y + 1) * shared_size + (threadIdx.x + 2)] + sharedGrid[(threadIdx.y + 2) * shared_size + threadIdx.x] + sharedGrid[(threadIdx.y + 2) * shared_size + (threadIdx.x + 1)] + sharedGrid[(threadIdx.y + 2) * shared_size + (threadIdx.x + 2)];

        if (sharedGrid[(threadIdx.y + 1) * shared_size + (threadIdx.x + 1)]) {
            if (neighbors < 2 || neighbors > 3) {
                nextGrid[global_y * N + global_x] = DEAD;
                atomicAdd(flag, 1);
            } 
            else {
                nextGrid[global_y * N + global_x] = ALIVE;
            }
        } 
        else {
            if (neighbors == 3) {
                nextGrid[global_y * N + global_x] = ALIVE;
                atomicAdd(flag, 1);
            } 
            else {
                nextGrid[global_y * N + global_x] = DEAD;
            }
        }
    }
}

void checkCuda(cudaError_t result) {
    if (result != cudaSuccess) {
        fprintf(stderr, "CUDA Runtime Error: %s\n", cudaGetErrorString(result));
        exit(EXIT_FAILURE);
    }
}

void printGrid(int* grid, int N) {
    for (int i = 1; i < N - 1; i++) {
        for (int j = 1; j < N - 1; j++) {
            printf("%d ", grid[i * N + j]);
        }
        printf("\n");
    }
}

int main(int argc, char** argv) {
    int N = atoi(argv[1]) + 2, maxGenerations = atoi(argv[2]);
    const int blockSize = 32;

    char outputDirectory[150];
    sprintf(outputDirectory, "%s/output.%s.%s.GPU.Shared.txt", argv[3], argv[1], argv[2]);

    int* hostGrid = (int*)malloc(N * N * sizeof(int));
    int* hostNextGrid = (int*)malloc(N * N * sizeof(int));

    for (int i = 0; i < N; i++) {
        srand(54321 | i);
        for (int j = 0; j < N; j++) {
            if (i == 0 || i == N - 1 || j == 0 || j == N - 1) {
                hostGrid[i * N + j] = DEAD;
            } 
            else {
                if (drand48() < 0.5) 
                    hostGrid[i * N + j] = ALIVE;
                else 
                    hostGrid[i * N + j] = DEAD;
            }
        }
    }

    #ifdef DEBUG
        printf("Initial Grid:\n");
        printGrid(hostGrid, N);
    #endif
    

    cudaEvent_t startEvent, stopEvent;
    checkCuda(cudaEventCreate(&startEvent));
    checkCuda(cudaEventCreate(&stopEvent));
    float ms;

    int* deviceCurrentGrid;
    int* deviceNextGrid;
    int* deviceFlag;
    checkCuda(cudaMalloc((void**)&deviceCurrentGrid, N * N * sizeof(int)));
    checkCuda(cudaMalloc((void**)&deviceNextGrid, N * N * sizeof(int)));
    checkCuda(cudaMalloc((void**)&deviceFlag, sizeof(int)));

    checkCuda(cudaMemcpy(deviceCurrentGrid, hostGrid, N * N * sizeof(int), cudaMemcpyHostToDevice));

    // N = matrix size + 2 for ghost cells
    dim3 threadsPerBlock(blockSize, blockSize);
    dim3 blocksPerGrid((N + blockSize - 1) / blockSize, (N + blockSize - 1) / blockSize);

    checkCuda(cudaEventRecord(startEvent));

    for (int gen = 0; gen < maxGenerations; gen++) {
        checkCuda(cudaMemset(deviceFlag, 0, sizeof(int)));
        
        updateBoard<<<blocksPerGrid, threadsPerBlock, (blockSize + 2) * (blockSize + 2) * sizeof(int)>>>(deviceCurrentGrid, deviceNextGrid, deviceFlag, N);

        int flag;
        checkCuda(cudaMemcpy(&flag, deviceFlag, sizeof(int), cudaMemcpyDeviceToHost));
        if (!flag) {
            printf("Converged at generation %d\n", gen + 1);
            break;
        }

        int* temp = deviceCurrentGrid;
        deviceCurrentGrid = deviceNextGrid;
        deviceNextGrid = temp;
    }

    checkCuda(cudaMemcpy(hostGrid, deviceCurrentGrid, N * N * sizeof(int), cudaMemcpyDeviceToHost));
    
    checkCuda(cudaEventRecord(stopEvent));
    checkCuda(cudaEventSynchronize(stopEvent));
    checkCuda(cudaEventElapsedTime(&ms, startEvent, stopEvent));

    checkCuda(cudaEventDestroy(startEvent));
    checkCuda(cudaEventDestroy(stopEvent));

    #ifdef DEBUG
        printf("Final Grid:\n");
        printGrid(hostGrid, N);
    #endif

    FILE* outputFile = fopen(outputDirectory, "w");
    for (int i = 1; i < N - 1; i++) {
        for (int j = 1; j < N - 1; j++) {
            fprintf(outputFile, "%d ", hostGrid[i * N + j]);
        }
        fprintf(outputFile, "\n");
    }
    fclose(outputFile);

    printf("GPU Shared Memory Optimization of the Game of Life: Matrix of size %d x %d with %d generations computed in %f sec\n\n", N - 2, N - 2, maxGenerations, ms / 1000);

    checkCuda(cudaFree(deviceCurrentGrid));
    checkCuda(cudaFree(deviceNextGrid));
    checkCuda(cudaFree(deviceFlag));

    free(hostGrid);
    free(hostNextGrid);

    return 0;
}