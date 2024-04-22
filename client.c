#include "csapp.h"
#include <ncurses.h>

#define BOARD_SIZE 20

void displayBoard(WINDOW *win) {
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            // Display the board content here
            // ...
        }
    }
}

int main() {
    initscr();
    cbreak();
    noecho();
    curs_set(0);

    int clientSocket;
    struct sockaddr_in serverAddr;

    // Create socket
    if ((clientSocket = Socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        unix_error("Cannot create socket");
        exit(1);
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(12345);
    serverAddr.sin_addr.s_addr = inet_addr("192.168.1.100"); // Server IP address

    // Connect to server
    Connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr));

    int board[BOARD_SIZE][BOARD_SIZE];
    struct move {
        int row;
        int col;
    } lastMove;

    WINDOW *gameWin = newwin(BOARD_SIZE + 2, BOARD_SIZE * 2 + 2, 0, 0);
    keypad(gameWin, TRUE);
    box(gameWin, 0, 0);
    mvwprintw(gameWin, 0, 1, "XO Game");

    while (1) {
        // Receive board state from server
        if (Read(clientSocket, board, sizeof(board)) < 0) {
            unix_error("Read failed");
            break;
        }

        // Display board
        displayBoard(gameWin);
        refresh();

        // Get user move
        int row, col;
        mvprintw(BOARD_SIZE + 1, 0, "Enter your move (row col): ");
        scanw("%d %d", &row, &col);

        // Validate move
        if (row < 0 || row >= BOARD_SIZE || col < 0 || col >= BOARD_SIZE || board[row][col] != 0) {
            mvprintw(BOARD_SIZE + 1, 0, "Invalid move. Try again.");
            refresh();
            continue;
        }

        // Send move to server
        char buf[100];
        sprintf(buf, "%d %d", row, col);
        Write(clientSocket, buf, strlen(buf) + 1);

        // Update last move
        lastMove.row = row;
        lastMove.col = col;

        // Check for win
        int win = 0;
        int player = board[lastMove.row][lastMove.col];
        int dr[] = {-1, 0, 1, -1, 1, -1, 0, 1};
        int dc[] = {-1, -1, -1, 0, 0, 1, 1, 1};
        for (int d = 0; d < 8; d++) {
            int i = lastMove.row + dr[d], j = lastMove.col + dc[d], count = 1;
            while (i >= 0 && i < BOARD_SIZE && j >= 0 && j < BOARD_SIZE && board[i][j] == player) {
                i += dr[d];
                j += dc[d];
                count++;
            }
            if (count >= 5) {
                win = 1;
                break;
            }
        }
        if (win) {
            mvprintw(BOARD_SIZE + 1, 0, "You win!");
            refresh();
            break;
        }
    }

    Close(clientSocket);
    endwin();
    return 0;
}
