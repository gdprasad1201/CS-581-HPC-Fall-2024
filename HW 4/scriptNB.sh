#!/bin/bash
source /apps/profiles/modules_asax.sh.dyn
module load openmpi/4.1.4-gcc11

mpicc -g -Wall -o game_of_lifeNB game_of_lifeNB.c

mpirun -n 1 game_of_lifeNB 5000 5000 /scratch/ualclsd0192
mpirun -n 2 game_of_lifeNB 5000 5000 /scratch/ualclsd0192
mpirun -n 4 game_of_lifeNB 5000 5000 /scratch/ualclsd0192
mpirun -n 8 game_of_lifeNB 5000 5000 /scratch/ualclsd0192
mpirun -n 10 game_of_lifeNB 5000 5000 /scratch/ualclsd0192
mpirun -n 16 game_of_lifeNB 5000 5000 /scratch/ualclsd0192
mpirun -n 20 game_of_lifeNB 5000 5000 /scratch/ualclsd0192

cd /scratch/ualclsd0192
diff output.5000.5000.1.txt output.5000.5000.2.txt
diff output.5000.5000.2.txt output.5000.5000.4.txt
diff output.5000.5000.4.txt output.5000.5000.8.txt
diff output.5000.5000.8.txt output.5000.5000.10.txt
diff output.5000.5000.10.txt output.5000.5000.16.txt
diff output.5000.5000.16.txt output.5000.5000.20.txt
