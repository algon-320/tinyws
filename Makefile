all: server client_example
server: server.c container_lib.o
	gcc server.c container_lib.o -o server `sdl2-config --cflags --libs` -lpthread
client_example: client_example.c
	gcc client_example.c -o client_example `sdl2-config --cflags --libs`
container_lib.o: lib/deque.c
	gcc -c lib/deque.c -o container_lib.o
