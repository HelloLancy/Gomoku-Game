#include "game.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>  
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>

#define POINTSTRSIZE 5

/*Initialize the board*/
void InitBoard(char board[ROWS][COLS],int row, int col)
{
    memset(board,' ',row*col*sizeof(char));
}
/*Print board*/
void PrintBoard(char board[ROWS][COLS],int row,int col)
{
    int  i = 0;
    printf("    1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10\n");
    for(; i < row;i++)
    {
        printf("%2d ",i+1);
        int j = 0;
        for(; j < col;j++)
        {
            if( j == col-1 )
            {
                printf(" %c \n",board[i][j]);
                if(i != row-1)
                    printf("   ---|---|---|---|---|---|---|---|---|---\n");
                break;
            }
            printf(" %c ",board[i][j]);
            printf("|");
        }

    }
}

/*Determine if the board is full*/
int IsFall(char board[ROWS][COLS])
{
    size_t i = 0;
    for(; i < ROWS;i++)
    {
        size_t j = 0;
        for(; j < COLS; j++)
        {
            if(board[i][j] == ' ')
                return 0;
        }
    }
    return 1;
}
/*Move*/
void CommonMove(char board[ROWS][COLS],Point* point,char ch)
{
    char scanpoint[POINTSTRSIZE+1];
    char *errstr;
    char *scanpointx;

    if(point == NULL)
    {
        return;
    }
    int x = point->row;
    int y = point->col;
    /*If the board is full, it will exit the loop, that is, it can no longer be played.*/
    while(1)
    {
        if(x >= 0 && x < ROWS && y >= 0 && y < COLS){
            if(board[x][y] == ' ')
            {
                board[x][y] = ch;
                point->row = x;
                point->col = y;
                break;
            }
            else
            {
                printf("The coordinate input is incorrect. Please re-enter the coordinates: ");
                input(scanpoint,POINTSTRSIZE);
                if (strlen(scanpoint) == 0){
                    input(scanpoint,6);
                }
                int i = 0;
                int tmp;
                scanpointx = scanpoint;
                while(tmp =strtol(scanpointx, &scanpointx, 10)){
                    if(i ==0){
                        x = tmp;
                    } else if (i==1){
                        y = tmp;
                        break;
                    }
                    i++;
                }
                /*scanf("%d%d",&x,&y);*/
                x--;
                y--;
            }
        }
        else{
            printf("The coordinate input is incorrect. Please re-enter the coordinates: ");
            input(scanpoint,POINTSTRSIZE);
            if (strlen(scanpoint) == 0){
                input(scanpoint,6);
            }
            int i = 0;
            int tmp;
            scanpointx = scanpoint;
            while(tmp =strtol(scanpointx, &scanpointx, 10)){
                if(i ==0){
                    x = tmp;
                } else if (i==1){
                    y = tmp;
                    break;
                }
                i++;
            }
            /*scanf("%d%d",&x,&y);*/
            x--;
            y--;
        }
    }
}
/*Client movement*/
void ClientMove(char board[ROWS][COLS],Point* point)
{
    CommonMove(board,point,'X');
}

