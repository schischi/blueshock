#include <stdio.h>
#include "log.h"

void dump(const char *tag, const unsigned char *buf, int len)
{
    if(!DEBUG)
        return;
    fprintf(stderr, "%s[%d]", tag, len);
    for (int i = 0; i < len; ++i)
        fprintf(stderr, " %02x", buf[i]);
    fprintf(stderr, "\n");
}

