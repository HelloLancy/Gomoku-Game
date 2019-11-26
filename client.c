#include "game.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>  
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define POINTSTRSIZE 5

void Menu()
{
    printf("==================================\n");
    printf("||  1.Start   0.Exit     ||\n");
    printf("==================================\n");
}
void Game(int sock)
{
    Point point_server,point_client;
    char board[ROWS][COLS];
    char scanpoint[POINTSTRSIZE+1];
    char* scanpointx;
    char* scanpointy;

    InitBoard(board,ROWS,COLS);
    PrintBoard(board,ROWS,COLS);
    ssize_t s;
    int x,y;
    while(1){
        printf("请下子(Please enter coordinates：x,y)> ");
        scanpoint[strlen(scanpoint)-1] = '\0';
        input(scanpoint,6);
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

        point_client.row = x-1;
        point_client.col = y-1;
        ClientMove(board,&point_client);
        PrintBoard(board,ROWS,COLS);
        write(sock,(void*)&point_client,sizeof(point_client));
        if(GameState(board,&point_client) == 'X')
        {
            printf("You win!\n");
            break;
        }
        else if(GameState(board,&point_client) == 'p')
        {
            printf("Draw!\n");
            break;
        }
        s = read(sock,(void*)&point_server,sizeof(point_server));
        if(s == 0)
        {
            printf("Server quit\n");
            return;
        }
        ServerMove(board,&point_server);
        PrintBoard(board,ROWS,COLS);
        if(GameState(board,&point_server) == 'O')
        {
            printf("Server win!\n");
            break;
        }
        else if(GameState(board,&point_server) == 'p')
        {
            printf("Draw!\n");
            break;
        }
    }
}

int main(int argc, char* argv[])
{
    char *errstr;
    if(argc != 3)
    {
        printf("Usage: ./server [ip] [port]\n");
        exit(1);
    }
    
    int sock = socket(AF_INET,SOCK_STREAM,0);
    if(sock < 0)
    {
        perror("socket");
        exit(1);
    }

    struct sockaddr_in client;
    client.sin_family = AF_INET;
    client.sin_addr.s_addr = inet_addr(argv[1]);
    client.sin_port = htons(strtol(argv[2],&errstr,10));
    int ret = connect(sock,(const struct sockaddr*)&client,sizeof(client));
    if(ret < 0)
    {
        perror("connect");
        exit(2);
    }
    Menu();
    int state;
    char scanstate[2];
    while(1)
    {
        printf("Please make your choice> ");
        input(scanstate,2);
        state = strtol(scanstate,&errstr,10);
        scanstate[strlen(scanstate)-1]='\0';
        /*scanf("%d",&state);*/
        if(state == 1)
        {
            Game(sock);
        }else if(state == 0)
        {
            break;
        }
        else
        {
            printf("Wrong selection\n");
            Menu();
            sleep(2);
        }
    }

    close(sock);
    return 0;
}
