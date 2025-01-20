#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>
#define RED "\033[1;31m"
#define BLUE "\033[1;34m"
#define RESET "\033[0m"
#define BG_BROWN "\033[48;2;133;186;134m"
#define WHITE_BG "\033[48;2;255;255;255m"
#define BLACK_TEXT "\033[30m"
#define BG_RESET "\033[0m"
#define RESET "\033[0m"
#define maxUndo 100

int boardSize = 0;
char **board;
char PLAYER = 'X';
char COMPUTER = 'O';
int undoStack[maxUndo], redoStack[maxUndo]; // we ope 100 char memory to save the moves
int undoTop = -1, redoTop = -1;
int d;

void resetBoard();
void printBoard();
int checkFreeSpaces();
void playerMove();
void computerMove();
char checkWinner();
void printWinner(char winner);
void undoMove(char **board, int *undoStack, int *redoStack, int *undoTop, int *redoTop, int boardSize);
void redoMove(char **board, int *undoStack, int *redoStack, int *undoTop, int *redoTop, int boardSize);
void ask(int d);

int main() {
    srand(time(0));
    printf("Select the boardSize of board(you must enter only 3,4 or 5 ): \n");

    while (1) {

        char input[40];
        fgets(input, sizeof(input), stdin); // Read input as a string. stdin is standard input stream.
        if (sscanf(input, "%d", &boardSize) == 1 && boardSize >= 3 && boardSize <= 5 && strlen(input) == 2) { //sscanf also cheks that is there any character that is not a number.
            break; // Valid input
        } else {
            printf("Invalid input. Select the boardSize of board(you must enter only 3,4 or 5 ): \n");
        }
    }

    printf("Select the difficulty level (easy -> 3, hard -> 2, really really but really impossible -> 1): ");
    scanf("%d", &d);
    ask(d);

    //We created 2d array with dynamic allocation.
    board = (char **)malloc(boardSize * sizeof(char *));
    for (int i = 0; i < boardSize; i++) {
        board[i] = (char *)malloc(boardSize * sizeof(char));
    }

    char winner = ' ';
    char response;
    // Gameplay loop
    do { // when player wants to play again
        winner = ' ';
        response = ' ';
        resetBoard();
        while (winner == ' ' && checkFreeSpaces() != 0) {
            printBoard();

            playerMove();
            winner = checkWinner();
            if (winner != ' ' || checkFreeSpaces() == 0) { // check if  player won
                break;
            }

            computerMove();
            winner = checkWinner();
            if (winner != ' ' || checkFreeSpaces() == 0) { // check if  computer won
                break;
            }
        }

        printBoard();
        printWinner(winner);

        printf("Do you want to play again? (Y/N): \n");
        while (getchar() != '\n'); // clears the temporary and take a correct input
        scanf("%c", &response);

    } while (response == 'Y' || response == 'y');

    // Free memory when game is over.
    for (int i = 0; i < boardSize; i++) {
        free(board[i]);
    }

    free(board);

    return 0;
}
void ask(int d) {
    switch (d) {
            case 1:
                printf("You selected difficulty level: impossible\n");

                break;
            case 2:
                printf("You selected difficulty level: hard\n");
                break;
            case 3:
                printf("You selected difficulty level: easy\n");
                break;
            default:
                printf("Invalid input. Please select 1, 2, or 3.\n");
                scanf("%d",&d);
                ask(d); //recursion until valid input.
    }
}
void printBoard() {
    for (int i = 0; i < boardSize; i++) {
        for (int j = 0; j < boardSize; j++) {
            // The system is: (Background color ___ text color _____ text _____ reset)
            if (board[i][j] == 'X') {
                printf(WHITE_BG RED " %c " RESET , board[i][j]);
            } else if (board[i][j] == 'O') {
                printf(WHITE_BG BLUE " %c " RESET, board[i][j]);
            } else {
                printf(WHITE_BG "   " RESET);
            }
            if (j < boardSize - 1) {
                printf(WHITE_BG BLACK_TEXT "|" RESET );
            }
        }
        printf("\n");
        if (i < boardSize - 1) {
            for (int k = 0; k < boardSize; k++) {
                printf(WHITE_BG BLACK_TEXT "---" RESET );
                if (k < boardSize - 1) {
                    printf(WHITE_BG BLACK_TEXT "|" RESET );
                }
            }
            printf("\n");
        }
    }
    printf(RESET); // we reset all the color
}

void resetBoard() {
    for (int i = 0; i < boardSize; i++) {
        for (int j = 0; j < boardSize; j++) {
            board[i][j] = ' ';
        }
    }
}

