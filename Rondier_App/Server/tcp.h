#ifndef _TCP_H
#define _TCP_H

#include <netinet/in.h>

const char *str_of_sockaddr(const struct sockaddr *addr);

int install_server(in_port_t port);

int install_client(const char *ip6, in_port_t port);

#endif /* "tcp.h" included. */
