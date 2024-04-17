#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8888
#define BOARD_SIZE 20
#define WINNING_COUNT 5

char board[BOARD_SIZE][BOARD_SIZE];

void initialize_board() {
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            board[i][j] = ' ';
        }
    }
}

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

int check_win(int row, int col, char symbol) {
    int count;

    // Check horizontally
    count = 0;
    for (int j = 0; j < BOARD_SIZE; j++) {
        if (board[row][j] == symbol) {
            count++;
            if (count == WINNING_COUNT) return 1;
        } else {
            count = 0;
        }
    }

    // Check vertically
    count = 0;
    for (int i = 0; i < BOARD_SIZE; i++) {
        if (board[i][col] == symbol) {
            count++;
            if (count == WINNING_COUNT) return 1;
        } else {
            count = 0;
        }
    }

    // Check diagonally (top-left to bottom-right)
    count = 0;
    int i = row, j = col;
    while (i > 0 && j > 0) {
        i--;
        j--;
    }
    while (i < BOARD_SIZE && j < BOARD_SIZE) {
        if (board[i][j] == symbol) {
            count++;
            if (count == WINNING_COUNT) return 1;
        } else {
            count = 0;
        }
        i++;
        j++;
    }

    // Check diagonally (top-right to bottom-left)
    count = 0;
    i = row;
    j = col;
    while (i > 0 && j < BOARD_SIZE - 1) {
        i--;
        j++;
    }
    while (i < BOARD_SIZE && j >= 0) {
        if (board[i][j] == symbol) {
            count++;
            if (count == WINNING_COUNT) return 1;
        } else {
            count = 0;
        }
        i++;
        j--;
    }

    return 0;
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size;

    // Create socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Error in socket creation");
        exit(EXIT_FAILURE);
    }

    // Initialize server address struct
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Bind socket
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Error in binding");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_socket, 10) == -1) {
        perror("Error in listening");
        exit(EXIT_FAILURE);
    }

    printf("Server waiting for connections...\n");

    addr_size = sizeof(client_addr);
    client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_size);
    if (client_socket == -1) {
        perror("Error in accepting connection");
        exit(EXIT_FAILURE);
    }

    printf("Connection accepted from client\n");

    initialize_board();
    print_board();

    // Game loop
    int row, col;
    char symbol = 'X'; // Server is 'X'
    while (1) {
        // Receive player's move
        if (recv(client_socket, &row, sizeof(row), 0) == -1) {
            perror("Error in receiving data");
            exit(EXIT_FAILURE);
        }
        if (recv(client_socket, &col, sizeof(col), 0) == -1) {
            perror("Error in receiving data");
            exit(EXIT_FAILURE);
        }

        // Update board
        board[row][col] = symbol;
        print_board();

        // Check for win
        if (check_win(row, col, symbol)) {
            printf("Server wins!\n");
            break;
        }

        // Switch symbol
        symbol = (symbol == 'X') ? 'O' : 'X';

        // Send updated board to client
        if (send(client_socket, &board, sizeof(board), 0) == -1) {
            perror("Error in sending data");
            exit(EXIT_FAILURE);
        }
    }

    // Close sockets
    close(client_socket);
    close(server_socket);

    return 0;
}
