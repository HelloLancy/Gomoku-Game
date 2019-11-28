# Gomoku-Game
This is successful running between server and client
C-based client and server applications responsible for running a multiplayer online Gomoku game. Implemented using sockets.


for server Makefile
```.PHONY:all
all:server 
server: game.c server.c 
	gcc -o $@ -g $^
.PHONY:clean
clean:
	rm -f server
```

For client

```.PHONY:all
all:client
client: game.c client.c
	gcc -o $@ -g $^
.PHONY:clean
clean:
	rm -f client
```


For gcc the ss_server.c and ss_client.c we should use command on below 

```gcc -Wall ss_client.c game.c -o client -L/usr/lib -lssl -lcrypto```

```gcc -Wall ss_server.c game.c -o server -L/usr/lib -lssl -lcrypto```