int checkFreeSpaces() {
    int freeSpaces = boardSize * boardSize;
    for (int i = 0; i < boardSize; i++) {
        for (int j = 0; j < boardSize; j++) {
            if (board[i][j] != ' ') {
                freeSpaces--;
            }
        }
    }
    return freeSpaces;
}

void playerMove() {
    int x, y;
    do {
        printf("Enter row number (between 1-%d) or (-1 -> undo , -2 ->redo): ", boardSize);
        while(scanf("%d", &x) !=1) {
            printf("Invalid move! Try again. \n");
            printf("Enter row number (between 1-%d) or (-1 -> undo , -2 ->redo): ", boardSize);
            while(getchar() != '\n');
        }

        if(x==-1) {
            undoMove(board, undoStack, redoStack, &undoTop, &redoTop, boardSize);
            printBoard();
            continue;
        }
        if(x==-2) {
            redoMove(board, undoStack, redoStack, &undoTop, &redoTop, boardSize);
            printBoard();
            continue;
        }
        printf("Enter column number (between 1-%d): ", boardSize);
        scanf("%d", &y);
        x--;
        y--;
        //we hatch the index because indexes starts from zero but users may not know this info.

        if (x < 0 || x >= boardSize || y < 0 || y >= boardSize || board[x][y] != ' ') {
            printf("Invalid move! Try again. \n");
        } else {
            board[x][y] = PLAYER;
            undoStack[++undoTop] = x * boardSize + y; // with that our 2d matrix become 1d matrix
            redoTop = -1; // Clear redo stack.
            break;
        }
    } while (true);
}

void computerMove() {
    int luckyToFind;
    int luckyToBlock = rand() % d;
    bool bools = true;
    if(d == 1){
        luckyToFind = rand() % 1;
    }
    else{
        luckyToFind = rand() % (d - 1);
    }
     int x, y;

    // Check to win
    printf("luckyToFind value is %d and luckyToBlock value is %d\n", luckyToFind, luckyToBlock);
    if(luckyToFind == 0) { // Possibility controller
        for (int i = 0; i < boardSize; i++) {
            for (int j = 0; j < boardSize; j++) {
                if (board[i][j] == ' ') { // Search blank space in the game board.
                    board[i][j] = COMPUTER;
                    if (checkWinner() == COMPUTER) {
                        undoStack[++undoTop] = i * boardSize + j; // Save the last move.
                        redoTop = -1; // Reset the redo stack
                        return;
                    }
                    board[i][j] = ' '; // Undo the move
                }
            }
        }
    }

    // Check to block


    if(luckyToBlock == 0) {
        // Try to block the user's win
        for (int i = 0; i < boardSize; i++) {
            for (int j = 0; j < boardSize; j++) {
                if (board[i][j] == ' ') {
                    // Simulate the user movement temporarily.
                    board[i][j] = PLAYER;
                    if (checkWinner() == PLAYER) { // If we find any possibilities to win then block the user.
                        board[i][j] = COMPUTER; // Make the move to block user
                        undoStack[++undoTop] = i * boardSize + j; // Save the movement
                        redoTop = -1; // Reset redo
                        return; // Exit the statement.
                    }
                    board[i][j] = ' '; // Undo the movement.
                }
            }
        }
    }

    // If no possibilities to block or win, check specific strategies
    if (d == 1) { // Activate the impossible mode
        if (board[1][1] == ' ') {
            board[1][1] = COMPUTER;
            undoStack[++undoTop] = 1 * boardSize + 1; // Save the move
            redoTop = -1;
            return;
        }
        else if (board[1][1] == COMPUTER) {
            if (board[0][1] != ' ' && board[1][2] != ' ' && board[2][1] != ' ' && board[1][0] != ' ' )
            {
                     do {
                    x = rand() % boardSize;
                    y = rand() % boardSize;
                }while (board[x][y] != ' ');
                board[x][y] = COMPUTER;
                undoStack[++undoTop] = x * boardSize + y; // Save the movement
                redoTop = -1; // Reset redo
            }
            else {
            while (bools) {
                int x = rand() % 4;

                switch (x) {
                    case 0:
                        if (board[0][1] == ' ') {
                            board[0][1] = COMPUTER;
                            undoStack[++undoTop] = 0 * boardSize + 1;
                            bools = false;
                        }
                        break;
                    case 1:
                        if (board[1][2] == ' ') {
                            board[1][2] = COMPUTER;
                            undoStack[++undoTop] = 1 * boardSize + 2;
                            bools = false;
                        }
                        break;
                    case 2:
                        if (board[2][1] == ' ') {
                            board[2][1] = COMPUTER;
                            undoStack[++undoTop] = 2 * boardSize + 1;
                            bools = false;
                        }
                        break;
                    default:
                        if (board[1][0] == ' ') {
                            board[1][0] = COMPUTER;
                            undoStack[++undoTop] = (1) * boardSize + (0);
                            bools = false;
                        }
                        break;
                }
            }
            }
            redoTop = -1;
            return;
        }

        //If player starts  from the middle computer will  perform these movements in order.
        else if(board[1][1] == PLAYER) {
             if (board[0][0] == ' ') {
                            board[0][0] = COMPUTER;
                            undoStack[++undoTop] = 0 * boardSize + 0;
                        }
            else if (board[2][2] == ' ') {
                            board[2][2] = COMPUTER;
                            undoStack[++undoTop] = 2 * boardSize + 2;
                        }
            else if (board[0][2] == ' ') {
                            board[0][2] = COMPUTER;
                            undoStack[++undoTop] = 0 * boardSize + 2;
                        }
            else if (board[2][0] == ' ') {
                            board[2][0] = COMPUTER;
                            undoStack[++undoTop] = 2 * boardSize + 0;
                        }
        }
    }
    else {
                do {
                    x = rand() % boardSize;
                    y = rand() % boardSize;
                }while (board[x][y] != ' ');
                board[x][y] = COMPUTER;
                undoStack[++undoTop] = x * boardSize + y; // Save the movement
                redoTop = -1; // Reset redo
            }
    // Play randomly if no specific strategy is applicable


}

