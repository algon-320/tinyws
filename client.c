#include <assert.h>
#include "client.h"
#include "lib/stack.h"

const client_id_t CLIENT_ID_INVALID = -1;

Deque clients_ptr;       // <struct Client *>
pthread_mutex_t clients_mutex;
client_id_t next_client_id;

void client_subsystem_init() {
    next_client_id = 0;
    clients_ptr = deque_new(0, sizeof(struct Client *));
    
    pthread_mutexattr_t mutex_attr;
    pthread_mutexattr_init(&mutex_attr);
    pthread_mutexattr_settype(&mutex_attr, PTHREAD_MUTEX_RECURSIVE_NP);
    pthread_mutex_init(&clients_mutex, &mutex_attr);
}

client_id_t client_new() {
    struct Client *client = calloc(1, sizeof(struct Client));
    client->openning_windows = deque_new(0, sizeof(window_id_t));
    client->events = deque_new(0, sizeof(struct Event));
    client->is_alive = true;
    pthread_cond_init(&client->events_non_empty, NULL);

    lock_mutex(&clients_mutex);
    {
        client->id = next_client_id++;
        deque_push_back(&clients_ptr, &client);
    }
    unlock_mutex(&clients_mutex);

    return client->id;
}

void client_close(client_id_t client_id) {
    struct Window *to_lock = window_get_own(0);
    lock_mutex(&clients_mutex);
    {
        struct Client *client = client_ref(client_id);
        
        // close all windows that opened by the client
        while (deque_size(&client->openning_windows)) {
            window_id_t win_id = DEQUE_TAKE(deque_back(&client->openning_windows), window_id_t);
            deque_pop_back(&client->openning_windows);

            struct Window *win = window_ref(win_id);
            window_close_ptr(win);
        }

        deque_free(&client->openning_windows);
        deque_free(&client->events);
        client->is_alive = false;

        pthread_cond_destroy(&client->events_non_empty);

        DEQUE_TAKE(deque_at(&clients_ptr, client->id), struct Client *) = NULL;
        free(client);
    }
    unlock_mutex(&clients_mutex);
    window_return_own(to_lock);
}


bool client_is_valid(client_id_t client_id) {
    lock_mutex(&clients_mutex);
    bool ret = true;
    if (!(0 <= client_id && (size_t)client_id < deque_size(&clients_ptr))) {
        debugprint("client_id out of range\n");
        ret = false;
    } else {
        struct Client *tmp = DEQUE_TAKE(deque_at(&clients_ptr, client_id), struct Client *);
        if (!tmp) {
            debugprint("client: %d is NULL\n", client_id);
            ret = false;
        }
    }
    unlock_mutex(&clients_mutex);
    return ret;
}

// get client without ownnership
struct Client *client_ref(client_id_t client_id) {
    if (!(0 <= client_id && (size_t)client_id < deque_size(&clients_ptr))) {
        return NULL;
    }
    return DEQUE_TAKE(deque_at(&clients_ptr, client_id), struct Client *);
}

// get client with ownnership
struct Client *client_get_own(client_id_t client_id) {
    lock_mutex(&clients_mutex);
    struct Client *client = client_ref(client_id);
    if (!client) {
        unlock_mutex(&clients_mutex);
        return NULL;
    }
    return client;
}

// return ownnership
void client_return_own(struct Client *client) {
    if (!client) return;
    unlock_mutex(&clients_mutex);
}

void client_event_push(client_id_t client_id, struct Event *event) {
    struct Client *client = client_get_own(client_id);
    if (!client) return;
    queue_push(&client->events, event);
    client_return_own(client);
    pthread_cond_signal(&client->events_non_empty);
}

bool client_event_pop(client_id_t client_id, struct Event *event) {
    bool ret = false;
    struct Client *client = client_get_own(client_id);
    if (!client) return false;
    if (queue_empty(&client->events)) {
        pthread_cond_wait(&client->events_non_empty, &clients_mutex);
    }
    *event = DEQUE_TAKE(queue_front(&client->events), struct Event);
    queue_pop(&client->events);
    ret = true;
    client_return_own(client);
    return ret;
}

void client_openning_window_push(client_id_t client_id, window_id_t win) {
    struct Client *client = client_get_own(client_id);
    if (!client) return;
    deque_push_back(&client->openning_windows, &win);
    client_return_own(client);
}