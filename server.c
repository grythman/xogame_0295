#include "csapp.h"
#include <ncurses.h>

#define BOARD_SIZE 20

int board[BOARD_SIZE][BOARD_SIZE] = {0};
int currentPlayer = 1; // 1 for X, -1 for O

struct move {
    int row;
    int col;
} lastMove;

void displayBoard(WINDOW *win) {
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (board[i][j] == 1) {
                mvprintw(i, j * 2, "X");
            } else if (board[i][j] == -1) {
                mvprintw(i, j * 2, "O");
            } else {
                mvprintw(i, j * 2, " ");
            }
        }
    }
}

int checkWin() {
    int row, col;

    // Check rows, columns, and diagonals
    for (row = 0; row < BOARD_SIZE; row++) {
        int count = 0;
        for (col = 0; col < BOARD_SIZE; col++) {
            if (board[row][col] == currentPlayer) {
                count++;
            } else {
                count = 0;
            }
            if (count == 5) {
                return 1;
            }
        }
    }

    for (col = 0; col < BOARD_SIZE; col++) {
        int count = 0;
        for (row = 0; row < BOARD_SIZE; row++) {
            if (board[row][col] == currentPlayer) {
                count++;
            } else {
                count = 0;
            }
            if (count == 5) {
                return 1;
            }
        }
    }

    // Check diagonals
    int count = 0;
    for (row = 0, col = 0; row < BOARD_SIZE && col < BOARD_SIZE; row++, col++) {
        if (board[row][col] == currentPlayer) {
            count++;
        } else {
            count = 0;
        }
        if (count == 5) {
            return 1;
        }
    }

    count = 0;
    for (row = 0, col = BOARD_SIZE - 1; row < BOARD_SIZE && col >= 0; row++, col--) {
        if (board[row][col] == currentPlayer) {
            count++;
        } else {
            count = 0;
        }
        if (count == 5) {
            return 1;
        }
    }

    return 0;
}

int main() {
    initscr();
    cbreak();
    noecho();
    curs_set(0);

    int serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t addrLen;

    // Create socket
    if ((serverSocket = Socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        unix_error("Cannot create socket");
        exit(1);
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(12345);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    // Bind socket
    Bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr));

    // Listen for connections
    Listen(serverSocket, 5);

    printf("Server started. Waiting for clients...\n");

    while (1) {
        addrLen = sizeof(clientAddr);
        if ((clientSocket = Accept(serverSocket, (struct sockaddr *)&clientAddr, &addrLen)) < 0) {
            unix_error("Accept failed");
            continue;
        }

        printf("Client connected. Starting game...\n");

        WINDOW *gameWin = newwin(BOARD_SIZE + 2, BOARD_SIZE * 2 + 2, 0, 0);
        keypad(gameWin, TRUE);
        box(gameWin, 0, 0);
        mvwprintw(gameWin, 0, 1, "XO Game");

        while (1) {
            int row, col;

            // Send current board state to client
            displayBoard(gameWin);
            refresh();
            Write(clientSocket, board, sizeof(board));

            // Receive move from client
            char buf[100];
            if (Read(clientSocket, buf, sizeof(buf)) < 0) {
                unix_error("Read failed");
                break;
            }

            sscanf(buf, "%d %d", &row, &col);

            // Validate move
            if (row < 0 || row >= BOARD_SIZE || col < 0 || col >= BOARD_SIZE || board[row][col] != 0) {
                mvprintw(BOARD_SIZE + 1, 0, "Invalid move. Try again.");
                refresh();
                continue;
            }

            // Update board
            board[row][col] = currentPlayer;
            lastMove.row = row;
            lastMove.col = col;

            // Check for win
            if (checkWin()) {
                mvprintw(BOARD_SIZE + 1, 0, "Player %c wins!", currentPlayer == 1 ? 'X' : 'O');
                refresh();
                break;
            }

            // Switch player
            currentPlayer = -currentPlayer;
        }

        Close(clientSocket);
        printf("Client disconnected.\n");
        delwin(gameWin);
    }

    Close(serverSocket);
    endwin();
    return 0;
}
