#include "game.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
 
#define POINTSTRSIZE 5
#define MAXBUF 1024
/************关于本文档********************************************
*filename: ssl-server.c
*purpose: 演示利用 OpenSSL 库进行基于 IP层的 SSL 加密通讯的方法，这是服务器端例子
*********************************************************************/
int main(int argc, char **argv)
{
    char point[POINTSTRSIZE+1];
    int sockfd, new_fd;
    socklen_t len;
    struct sockaddr_in my_addr, their_addr;
    unsigned int myport, lisnum;
    char buf[MAXBUF + 1];
    SSL_CTX *ctx;
 
    if (argv[1])
        myport = atoi(argv[1]);
    else
        myport = 7838;
 
    if (argv[2])
        lisnum = atoi(argv[2]);
    else
        lisnum = 2;
 
    /* SSL 库初始化 */
    SSL_library_init();
    /* 载入所有 SSL 算法 */
    OpenSSL_add_all_algorithms();
    /* 载入所有 SSL 错误消息 */
    SSL_load_error_strings();
   /* 以 SSL V2 和 V3 标准兼容方式产生一个 SSL_CTX ，即 SSL Content Text */
    ctx = SSL_CTX_new(SSLv23_server_method());
    /* 也可以用 SSLv2_server_method() 或 SSLv3_server_method() 单独表示 V2 或 V3标准 */
    if (ctx == NULL) {
        ERR_print_errors_fp(stdout);
        exit(1);
    }
    /* 载入用户的数字证书， 此证书用来发送给客户端。 证书里包含有公钥 */
    if (SSL_CTX_use_certificate_file(ctx, argv[4], SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stdout);
        exit(1);
    }
    /* 载入用户私钥 */
    if (SSL_CTX_use_PrivateKey_file(ctx, argv[5], SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stdout);
        exit(1);
    }
    /* 检查用户私钥是否正确 */
    if (!SSL_CTX_check_private_key(ctx)) {
        ERR_print_errors_fp(stdout);
        exit(1);
    }
 
    /* 开启一个 socket 监听 */
    if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    } else
        printf("socket created\n");
 
    bzero(&my_addr, sizeof(my_addr));
    my_addr.sin_family = PF_INET;
    my_addr.sin_port = htons(myport);
    if (argv[3])
        my_addr.sin_addr.s_addr = inet_addr(argv[3]);
    else
        my_addr.sin_addr.s_addr = INADDR_ANY;
 
    if (bind(sockfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr))
        == -1) {
        perror("bind");
        exit(1);
    } else
        printf("binded\n");
 
    if (listen(sockfd, lisnum) == -1) {
        perror("listen");
        exit(1);
    } else
        printf("begin listen\n");
 
    while (1) {
        SSL *ssl;
        len = sizeof(struct sockaddr);
        /* 等待客户端连上来 */
        if ((new_fd =
             accept(sockfd, (struct sockaddr *) &their_addr,
                    &len)) == -1) {
            perror("accept");
            exit(errno);
        } else
            printf("server: got connection from %s, port %d, socket %d\n",
                   inet_ntoa(their_addr.sin_addr),
                   ntohs(their_addr.sin_port), new_fd);
 
        /* 基于 ctx 产生一个新的 SSL */
        ssl = SSL_new(ctx);
        /* 将连接用户的 socket 加入到 SSL */
        SSL_set_fd(ssl, new_fd);
        /* 建立 SSL 连接 */
        if (SSL_accept(ssl) == -1) {
            perror("accept");
            close(new_fd);
            break;
        }
 
        /* 开始处理每个新连接上的数据收发 */
        bzero(buf, MAXBUF + 1);
        strcpy(buf, "server->client");
        /* 发消息给客户端 */
        len = SSL_write(ssl, buf, strlen(buf));
 
        if (len <= 0) {
            printf
                ("消息'%s'发送失败！错误代码是%d，错误信息是'%s'\n",
                 buf, errno, strerror(errno));
            goto finish;
        } else
            printf("消息'%s'发送成功，共发送了%d个字节！\n",
                   buf, len);
 
        bzero(buf, MAXBUF + 1);
        /* 接收客户端的消息 */
        len = SSL_read(ssl, buf, MAXBUF);
        if (len > 0)
            printf("接收消息成功:'%s'，共%d个字节的数据\n",
                   buf, len);
        else
            printf
                ("消息接收失败！错误代码是%d，错误信息是'%s'\n",
                 errno, strerror(errno));
        /* 处理每个新连接上的数据收发结束 */

        for(;;)
        {
            ssize_t s;
            Point point_server,point_client;
            char board[ROWS][COLS];
            InitBoard(board,ROWS,COLS);
            int x,y;
            while(1){
                s = SSL_read(ssl,(void*)&point_client,sizeof(point_client));
                if(s == 0)
                {
                    printf("Client quit...\n");
                    close(sockfd);
                }
                ClientMove(board,&point_client);
                PrintBoard(board,ROWS,COLS);
                if(GameState(board,&point_client) == 'X')
                {
                    printf("Client win!\n");
                    break;
                }
                else if(GameState(board,&point_client) == 'p')
                {
                    printf("Draw!\n");
                    break;
                }
                printf("请下子(Please enter coordinates：x,y)> ");
                input(point,POINTSTRSIZE);
                int i = 0;
                int tmp;
                char* scanpointx = point;
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
                point_server.row = x-1;
                point_server.col = y-1;
                ServerMove(board,&point_server);
                PrintBoard(board,ROWS,COLS);
                len = write(ssl,(void*)&point_server,sizeof(point_server));
                if(GameState(board,&point_client) == 'O')
                {
                    printf("You win!\n");
                    break;
                }
                else if(GameState(board,&point_client) == 'p')
                {
                    printf("Draw!\n");
                    break;
                }

            }/*game over*/
        }
 
      finish:
        /* 关闭 SSL 连接 */
        SSL_shutdown(ssl);
        /* 释放 SSL */
        SSL_free(ssl);
        /* 关闭 socket */
        close(new_fd);
    }

    
    /* 关闭监听的 socket */
    close(sockfd);
    /* 释放 CTX */
    SSL_CTX_free(ctx);
    return 0;
}
