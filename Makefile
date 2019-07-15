SERVER_REQUIRE_OBJS = container_lib.o display.o draw.o query.o tcp.o
CC = gcc
CFLAGS = -Wall -Wextra -g

all: server.out client_example.out
clean:
	rm *.out *.o

server.out: server.c $(SERVER_REQUIRE_OBJS)
	$(CC) server.c $(SERVER_REQUIRE_OBJS) -o server.out $(CFLAGS) -lpthread `sdl2-config --cflags --libs`

container_lib.o: lib/deque.c
	$(CC) -c lib/deque.c -o container_lib.o $(CFLAGS) 

display.o: display.c
	$(CC) -c $^ -o $@ $(CFLAGS)
draw.o: draw.c
	$(CC) -c $^ -o $@ $(CFLAGS)
query.o: query.c
	$(CC) -c $^ -o $@ $(CFLAGS)
tcp.o: tcp.c
	$(CC) -c $^ -o $@ $(CFLAGS)

CLIENT_REQUIRE_OBJS = query.o tcp.o
client_example.out: client_example.c $(CLIENT_REQUIRE_OBJS)
	$(CC) client_example.c $(CLIENT_REQUIRE_OBJS) -o client_example.out $(CFLAGS) `sdl2-config --cflags --libs`
