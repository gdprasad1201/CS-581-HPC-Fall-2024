# Synchronous (Non-blocking) and Asynchronous (Blocking) Implentations of the Game of Life
## Installation
### Windows 11
1. Download the Cygwin installer from [Cygwin's official website](https://www.cygwin.com/)
2. Run the installer and follow the installation instructions.
3. During the package selection step, search for and select the following packages:
    - `gcc-core`
    - `gcc-g++`
    - `gdb`
    - `emacs`
    - `vim`
    - `vim-common`
    - `make`
    - `openssh`
    - `rsync`
    - `libopenmpi-devel`
    - `libopenmpi40`
    - `libopenmpicxx1`
    - `openmpi`
    - `openmpi-debuginfo`
4. Complete the installation process.
5. Open the Cygwin terminal.
6. Verify the installation by running:
    ```sh
    mpicc --version
    mpirun --version
    ```

### MacOS
1. Install Open MPI using Homebrew:
    ```sh
    brew install open-mpi
    ```
2. Open Terminal and navigate to the directory containing your code.
3. Compile and run the code using the provided commands.
## Blocking version of the Game of Life using MPI
- To Compile:
```sh
mpicc -g -Wall -std=c99 -o game_of_lifeB game_of_lifeB.c
```

- To Run:
```sh
mpirun -n <number of processes> game_of_lifeB <matrix size> <number of iterations> <output directory>
```

## Non-blocking version of the Game of Life using MPI
- To Compile:
```sh
mpicc -g -Wall -std=c99 -o game_of_lifeNB game_of_lifeNB.c
```

- To Run:
```sh
mpirun -n <number of processes> game_of_lifeNB <matrix size> <number of iterations> <output directory>
```

