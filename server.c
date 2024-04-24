#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BOARD_SIZE 20
#define WIN_LENGTH 5
#define PORT 8888

char board[BOARD_SIZE][BOARD_SIZE];
char turn = 'X';

void init_board() {
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            board[i][j] = ' ';
        }
    }
}

bool is_valid_move(int row, int col) {
    return row >= 0 && row < BOARD_SIZE && col >= 0 && col < BOARD_SIZE && board[row][col] == ' ';
}

bool check_win(int row, int col) {
    char symbol = board[row][col];
    int count;

    count = 1;
    for (int i = col + 1; i < BOARD_SIZE && board[row][i] == symbol; i++) {
        count++;
    }
    for (int i = col - 1; i >= 0 && board[row][i] == symbol; i--) {
        count++;
    }
    if (count >= WIN_LENGTH) {
        return true;
    }

    count = 1;
    for (int i = row + 1; i < BOARD_SIZE && board[i][col] == symbol; i++) {
        count++;
    }
    for (int i = row - 1; i >= 0 && board[i][col] == symbol; i--) {
        count++;
    }
    if (count >= WIN_LENGTH) {
        return true;
    }

    count = 1;
    for (int i = row - 1, j = col - 1; i >= 0 && j >= 0 && board[i][j] == symbol; i--, j--) {
        count++;
    }
    for (int i = row + 1, j = col + 1; i < BOARD_SIZE && j < BOARD_SIZE && board[i][j] == symbol; i++, j++) {
        count++;
    }
    if (count >= WIN_LENGTH) {
        return true;
    }

    count = 1;
    for (int i = row + 1, j = col - 1; i < BOARD_SIZE && j >= 0 && board[i][j] == symbol; i++, j--) {
        count++;
    }
    for (int i = row - 1, j = col + 1; i >= 0 && j < BOARD_SIZE && board[i][j] == symbol; i--, j++) {
        count++;
    }
    if (count >= WIN_LENGTH) {
        return true;
    }

    return false;
}

bool is_draw() {
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (board[i][j] == ' ') {
                return false;
            }
        }
    }
    return true;
}

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

void update_board(int row, int col) {
    board[row][col] = turn;
}

int main() {
    init_board();

    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);


    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }


    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                   &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);


    if (bind(server_fd, (struct sockaddr *) &address,
             sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    if ((new_socket = accept(server_fd, (struct sockaddr *) &address,
                              (socklen_t *) &addrlen)) < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    while (1) {
        print_board();

        printf("Player %c's turn. Enter row and column (space): ", turn);
        int row, col;
        scanf("%d %d", &row, &col);

        if (!is_valid_move(row, col)) {
            printf("Invalid move! Try again.\n");
            continue;
        }

        update_board(row, col);

        if (check_win(row, col)) {
            printf("Player %c wins!\n", turn);
            send(new_socket, &turn, sizeof(turn), 0); 
            break;
        } else if (is_draw()) {
            printf("It's  draw!\n");
            send(new_socket, "D", 1, 0); 
            break;
        }

        turn = (turn == 'X') ? 'O' : 'X';

        // Send updated board to client
        send(new_socket, &board, sizeof(board), 0);

        // Receive move from client
        int client_row, client_col;
        recv(new_socket, &client_row, sizeof(client_row), 0);
        recv(new_socket, &client_col, sizeof(client_col), 0);
        update_board(client_row, client_col);

        // Check win or draw after client move
        if (check_win(client_row, client_col)) {
            printf("Player %c wins!\n", board[client_row][client_col]);
            send(new_socket, &board[client_row][client_col], sizeof(board[client_row][client_col]), 0); // Send winner information to client
            break;
        } else if (is_draw()) {
            printf("It's  draw!\n");
            send(new_socket, "D", 1, 0); // Send draw information to client
            break;
        }
    }
    close(new_socket);
    close(server_fd);
    return 0;
}
