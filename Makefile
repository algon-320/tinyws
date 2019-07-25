SERVER_REQUIRE_OBJS = container_lib.o display.o draw.o query.o response.o tcp.o basic_structures.o window.o
CC = gcc
CFLAGS = -std=gnu99 -Wall -Wextra -g

all: server.out client_example.out
clean:
	rm *.out *.o

server.out: server.c $(SERVER_REQUIRE_OBJS)
	$(CC) server.c $(SERVER_REQUIRE_OBJS) -o server.out $(CFLAGS) -lpthread `sdl2-config --cflags --libs`

container_lib.o: lib/deque.c
	$(CC) -c lib/deque.c -o container_lib.o $(CFLAGS) 

basic_structures.o: basic_structures.c
	$(CC) $(CFLAGS) -c $^ -o $@ 
display.o: display.c
	$(CC) $(CFLAGS) -c $^ -o $@ 
draw.o: draw.c
	$(CC) $(CFLAGS) -c $^ -o $@ 
query.o: query.c
	$(CC) $(CFLAGS) -c $^ -o $@ 
response.o: response.c
	$(CC) $(CFLAGS) -c $^ -o $@ 
tcp.o: tcp.c
	$(CC) $(CFLAGS) -c $^ -o $@ 
window.o: window.c
	$(CC) $(CFLAGS) -c $^ -o $@ 

CLIENT_REQUIRE_OBJS = query.o response.o tcp.o
client_example.out: client_example.c $(CLIENT_REQUIRE_OBJS)
	$(CC) client_example.c $(CLIENT_REQUIRE_OBJS) -o client_example.out $(CFLAGS) `sdl2-config --cflags --libs`
