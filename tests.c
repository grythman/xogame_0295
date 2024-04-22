#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define SIZE 20
#define WIN_COUNT 5
#define PORT 8080

char board[SIZE][SIZE];
int currentPlayer;
int serverSocket, clientSocket1, clientSocket2;

void initialize_board() {
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            board[i][j] = ' ';
        }
    }
}

void print_board() {
    printf("  ");
    for (int i = 0; i < SIZE; i++) {
        printf("%2d", i);
    }
    printf("\n");
    for (int i = 0; i < SIZE; i++) {
        printf("%2d", i);
        for (int j = 0; j < SIZE; j++) {
            printf("|%c", board[i][j]);
        }
        printf("|\n");
    }
}

int check_winner(char player) {
    // Check rows
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j <= SIZE - WIN_COUNT; j++) {
            int count = 0;
            for (int k = 0; k < WIN_COUNT; k++) {
                if (board[i][j + k] == player)
                    count++;
                else
                    break;
            }
            if (count == WIN_COUNT)
                return 1;
        }
    }

    // Check columns
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j <= SIZE - WIN_COUNT; j++) {
            int count = 0;
            for (int k = 0; k < WIN_COUNT; k++) {
                if (board[j + k][i] == player)
                    count++;
                else
                    break;
            }
            if (count == WIN_COUNT)
                return 1;
        }
    }

    // Check diagonals
    for (int i = 0; i <= SIZE - WIN_COUNT; i++) {
        for (int j = 0; j <= SIZE - WIN_COUNT; j++) {
            int count1 = 0, count2 = 0;
            for (int k = 0; k < WIN_COUNT; k++) {
                if (board[i + k][j + k] == player)
                    count1++;
                else
                    break;
            }
            for (int k = 0; k < WIN_COUNT; k++) {
                if (board[i + k][j + WIN_COUNT - 1 - k] == player)
                    count2++;
                else
                    break;
            }
            if (count1 == WIN_COUNT || count2 == WIN_COUNT)
                return 1;
        }
    }

    return 0;
}

int make_move(int row, int col, char player) {
    if (board[row][col] == ' ') {
        board[row][col] = player;
        return 1;
    } else {
        return 0;
    }
}

int main() {
    int clientMove[2];

    initialize_board();
    currentPlayer = 1;

    // Create socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        printf("Failed to create socket\n");
        return -1;
    }

    // Bind socket to port
    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(PORT);
    if (bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) == -1) {
        printf("Failed to bind socket\n");
        return -1;
    }

    // Listen for incoming connections
    if (listen(serverSocket, 2) == -1) {
        printf("Failed to listen for connections\n");
        return -1;
    }

    // Accept first client connection
    int addrLen = sizeof(serverAddress);
    clientSocket1 = accept(serverSocket, (struct sockaddr *)&serverAddress, (socklen_t *)&addrLen);
    if (clientSocket1 == -1) {
        printf("Failed to accept first client connection\n");
        return -1;
    }
    printf("First client connected\n");

    // Accept second client connection
    clientSocket2 = accept(serverSocket, (struct sockaddr *)&serverAddress, (socklen_t *)&addrLen);
    if (clientSocket2 == -1) {
        printf("Failed to accept second client connection\n");
        return -1;
    }
    printf("Second client connected\n");

    while (1) {
        // Print board
        print_board();

        // Wait for first client move
        printf("Waiting for first client move...\n");
        if (recv(clientSocket1, &clientMove, sizeof(clientMove), 0) <= 0) {
            printf("Failed to receive first client move\n");
            break;
        }

        // Make first client move
        if (!make_move(clientMove[0], clientMove[1], 'X')) {
            printf("Invalid move from first client!\n");
            break;
        }

        // Check for first client win
        if (check_winner('X')) {
            printf("First client wins!\n");
            break;
        }

        // Print board
        print_board();

        // Wait for second client move
        printf("Waiting for second client move...\n");
        if (recv(clientSocket2, &clientMove, sizeof(clientMove), 0) <= 0) {
            printf("Failed to receive second client move\n");
            break;
        }

        // Make second client move
        if (!make_move(clientMove[0], clientMove[1], 'O')) {
            printf("Invalid move from second client!\n");
            break;
        }

        // Check for second client win
        if (check_winner('O')) {
            printf("Second client wins!\n");
            break;
        }

        // Send updated board to clients
        if (send(clientSocket1, &board, sizeof(board), 0) == -1) {
            printf("Failed to send updated board to first client\n");
            break;
        }
        if (send(clientSocket2, &board, sizeof(board), 0) == -1) {
            printf("Failed to send updated board to second client\n");
            break;
        }
    }

    // Close sockets
    close(clientSocket1);
    close(clientSocket2);
    close(serverSocket);

    return 0;
}
