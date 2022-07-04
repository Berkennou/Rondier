#ifndef _UTILS_H
#define _UTILS_H

#include <sys/types.h>
#include <stddef.h>

#define __POS__ strpos(__FILE__, __func__, __LINE__)

#define log_msg(msg) log_msg_f(__POS__, msg)

#define log_error(rc, msg) log_error_f(rc, __POS__, msg)

#define exit_error(rc, msg) exit_error_f(rc, __POS__, msg)

#define handle_error(rc, msg) handle_error_f(rc, __POS__, msg)


const char *strpos(const char *file, const char *func, int line);

void log_msg_f(const char *pos, const char *msg);

void log_error_f(int rc, const char *pos, const char *msg);

void exit_error_f(int rc, const char *pos, const char *msg);

void handle_error_f(int rc, const char *pos, const char *msg);


ssize_t write_all(int fd, const void *buf, size_t count);

ssize_t read_all(int fd, void *buf, size_t count);

#endif /* "utils.h" included. */
