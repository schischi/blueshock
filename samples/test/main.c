#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ps3_controller.h>

int main(int argc, char *argv[])
{
    struct input_s b;

    ps3Controller_start();
    while(ps3Controller_count() == 0)
        ;
    fprintf(stderr, "Device found!\n");
    for(int i = 0; i < 1000000; ++i) {
        if(!ps3Controller_get(0, &b))
            fprintf(stderr, "Select is %d\n", b.digitalInput.select);
    }
    return 0;
}
