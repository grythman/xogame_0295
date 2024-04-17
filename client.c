#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8888
#define BOARD_SIZE 20
#define WINNING_COUNT 5

char board[BOARD_SIZE][BOARD_SIZE];

void print_board() {
    printf("\n");
    printf("   ");
    for (int i = 0; i < BOARD_SIZE; i++) {
        printf("%2d ", i);
    }
    printf("\n");
    for (int i = 0; i < BOARD_SIZE; i++) {
        printf("%2d ", i);
        for (int j = 0; j < BOARD_SIZE; j++) {
            printf("|%2c", board[i][j]);
        }
        printf("|\n");
    }
}

int main() {
    int client_socket;
    struct sockaddr_in server_addr;

    // Create socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("Error in socket creation");
        exit(EXIT_FAILURE);
    }

    // Initialize server address struct
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Connect to server
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Error in connecting to server");
        exit(EXIT_FAILURE);
    }

    printf("Connected to server\n");

    // Game loop
    int row, col;
    char symbol = 'O'; // Client is 'O'
    while (1) {
        printf("Your turn (row column): ");
        scanf("%d %d", &row, &col);

        // Send player's move
        if (send(client_socket, &row, sizeof(row), 0) == -1) {
            perror("Error in sending data");
            exit(EXIT_FAILURE);
        }
        if (send(client_socket, &col, sizeof(col), 0) == -1) {
            perror("Error in sending data");
            exit(EXIT_FAILURE);
        }

        // Receive updated board from server
        if (recv(client_socket, &board, sizeof(board), 0) == -1) {
            perror("Error in receiving data");
            exit(EXIT_FAILURE);
        }

        print_board();

        // Check for win
        // (Note: Client does not check for win, it's handled by the server)
    }

    // Close socket
    close(client_socket);

    return 0;
}
