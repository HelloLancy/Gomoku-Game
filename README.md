# Gomoku-Game
This is successful running between server and client
C-based client and server applications responsible for running a multiplayer online word guessing game. Implemented using sockets.


for server Makefile
.PHONY:all
all:server 
server: game.c server.c 
	gcc -o $@ -g $^
.PHONY:clean
clean:
	rm -f server 

For client

.PHONY:all
all:client
client: game.c client.c
	gcc -o $@ -g $^
.PHONY:clean
clean:
	rm -f client
