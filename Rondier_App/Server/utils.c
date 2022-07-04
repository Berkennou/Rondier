#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "utils.h"

#define POS_BUF_SIZE 256

char pos_buf[POS_BUF_SIZE];

const char *
strpos(const char *file, const char *func, int line)
{
    snprintf(pos_buf, POS_BUF_SIZE,
             "pid %d, file %s, func %s, line %d",
             getpid(), file, func, line);
    return pos_buf;
}

void
log_msg_f(const char *pos, const char *msg)
{
    fprintf(stderr, "%s: %s\n", pos, msg);
}

void
log_error_f(int rc, const char *pos, const char *msg)
{
    if (rc < 0) {
        fprintf(stderr, "error: %s: %s: %s\n",
                pos, msg, strerror(errno));
    } else {
        fprintf(stderr, "error: %s: %s\n", pos, msg);
    }
}

void
exit_error_f(int rc, const char *pos, const char *msg)
{
    log_error_f(rc, pos, msg);
    exit(EXIT_FAILURE);
}

void
handle_error_f(int rc, const char *pos, const char *msg)
{
    if (rc < 0) {
        exit_error_f(rc, pos, msg);
    }
}


ssize_t
write_all(int fd, const void *buf, size_t count)
{
    const char *charbuf = buf;
    size_t buf_div = 0;
    while (buf_div < count) {
        int rc = write(fd, charbuf + buf_div, count - buf_div);
        if (rc < 0) {
            log_error(rc, "write()");
            return -1;
        }
        buf_div += rc;
    }
    return buf_div;
}

ssize_t
read_all(int fd, void *buf, size_t count)
{
    char *charbuf = buf;
    size_t buf_div = 0;
    while (buf_div < count) {
        int rc = read(fd, charbuf + buf_div, count - buf_div);
        if (rc < 0) {
            log_error(rc, "read()");
            return -1;
        }
        if (rc == 0) {
            break;
        }
        buf_div += rc;
    }
    return buf_div;
}
