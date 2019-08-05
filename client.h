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
};

void client_subsystem_init();
struct Client *client_new();
void client_close(struct Client *client);
struct Client *client_get_by_id(client_id_t client_id);
void client_event_push(struct Client *client, struct Event *event);
bool client_event_pop(struct Client *client, struct Event *event);

void client_openning_window_push(struct Client *client, window_id_t win_id);

#endif