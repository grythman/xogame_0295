#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BOARD_SIZE 20
#define PORT 8888

char board[BOARD_SIZE][BOARD_SIZE];

void print_board() {
    printf("\n");
    printf("   ");
    for (int i = 0; i < BOARD_SIZE; i++) {
    }
    printf("\n");
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            printf("|%c", board[i][j]);
        }
        printf("|\n");
    }
    printf("\n");
}

int main(int argc, char *argv[]) {
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    printf("Connected to server. Let's play!\n");

    while (1) {
        if ((valread = read(sock, &board, sizeof(board))) == 0) {
            printf("Connection closed by server.\n");
            break;
        }

        print_board();

        if (valread == sizeof(char)) {
            char winner;
            read(sock, &winner, sizeof(char));
            if (winner == 'D') {
                printf("It's draw!\n");
            } else {
                printf("Player %c wins!\n", winner);
            }
            break;
        }

        // Player move
        printf("Enter row and column (space): ");
        int row, col;
        scanf("%d %d", &row, &col);
        send(sock, &row, sizeof(row), 0);
        send(sock, &col, sizeof(col), 0);

        // Receive updated board after server move
        if ((valread = read(sock, &board, sizeof(board))) == 0) {
            printf("Connection closed by server.\n");
            break;
        }

        print_board();

        // Check if there is winner or draw
        if (valread == sizeof(char)) {
            char winner;
            read(sock, &winner, sizeof(char));
            if (winner == 'D') {
                printf("It's draw!\n");
            } else {
                printf("Player %c wins!\n", winner);
            }
            break;
        }
    }

    return 0;
}
