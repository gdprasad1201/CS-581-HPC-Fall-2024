#!/bin/bash
source /apps/profiles/modules_asax.sh.dyn

module load cuda/11.7.0

nvcc hw5_GPU_Shared.cu -o hw5_GPU_Shared
./hw5_GPU_Shared 5000 5000 /scratch/ualclsd0192
./hw5_GPU_Shared 10000 5000 /scratch/ualclsd0192

nvcc hw5_GPU_Baseline.cu -o hw5_GPU_Baseline
./hw5_GPU_Baseline 5000 5000 /scratch/ualclsd0192
./hw5_GPU_Baseline 10000 5000 /scratch/ualclsd0192

module load intel

icx -Wall -o hw5_CPU hw5_CPU.c
./hw5_CPU 5000 5000 /scratch/ualclsd0192
./hw5_CPU 10000 5000 /scratch/ualclsd0192

cd /scratch/ualclsd0192
diff output.5000.5000.CPU.txt output.5000.5000.GPU.Shared.txt
diff output.10000.5000.CPU.txt output.10000.5000.GPU.Shared.txt
diff output.5000.5000.GPU.Shared.txt output.5000.5000.GPU.Baseline.txt
diff output.10000.5000.GPU.Shared.txt output.10000.5000.GPU.Baseline.txt