/*Server Movement*/
void ServerMove(char board[ROWS][COLS],Point* point)
{
    CommonMove(board,point,'O');
}
/*List*/
int ColState(char board[ROWS][COLS],Point* point)
{
    int x = point->row;
    int y = point->col;
    int count = 1;
    while(x-1 >= 0)
    {
        if(board[x][y] != board[x-1][y])
            break;
        count++;
        if(count == 5)
        {
            /*If you traverse up enough, it means that you have won, and you can return directly*/
            return count;
        }
        x--;
    }
    /*
    Here to explain that the current point is not up to the conditions
    But it has reached the top point, so look for it from the critical point to the opposite direction.
    Must find five consecutive cases to meet the conditions*/
    count = 1;/*count reset to 1*/
    while(x+1 <= ROWS)
    {
        if(board[x][y] == board[x+1][y])
        {
            count++;
            if(count == 5)
            {
                return count;
            }
            x++;
        }
        else
        {
            /*n represents the condition that the current state does not satisfy the win*/
            return 0;
        }
    }
    return 0;
}
/*
Line*/
int RowState(char board[ROWS][COLS],Point* point)
{
    int x = point->row;
    int y = point->col;
    int count = 1;
    while(y-1 >= 0)
    {
        if(board[x][y] != board[x][y-1])
            break;
        count++;
        if(count == 5)
        {
            /*
If you have traversed to the left, it is enough for five, indicating that you have won, return directly*/
            return count;
        }
        y--;
    }
    /*
Here to explain that the current point to the left is not satisfied*/
    /*But it has reached the leftmost point, so look for it from the critical point to the opposite direction.
    /*Must find five consecutive cases to meet the conditions*/
    count = 1;/*count reset to 1*/
    while(y+1 <= COLS)
    {
        if(board[x][y] == board[x][y+1])
        {
            count++;
            if(count == 5)
            {
                return count;
            }
            y++;
        }
        else
        {
            /*n Represents the condition that the current state does not satisfy the win*/
            return 0;
        }
    }
    return 0;
}
/*Diagonal from top left to bottom right*/
int UpLeftState(char board[ROWS][COLS],Point* point)
{
    int x = point->row;
    int y = point->col;
    int count = 1;
    while(x-1 >= 0 && y-1 >= 0 )
    {
        if(board[x][y] != board[x-1][y-1])
            break;
        count++;
        if(count == 5)
        {
            /*
If you have traversed to the left, it is enough for five, indicating that you have won, return directly*/
            return count;
        }
        x--;
        y--;
    }
    /*
It is here that the current point to the left is not satisfied.*/
    /* But it has reached the top left point, so look for it from the critical point*/
    /* must find five consecutive cases to meet the conditions*/
    count = 1;/*count reset to 1*/
    while(x+1 <= ROWS && y+1 <= COLS)
    {
        if(board[x][y] == board[x+1][y+1])
        {
            count++;
            if(count == 5)
            {
                return count;
            }
            x++;
            y++;
        }
        else
        {
            /*n represents the condition that the current state does not satisfy the win*/
            return 0;
        }
    }
    return 0;
}
/*Right upper to lower left diagonal*/
int UpRightState(char board[ROWS][COLS], Point* point)
{
    int x = point->row;
    int y = point->col;
    int count = 1;
    while(x-1 >= 0 && y+1 <= COLS)
    {
        if(board[x][y] != board[x-1][y+1])
            break;
        count++;
        if(count == 5)
        {
            /* If there are enough five traversals to the upper right, it means that you have won, return directly.*/
            return count;
        }
        x--;
        y++;
    }
    /*Here to explain that the current point to the upper right is not satisfied with the conditions * /
    /* But it has reached the top right point, so look for it from the critical point in the opposite direction*/
    /* Must find five consecutive cases to meet the conditions*/
    count = 1;/*count Reset to 1 */
    while(x+1 <= ROWS && y-1 <=COLS)
    {
        if(board[x][y] == board[x+1][y-1])
        {
            count++;
            if(count == 5)
            {
                return count;
            }
            x++;
            y--;
        }
        else
        {
            /*n  Represents the condition that the current state does not satisfy the win*/
            return 0;
        }
    }
    return 0;
}
/*
Return to the game progress status*/
char GameState(char board[ROWS][COLS],Point* point)
{
    if(point == NULL)
    {
        return 'e';
    }
    /*Determine whether the current point is in the column of five consecutive children*/
    if(ColState(board,point) == 5)
    {
        return board[point->row][point->col];
    }
    /*Determine if the current point is a row of five children*/
    else if(RowState(board,point) == 5)
    {
        return board[point->row][point->col];
    }
    /*
Determine whether the diagonal of the upper left and lower right of the current point satisfies the condition*/
    else if(UpLeftState(board,point) == 5)
    {
        return board[point->row][point->col];
    }
    /*
Determine whether the diagonal of the upper left and lower right of the current point satisfies the condition*/
    else if(UpRightState(board,point) == 5)
    {
        return board[point->row][point->col];
    }
    else if(IsFall(board))
    {
        /*Draw*/
        return 'p'; 
    }
    /*There is no win or loss at present, continue the game*/
    return 'g';
}

/*Secure input file*/
void input(char *string,int length)
{
    int x;
    fgets(string,length,stdin);
    for(x=0;x<=length;x++)
    {
        if( string[x] == '\n')
        {
            string[x] = '\0';
            break;
        }
    }
}