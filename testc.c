#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h> 
#include <unistd.h>

#define SIZE 20
#define PORT 8080

void print_board(char board[SIZE][SIZE]) {
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

int main() {
    char serverBoard[SIZE][SIZE];
    int clientMove[2];

    // Create socket
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        printf("Failed to create socket\n");
        return -1;
    }

    // Connect to server
    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);
    if (inet_pton(AF_INET, "127.0.0.1", &(serverAddress.sin_addr)) <= 0) {
        printf("Invalid server address\n");
        return -1;
    }

    if (connect(clientSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) == -1) {
        printf("Failed to connect to server\n");
        return -1;
    }

    while (1) {
        // Print current board
        printf("Current board:\n");
        print_board(serverBoard);

        // Make your move
        printf("Enter your move (row column): ");
        if (scanf("%d %d", &clientMove[0], &clientMove[1]) != 2) {
            printf("Invalid input!\n");
            break;
        }

        // Send your move to server
        if (send(clientSocket, &clientMove, sizeof(clientMove), 0) == -1) {
            printf("Failed to send client move\n");
            break;
        }

        // Receive updated board from server
        if (recv(clientSocket, &serverBoard, sizeof(serverBoard), 0) <= 0) {
            printf("Failed to receive updated board\n");
            break;
        }
    }

    // Close socket
    close(clientSocket);

    return 0;
}
