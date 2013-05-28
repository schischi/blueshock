#ifndef LOG_H
# define LOG_H

# include <errno.h>

# define DEBUG 1

# define LOG(fmt, ...)                         \
    do {                                       \
        if (DEBUG)                             \
            fprintf(stderr, fmt, __VA_ARGS__); \
    } while (0)

# define fatal(x)                           \
    do {                                    \
        if (errno)                          \
            perror(x);                      \
        else                                \
            fprintf(stderr, "%s \n", x);    \
        exit(1);                            \
    } while (0)

void dump(const char *tag, const unsigned char *buf, int len);

#endif
