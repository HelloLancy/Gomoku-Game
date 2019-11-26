#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define ROWS 10
#define COLS 10


typedef struct Point{
    int row;
    int col;
}Point;

/*
Initialize the board*/
void InitBoard(char board[ROWS][COLS],int row, int col);

/*Print board*/
void PrintBoard(char board[ROWS][COLS], int row, int col);

/*Moving together*/
void CommonMove(char board[ROWS][COLS],Point* point,char ch);

/*Client movement*/
void ClientMove(char board[ROWS][COLS],Point* point);

/*
Server move*/
void ServerMove(char board[ROWS][COLS],Point* point);


/*Judging win or lose*/
char GameState(char board[ROWS][COLS],Point* point);

/*Secure input file*/
void input(char* string,int length);