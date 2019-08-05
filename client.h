#ifndef INCLUDE_GUARD_CLIENT_H
#define INCLUDE_GUARD_CLIENT_H

#include <stdbool.h>

#include "common.h"
#include "lib/deque.h"
#include "lib/queue.h"
#include "window.h"
#include "event.h"

struct Client {
    client_id_t id;
    bool is_alive;
    Deque openning_windows;  // <window_id_t>
    Queue events; // <struct Event>

    pthread_mutex_t mutex;
};

void client_subsystem_init();
client_id_t client_new();
void client_close(client_id_t client_id);

// check is the client_id valid
bool client_is_valid(client_id_t client_id);
// get client without ownnership
struct Client *client_ref(client_id_t client_id);
// get client with ownnership
struct Client *client_get_own(client_id_t client_id);
// return ownnership
void client_return_own(struct Client *client);

void client_event_push(client_id_t client_id, struct Event *event);
bool client_event_pop(client_id_t client_id, struct Event *event);

void client_openning_window_push(client_id_t client_id, window_id_t win_id);

#endif