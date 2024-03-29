#include "game.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <resolv.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#define MAXBUF 1024

#define POINTSTRSIZE 5

void Menu()
{
    printf("==================================\n");
    printf("||  1.Start   0.Exit     ||\n");
    printf("==================================\n");
}

void ShowCerts(SSL * ssl)
{
    X509 *cert;
    char *line;
 
    cert = SSL_get_peer_certificate(ssl);
    if (cert != NULL) {
        printf("数字证书信息:\n");
        line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
        printf("证书: %s\n", line);
        free(line);
        line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
        printf("颁发者: %s\n", line);
        free(line);
        X509_free(cert);
    } else
        printf("无证书信息！\n");
}

void Game(SSL *ssl)
{
    Point point_server,point_client;
    char board[ROWS][COLS];
    char scanpoint[POINTSTRSIZE+1];
    char* scanpointx;
    char* scanpointy;

    InitBoard(board,ROWS,COLS);
    PrintBoard(board,ROWS,COLS);
    ssize_t s;
    int x,y, len;
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
        len = SSL_write(ssl,(void*)&point_client,sizeof(point_client));
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
        s = SSL_read(ssl,(void*)&point_server,sizeof(point_server));
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

/************关于本文档********************************************
*filename: ssl-client.c
*purpose: 演示利用 OpenSSL 库进行基于 IP层的 SSL 加密通讯的方法，这是客户端例子
*********************************************************************/
int main(int argc, char **argv)
{
    int sockfd, len;
    struct sockaddr_in dest;
    char buffer[MAXBUF + 1];
    char *errstr;
    SSL_CTX *ctx;
    SSL *ssl;
 
    if (argc != 3) {
        printf
            ("参数格式错误！正确用法如下：\n\t\t%s IP地址 端口\n\t比如:\t%s 127.0.0.1 80\n此程序用来从某个 IP 地址的服务器某个端口接收最多 MAXBUF 个字节的消息",
             argv[0], argv[0]);
        exit(0);
    }
 
    /* SSL 库初始化，参看 ssl-server.c 代码 */
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    ctx = SSL_CTX_new(SSLv23_client_method());
    if (ctx == NULL) {
        ERR_print_errors_fp(stdout);
        exit(1);
    }
 
    /* 创建一个 socket 用于 tcp 通信 */
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket");
        exit(errno);
    }
    printf("socket created\n");
 
    /* 初始化服务器端（对方）的地址和端口信息 */
    bzero(&dest, sizeof(dest));
    dest.sin_family = AF_INET;
    dest.sin_port = htons(atoi(argv[2]));
    if (inet_aton(argv[1], (struct in_addr *) &dest.sin_addr.s_addr) == 0) {
        perror(argv[1]);
        exit(errno);
    }
    printf("address created\n");
 
    /* 连接服务器 */
    if (connect(sockfd, (struct sockaddr *) &dest, sizeof(dest)) != 0) {
        perror("Connect ");
        exit(errno);
    }
    printf("server connected\n");
 
    /* 基于 ctx 产生一个新的 SSL */
    ssl = SSL_new(ctx);
    SSL_set_fd(ssl, sockfd);
    /* 建立 SSL 连接 */
    if (SSL_connect(ssl) == -1)
        ERR_print_errors_fp(stderr);
    else {
        printf("Connected with %s encryption\n", SSL_get_cipher(ssl));
        ShowCerts(ssl);
    }
 
    /* 接收对方发过来的消息，最多接收 MAXBUF 个字节 */
    bzero(buffer, MAXBUF + 1);
    /* 接收服务器来的消息 */
    len = SSL_read(ssl, buffer, MAXBUF);
    if (len > 0)
        printf("接收消息成功:'%s'，共%d个字节的数据\n",
               buffer, len);
    else {
        printf
            ("消息接收失败！错误代码是%d，错误信息是'%s'\n",
             errno, strerror(errno));
        goto finish;
    }
    bzero(buffer, MAXBUF + 1);
    strcpy(buffer, "from client->server");
    /* 发消息给服务器 */
    len = SSL_write(ssl, buffer, strlen(buffer));
    if (len < 0)
        printf
            ("消息'%s'发送失败！错误代码是%d，错误信息是'%s'\n",
             buffer, errno, strerror(errno));
    else
        printf("消息'%s'发送成功，共发送了%d个字节！\n",
               buffer, len);
    
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
            Game(ssl);
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

  finish:
    /* 关闭连接 */
    SSL_shutdown(ssl);
    SSL_free(ssl);
    close(sockfd);
    SSL_CTX_free(ctx);
    return 0;

}