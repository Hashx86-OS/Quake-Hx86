#ifndef _UNISTD_H_
#define _UNISTD_H_

#include <stddef.h>

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

int open(const char *pathname, int flags, ...);
int close(int fd);
int read(int fd, void *buf, int count);
int write(int fd, const void *buf, int count);
int lseek(int fd, int offset, int whence);
int unlink(const char *pathname);

#endif
