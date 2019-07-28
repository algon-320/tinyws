SERVER_REQUIRE_OBJS = lib.o display.o draw.o query.o response.o tcp.o basic_structures.o window.o
CC = gcc
CFLAGS = -std=gnu99 -Wall -Wextra -g

all: server.out client_example.out
clean:
	rm *.out *.o

server.out: server.c $(SERVER_REQUIRE_OBJS)
	$(CC) server.c $(SERVER_REQUIRE_OBJS) -o server.out $(CFLAGS) -lpthread `sdl2-config --cflags --libs`

lib.o: lib_deque.o lib_emb_linked_list.o
	ld -r $^ -o $@
lib_deque.o: lib/deque.c
	$(CC) $(CFLAGS) -c $^ -o $@
lib_emb_linked_list.o: lib/embedded_linked_list.c
	$(CC) $(CFLAGS) -c $^ -o $@

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

CLIENT_REQUIRE_OBJS = query.o response.o tcp.o basic_structures.o
client_example.out: client_example.c $(CLIENT_REQUIRE_OBJS)
	$(CC) client_example.c $(CLIENT_REQUIRE_OBJS) -o client_example.out $(CFLAGS) `sdl2-config --cflags --libs`
