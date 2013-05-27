#ifndef DUALSHOCK3_H
# define DUALSHOCK3_H

#include <stdint.h>
#include <pthread.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/l2cap.h>
#include <errno.h>

struct deviceConnection_s {
    unsigned int index;
    bdaddr_t addr;
    int csk;
    int isk;
    struct deviceConnection_s *next;
};

typedef struct input_s *input_t;
struct input_s {
    struct {
        uint8_t 
            leftStick_x,
            leftStick_y,
            rightStick_x,
            rightStick_y;
    } stick;

    struct {
        uint8_t 
            select:1,
            l3:1,
            r3:1,
            start:1,
            up:1,
            right:1,
            down:1,
            left:1,
            l2:1,
            r2:1,
            l1:1,
            r1:1,
            triangle:1,
            circle:1,
            cross:1,
            square:1;
    } digitalInput;

    struct {
        uint8_t l1;
        uint8_t l2;
        uint8_t r1;
        uint8_t r2;
        uint8_t triangle;
        uint8_t circle;
        uint8_t cross;
        uint8_t square;
        uint8_t up;
        uint8_t right;
        uint8_t down;
        uint8_t left;
    } analogInput;

    struct {
        int8_t x;
        int8_t y;
        int8_t z;
        int8_t gZ;
    }axis;
};

typedef struct controller_s *controller_t;
struct controller_s {
    int                         index;
    struct deviceConnection_s   connection;
    struct input_s              buttons;
    pthread_mutex_t             mutex;
    controller_t                next;
};

void ps3Controller_start();
int ps3Controller_get(int index, input_t buttons);
int ps3Controller_count();

//TODO: gestion index si deconnexion

#endif