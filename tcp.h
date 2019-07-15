#ifndef INCLUDE_GUARD_TCP_H
#define INCLUDE_GUARD_TCP_H

#include <stdio.h>

int tcp_acc_port(int portno, int ip_version);
int tcp_connect(char *server, int portno);

int fdopen_sock(int sock, FILE **inp, FILE **outp);

#endif