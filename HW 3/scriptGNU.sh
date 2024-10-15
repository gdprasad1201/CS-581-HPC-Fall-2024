#!/bin/bash
source /apps/profiles/modules_asax.sh.dyn
module load gcc/11.3.0
gcc -Wall -o hw3 hw3.c -fopenmp

./hw3 5000 5000 1 /scratch/ualclsd0192/
./hw3 5000 5000 2 /scratch/ualclsd0192/
./hw3 5000 5000 4 /scratch/ualclsd0192/
./hw3 5000 5000 8 /scratch/ualclsd0192/
./hw3 5000 5000 10 /scratch/ualclsd0192/
./hw3 5000 5000 16 /scratch/ualclsd0192/
./hw3 5000 5000 20 /scratch/ualclsd0192/

cd /scratch/ualclsd0192
diff output.5000.5000.1.txt output.5000.5000.2.txt
diff output.5000.5000.2.txt output.5000.5000.4.txt
diff output.5000.5000.4.txt output.5000.5000.8.txt
diff output.5000.5000.8.txt output.5000.5000.10.txt
diff output.5000.5000.10.txt output.5000.5000.16.txt
diff output.5000.5000.16.txt output.5000.5000.20.txt