char checkWinner() {
    // Check rows
    for (int i = 0; i < boardSize; i++) {
        bool rowCheck = true;
        for (int j = 1; j < boardSize; j++) {
            if (board[i][j] != board[i][0] || board[i][j] == ' ') {
                rowCheck = false;
                break;
            }
        }
        if (rowCheck) {
            return board[i][0];
        }
    }

    // Check columns
    for (int i = 0; i < boardSize; i++) {
        bool columnCheck = true;
        for (int j = 1; j < boardSize; j++) {
            if (board[j][i] != board[0][i] || board[j][i] == ' ') {
                columnCheck = false;
                break;
            }
        }
        if (columnCheck) {
            return board[0][i];
        }
    }

    // Check main diagonal
    bool diagonalCheck = true;
    for (int i = 1; i < boardSize; i++) {
        if (board[i][i] != board[0][0] || board[i][i] == ' ') {
            diagonalCheck = false;
            break;
        }
    }
    if (diagonalCheck) {
        return board[0][0];
    }

    // Check reverse diagonal
    diagonalCheck = true;
    for (int i = 1; i < boardSize; i++) {
        if (board[i][boardSize - i - 1] != board[0][boardSize - 1] || board[i][boardSize - i - 1] == ' ') {
            diagonalCheck = false;
            break;
        }
    }
    if (diagonalCheck) {
        return board[0][boardSize - 1];
    }

    return ' '; // No winner
}
void printWinner(char winner) {
    if (winner == PLAYER) {
        printf("YOU WIN!\n");
    } else if (winner == COMPUTER) {
        printf("YOU LOSE!\n");
    } else {
        printf("IT'S A DRAW!\n");
    }

}
void undoMove(char **board, int *undoStack, int *redoStack, int *undoTop, int *redoTop, int boardSize) {
    for (int i = 0; i < 2; i++) {
        if (*undoTop == -1) { // it means undotop is empty because we started -1
            printf("No more moves to undo!\n");
            return;
        }
        int lastMove = undoStack[(*undoTop)--]; // it removes the last element from undostack, and after that index decrease by 1
        redoStack[++(*redoTop)] = lastMove;// First the index is increased by 1, and then the element removed by undoTop is added to redoTop
        board[lastMove / boardSize][lastMove % boardSize] = ' '; //
    }
}

void redoMove(char **board, int *undoStack, int *redoStack, int *undoTop, int *redoTop, int boardSize) {

    if (*redoTop == -1) {
        printf("No moves to redo!\n");
        return;
    } // The last element always will be PLAYER. That is why we firstly take the Player's move to undotop.
    int lastMove = redoStack[(*redoTop)--];
    undoStack[++(*undoTop)] = lastMove;
    board[lastMove / boardSize][lastMove % boardSize] = PLAYER;
    if (*redoTop == -1) {
        printf("No moves to redo!\n");
        return;
    } //then also we take computer's  movement back.
     lastMove = redoStack[(*redoTop)--];
    undoStack[++(*undoTop)] = lastMove;
    board[lastMove / boardSize][lastMove % boardSize]=COMPUTER;

}
