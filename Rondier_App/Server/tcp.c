#include "tcp.h"
#include "utils.h"
#include <arpa/inet.h>
#include <stdio.h>

char in6_addr_buf[INET6_ADDRSTRLEN];

const char *str_of_in6_addr(const struct in6_addr *ip6) {
  const char *res = inet_ntop(AF_INET6, ip6, in6_addr_buf, INET6_ADDRSTRLEN);
  if (res == NULL) {
    log_error(-1, "inet_ntop(AF_INET6)");
  }
  return res;
}

char in_addr_buf[INET_ADDRSTRLEN];

const char *str_of_in_addr(const struct in_addr *ip) {
  const char *res = inet_ntop(AF_INET, ip, in_addr_buf, INET_ADDRSTRLEN);
  if (res == NULL) {
    log_error(-1, "inet_ntop(AF_INET)");
  }
  return res;
}

#define SOCKADDRSTRLEN 64
char sockaddr_buf[SOCKADDRSTRLEN];

const char *str_of_sockaddr(const struct sockaddr *addr) {
  const struct sockaddr_in *sin;
  const struct sockaddr_in6 *sin6;
  int count;
  switch (addr->sa_family) {
  case AF_INET:
    sin = (struct sockaddr_in *)addr;
    count = snprintf(sockaddr_buf, SOCKADDRSTRLEN, "%s:%d",
                     str_of_in_addr(&sin->sin_addr), ntohs(sin->sin_port));
    break;
  case AF_INET6:
    sin6 = (struct sockaddr_in6 *)addr;
    count = snprintf(sockaddr_buf, SOCKADDRSTRLEN, "[%s]:%d",
                     str_of_in6_addr(&sin6->sin6_addr), ntohs(sin6->sin6_port));
    break;
  default:
    log_error(0, "unknown sockaddr family");
    count = snprintf(sockaddr_buf, SOCKADDRSTRLEN,
                     "(unknown sockaddr family %d)", addr->sa_family);
    break;
  }
  if (count < 0) {
    log_error(0, "snprintf(): output error");
    return NULL;
  }
  if (count >= SOCKADDRSTRLEN) {
    log_error(0, "snprintf(): not enough space");
  }
  return sockaddr_buf;
}

int install_server(in_port_t port) {
  int rc;

  int sockfd = socket(AF_INET6, SOCK_STREAM, 0);
  if (sockfd < 0) {
    log_error(sockfd, "socket()");
    return -1;
  }

  int opt_reuseaddr = 1;
  rc = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt_reuseaddr,
                  sizeof(opt_reuseaddr));
  if (rc < 0) {
    log_error(rc, "setsockopt(SO_REUSEADDR)");
    return -1;
  }

  int opt_v6only = 0;
  rc = setsockopt(sockfd, IPPROTO_IPV6, IPV6_V6ONLY, &opt_v6only,
                  sizeof(opt_v6only));
  if (rc < 0) {
    log_error(rc, "setsockopt(IPV6_V6ONLY)");
    return -1;
  }

  struct sockaddr_in6 sin6 = {.sin6_family = AF_INET6,
                              .sin6_port = htons(port)};
  rc = bind(sockfd, (struct sockaddr *)&sin6, sizeof(sin6));
  if (rc < 0) {
    log_error(rc, "bind()");
    return -1;
  }

  rc = listen(sockfd, 1024);
  if (rc < 0) {
    log_error(rc, "listen()");
    return -1;
  }

  return sockfd;
}

int install_client(const char *ip6, in_port_t port) {
  int rc;

  int fd = socket(AF_INET6, SOCK_STREAM, 0);
  if (fd < 0) {
    log_error(fd, "socket()");
    return -1;
  }

  int opt_v6only = 0;
  rc = setsockopt(fd, IPPROTO_IPV6, IPV6_V6ONLY, &opt_v6only,
                  sizeof(opt_v6only));
  if (rc < 0) {
    log_error(rc, "setsockopt(IPV6_V6ONLY)");
    return -1;
  }

  struct sockaddr_in6 sin6 = {.sin6_family = AF_INET6,
                              .sin6_port = htons(port)};
  rc = inet_pton(AF_INET6, ip6, &sin6.sin6_addr);
  if (rc < 1) {
    log_error(rc, "Incorrect ipv6 address");
    return rc;
  }
  rc = connect(fd, (struct sockaddr *)&sin6, sizeof(sin6));
  if (rc < 0) {
    log_error(rc, "connect()");
    return -1;
  }

  return fd;
}
