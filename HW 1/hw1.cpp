/*
   Name: Gowtham Prasad
   Email: gdprasad@crimson.ua.edu
   Course Section: CS 581
   Homework #: 1
   Instructions to compile the program: g++ -std=c++11 hw1.cpp -o hw1
   Instructions to run the program: ./a.out <size of board> <max generations>
*/

#include <chrono>
#include <iostream>
#include <vector>

using namespace std;

// Function to print the board
void printBoard(vector<vector<int>>& board, int N) {
    for (int i = 1; i <= N; i++) {
        for (int j = 1; j <= N; j++) {
            if (board[i][j]) {
                cout << " " << board[i][j] << " ";
            }
            else {
                cout << " " << board[i][j] << " ";
            }
        }
        cout << endl;
    }
}

// Function to count the number of live neighbors
int countLiveNeighbors(vector<vector<int>> board, int i, int j) {
    return board[i - 1][j - 1] + board[i - 1][j] + board[i - 1][j + 1] + board[i][j - 1] + board[i][j + 1] + board[i + 1][j - 1] + board[i + 1][j] + board[i + 1][j + 1];
}

// Function to simulate the game of life
void gameOfLife(vector<vector<int>>& board, int N, int maxGenerations) {
    vector<vector<int>> newBoard = board; // Create a new board to store the next generation

    int generation = 0; // Generation counter

    while (generation < maxGenerations) { // Run the simulation for the specified number of generations
        bool change = false;

        for (int i = 1; i <= N; i++) {
            for (int j = 1; j <= N; j++) {
                int liveNeighbors = countLiveNeighbors(newBoard, i, j);
                if (board[i][j]) {
                    if (liveNeighbors < 2 || liveNeighbors > 3) {
                        newBoard[i][j] = 0;
                        change = true;
                    } 
                    else {
                        newBoard[i][j] = 1;
                    }
                } 
                else if (!board[i][j]) {
                    if (liveNeighbors == 3) {
                        newBoard[i][j] = 1;
                        change = true;
                    } 
                    else {
                        newBoard[i][j] = 0;
                    }
                }
            }
        }

        if (!change) {
            break;
        }

        board = newBoard; // Update the board with the new generation
        generation++; // Increment the generation counter
    }
}

int main(int argc, char** argv) {
    if (argc != 3) {
        cout << "Usage: " << argv[0] << " <size of board> <max generations>" << endl;
        return 1;
    }

    int N = stoi(argv[1]), maxGenerations = stoi(argv[2]); // Size of the board and maximum number of generations
    double timeTaken[3]; // Array to store the time taken for each run
    vector<vector<int>> board(N + 2, vector<int>(N + 2, 0)); 

    srand(time(0));
    for (int i = 1; i <= N; i++) { // Initialize the board with random values of 0 and 1
        for (int j = 1; j <= N; j++) {
            board[i][j] = rand() % 2;
        }
    }

    for (int i = 0; i < 3; i++) { // Run the simulation 3 times
        auto start = chrono::steady_clock::now(); // Start the timer
        gameOfLife(board, N, maxGenerations); // Run the simulation
        auto end = chrono::steady_clock::now(); // Stop the timer

        chrono::duration<double> duration = end - start; // Calculate the time taken
        timeTaken[i] = duration.count(); // Store the time taken
    }

    cout << "Time taken: " << (timeTaken[0] + timeTaken[1] + timeTaken[2]) / 3 << " seconds" << endl;

    return 0;
}