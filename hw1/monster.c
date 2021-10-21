#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void printboard(char** board, int boardX, int boardY){
    for (int i = boardY-1; i >= 0; i--){
        for (int j = 0; j < boardX; j++){
            printf("%c ", board[i][j]);
        }
        putchar('\n');
    }
    return;
}

void plrmove(char** board, int X, int Y, char move){
    
}
int main (int argc, char* argv[]){
    if (argc!=9) return 0;
    int boardX = atoi(argv[1]);
    int boardY = atoi(argv[2]);
    int plrX = atoi(argv[3]);
    int plrY = atoi(argv[4]);
    int goalX = atoi(argv[5]);
    int goalY = atoi(argv[6]);
    int monX = atoi(argv[7]);
    int monY = atoi(argv[8]);

    char** board = malloc(boardY * sizeof(char*));
    for (int i = 0; i < boardY; i++){
        board[i] = (char*) malloc(boardX * sizeof(char));
    }
    for (int i = 0; i < boardY; i++){
        for (int j = 0; j < boardX; j++){
            board[i][j] = '.';
        }
    }

    board[goalY][goalX] = 'G';    
    board[plrY][plrX] = 'P';
    board[monY][monX] = 'M';

    printboard(board, boardX, boardY);

    /*char cmd[255];
    char move;
    while(fgets(cmd, 255, stdin) != NULL){
        //if ((plrX != goalX && plrY != goalY && monX != plrX && monY != plrY)) break;
        move = cmd[0];
        switch (move) {
        case 'W':
            if (plrY >= 0){
                board[plrY][plrX] = '.';
                board[plrY--][plrX] = 'P';
            } 
        case 'S':
            if (plrY < boardY){
                board[plrY][plrX] = '.';
                board[plrY++][plrX] = 'P';
            } 
        case 'A':
            if (plrX >= 0) plrX--;
        case 'D':
            if (plrX <boardX) plrX++;
        default:
            break;
        }
        printboard(board, boardX, boardY);
    }*/

    for (int i = 0; i < boardY; i++) free(board[i]);
    free(board);
}