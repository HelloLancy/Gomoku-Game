.PHONY:all
all:server 
server: game.c server.c 
	gcc -o $@ -g $^

.PHONY:clean
clean:
	rm -f server 
