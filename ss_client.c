#include "game.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>  
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>

#include <string.h>
#include <errno.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <resolv.h>
#include <stdio.h>
#include <sys/stat.h>
#define MAXBUF 1024

void ShowCerts(SSL * ssl)
{
    X509 *cert;
    char *line;
 
    cert = SSL_get_peer_certificate(ssl);
    if (cert != NULL) {
        printf("Digital certificate information:\n");
        line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
        printf("certificate: %s\n", line);
        free(line);
        line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
        printf("Issuer: %s\n", line);
        free(line);
        X509_free(cert);
    } else
        printf("No certificate information！\n");
}

void Menu()
{
    printf("==================================\n");
    printf("||  1.Start   0.Exit     ||\n");
    printf("==================================\n");
}
void Game(SSL *ssl)
{
    Point point_server,point_client;
    char board[ROWS][COLS];
    InitBoard(board,ROWS,COLS);
    PrintBoard(board,ROWS,COLS);
    ssize_t s;
    int x,y;
    while(1){
        printf("请下子(Please enter coordinates：x,y)> ");
        scanf("%d%d",&x,&y);
        point_client.row = x-1;
        point_client.col = y-1;
        ClientMove(board,&point_client);
        PrintBoard(board,ROWS,COLS);
        SSL_write(ssl,(void*)&point_client,sizeof(point_client));
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

int main(int argc, char* argv[])
{
	SSL_CTX *ctx;
    SSL *ssl;
    if(argc != 3)
    {
        printf("Usage: ./server [ip] [port]\n");
        exit(1);
    }
    
	/* SSL library initialization, see ssl-server.c  */
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    ctx = SSL_CTX_new(SSLv23_client_method());
    if (ctx == NULL) {
        ERR_print_errors_fp(stdout);
        exit(1);
    }
	
    int sock = socket(AF_INET,SOCK_STREAM,0);
    if(sock < 0)
    {
        perror("socket");
        exit(1);
    }

    struct sockaddr_in client;
    bzero(&client, sizeof(client));
    client.sin_family = AF_INET;
    client.sin_addr.s_addr = inet_addr(argv[1]);
    client.sin_port = htons(atoi(argv[2]));
    if (inet_aton(argv[1], (struct in_addr *) &client.sin_addr.s_addr) == 0) {
        perror(argv[1]);
        exit(errno);
    }
	
    int ret = connect(sock,(const struct sockaddr*)&client,sizeof(client));
    if(ret < 0)
    {
        perror("connect");
        exit(2);
    }
	
	/* Generate a new SSL based on ctx */
    ssl = SSL_new(ctx);
    SSL_set_fd(ssl, sock);
    /* Establish an SSL connection */
    if (SSL_connect(ssl) == -1)
        ERR_print_errors_fp(stderr);
    else {
        printf("Connected with %s encryption\n", SSL_get_cipher(ssl));
        ShowCerts(ssl);
    }	
	
    Menu();
    int state;
    while(1)
    {
        printf("Please make your choice> ");
        scanf("%d",&state);
        if(state == 1)
        {
            /*Game(sock);*/
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

    /* close SSL 连接 */
			SSL_shutdown(ssl);
			/* release SSL */
			SSL_free(ssl);
			/* close socket */
			close(sock);
			SSL_CTX_free(ctx);
    return 0;
}
