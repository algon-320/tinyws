#include "client.h"
#include "lib/stack.h"

Deque clients;       // <struct Client>
Stack free_clients;  // <struct Client *>

void expand_clients(size_t num) {
    int cur_len = deque_size(&clients);
    for (int idx = cur_len + num - 1; idx >= cur_len; idx--) {
        deque_push_back(&clients, NULL);
    }
    for (int idx = cur_len + num - 1; idx >= cur_len; idx--) {
        struct Client *client = deque_at(&clients, idx);
        client->id = idx;
        stack_push(&free_clients, &client);
    }
}

void client_subsystem_init() {
    static const int INITIAL_CLIENT_ALLOC_NUM = 16;
    clients = deque_new_with_capacity(0, sizeof(struct Client), INITIAL_CLIENT_ALLOC_NUM);
    free_clients = stack_new(sizeof(struct Client *));

    expand_clients(INITIAL_CLIENT_ALLOC_NUM);
}

struct Client *client_new() {
    if (stack_empty(&free_clients)) {
        size_t expand_len = deque_size(&clients);
        expand_clients(expand_len);
    }

    struct Client *client = DEQUE_TAKE(stack_top(&free_clients), struct Client *);
    stack_pop(&free_clients);

    client->openning_windows = deque_new(0, sizeof(struct Window *));
    client->events = deque_new(0, sizeof(struct Event));
    client->is_alive = true;

    return client;
}

void client_close(struct Client *client) {
    // close all windows that opened by the client
    while (deque_size(&client->openning_windows)) {
        struct Window *win = DEQUE_TAKE(deque_back(&client->openning_windows), struct Window *);
        deque_pop_back(&client->openning_windows);
        window_close(win);
    }

    deque_free(&client->openning_windows);
    deque_free(&client->events);
    client->is_alive = false;

    stack_push(&free_clients, &client);
}

struct Client *client_get_by_id(client_id_t client_id) {
    if (0 <= client_id && client_id < (client_id_t)deque_size(&clients)) {
        struct Client *client = deque_at(&clients, client_id);
        if (client->is_alive) {
            return client;
        }
    }
    return NULL;
}

void client_send_event(struct Client *client, struct Event *event) {
    queue_push(&client->events, event);
}