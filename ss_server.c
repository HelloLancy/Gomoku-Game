#include "game.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>  
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#define MAXBUF 1024


int main(int argc, char* argv[])
{
    /*if(argc != 3)
    {
        printf("Usage: ./server [ip] [port]\n");
        exit(1);
    }
    */
    int sock = socket(AF_INET,SOCK_STREAM,0);
    if(sock < 0)
    {
        perror("socket");
        exit(1);
    }

	SSL_CTX *ctx;
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(argv[1]);
    server.sin_port = htons(atoi(argv[2]));
	
    unsigned int myport, lisnum;
	
	  if (argv[1])
        myport = atoi(argv[1]);
    else
        myport = 10002;
 
    if (argv[2])
        lisnum = atoi(argv[2]);
    else
        lisnum = 2;
 
	/*add SSL*/
	 /*Initialize the ssl library */
    SSL_library_init();
    /* Load all SSL algorithms */
    OpenSSL_add_all_algorithms();
    /* Load all SSL error message */
    SSL_load_error_strings();
   /* Generate an SSL_CTX in SSL V2 and V3 standards compatible ，It's SSL Content Text */
    ctx = SSL_CTX_new(SSLv23_server_method());
    /* 也可以用 SSLv2_server_method() 或 SSLv3_server_method() 单独表示 V2 或 V3标准 */
    if (ctx == NULL) {
        ERR_print_errors_fp(stdout);
        exit(1);
    }
    /* Load the user's digital certificate, which is used to send to the client. Certificate contains public key */
    if (SSL_CTX_use_certificate_file(ctx, argv[4], SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stdout);
        exit(1);
    }
    /* Load user private key */
    if (SSL_CTX_use_PrivateKey_file(ctx, argv[5], SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stdout);
        exit(1);
    }
    /* Check if the user's private key is correct */
    if (!SSL_CTX_check_private_key(ctx)) {
        ERR_print_errors_fp(stdout);
        exit(1);
    }

    bzero(&server, sizeof(server));
    server.sin_family = PF_INET;
    server.sin_port = htons(myport);
    if (argv[3])
        server.sin_addr.s_addr = inet_addr(argv[3]);
    else
        server.sin_addr.s_addr = INADDR_ANY;
		
    int ret = bind(sock,(const struct sockaddr*)&server,sizeof(server));
    if(ret < 0)
    {
        perror("bind");
        exit(2);
    }
    ret = listen(sock,5);
    if(ret < 0)
    {
        perror("listen");
        exit(3);
    }
    SSL *ssl;
    for(;;)
    {
        struct sockaddr_in client;
        socklen_t len;
		
	len = sizeof(struct sockaddr);		
        int client_sock = accept(sock,(struct sockaddr*)&client,&len);
		
		   
		
        if(client_sock < 0)
        {
            continue;
        }else {
			 
            printf("server: got connection from %s, port %d, socket %d\n",
                   inet_ntoa(client.sin_addr),
                   ntohs(client.sin_port), client_sock);
				 
		}
		
		
 	
        /* Generate a new SSL based on ctx */
        ssl = SSL_new(ctx);
        /* Add the socket of the connected user to SSL */
        SSL_set_fd(ssl, client_sock);
        /* Establish an SSL connection */
        if (SSL_accept(ssl) == -1) {
            perror("accept");
            close(client_sock);
            break;
        }
		
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
				finish:
			/* close SSL 连接 */
				SSL_shutdown(ssl);
			/* release SSL */
				SSL_free(ssl);
			/* close socket */
				close(client_sock);
				SSL_CTX_free(ctx);
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
            scanf("%d%d",&x,&y);
            point_server.row = x-1;
            point_server.col = y-1;
            ServerMove(board,&point_server);
            PrintBoard(board,ROWS,COLS);
            SSL_write(ssl,(void*)&point_server,sizeof(point_server));
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
 
			close(sock);
			
    return 0;
}
