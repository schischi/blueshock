#include "dualshock3.h"
#include "log.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/socket.h>

/* Global variables */
controller_t controllerList_g = NULL;
static int csk = 0;
static int isk = 0;

static int l2cap_listen(const bdaddr_t *bdaddr, unsigned short psm);
static void ps3Controller_init();
static void ps3Controller_setupDevice(int sk);
static void ps3Controller_handleConnection();
static void ps3Controller_handleDisconnection();
static void ps3Controller_handleReport(unsigned char buf[static 49], int len,
        controller_t c);
static void* ps3Controller_mainLoop();

void ps3Controller_start()
{
    pthread_t t;

    if(csk == 0 && isk == 0) {
        ps3Controller_init(&csk, &isk);
        pthread_create(&t, NULL, &ps3Controller_mainLoop, NULL);
        //pthread_join(t, NULL); 
        //ps3Controller_mainLoop();
    }
}

int ps3Controller_count()
{
    controller_t c;
    int i = 0;
    for (c = controllerList_g; c; c = c->next)
        ++i;
    return i;
}

int ps3Controller_get(int index, input_t buttons)
{
    controller_t c;
    //LOG("Get index %d\n", index);
    for (c = controllerList_g; c && c->index != index; c = c->next)
        LOG("index: %d\n", c->index);
    if(c == NULL) {
        //LOG("Index %d not found\n", index);
        return -1;
    }
    pthread_mutex_lock(&c->mutex);
    *buttons = c->buttons;
    pthread_mutex_unlock(&c->mutex);

    return 0;
}

static int l2cap_listen(const bdaddr_t *bdaddr, unsigned short psm)
{
    int sk = socket(PF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP);
    if (sk < 0)
        fatal("socket");

    struct sockaddr_l2 addr;
    memset(&addr, 0, sizeof(addr));
    addr.l2_family = AF_BLUETOOTH;
    addr.l2_bdaddr = *BDADDR_ANY;
    addr.l2_psm = htobs(psm);
    if (bind(sk, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        fatal("bind");
        close(sk);
        return -1;
    }
    if (listen(sk, 5) < 0)
        fatal("listen");
    return sk;
}

static void ps3Controller_init()
{
    uid_t uid = getuid();
    if(uid) {
        fprintf(stderr, "Run as root, and you will be happy :)\n");
        exit(EXIT_FAILURE);
    }
    csk = l2cap_listen(BDADDR_ANY, L2CAP_PSM_HIDP_CTRL);
    isk = l2cap_listen(BDADDR_ANY, L2CAP_PSM_HIDP_INTR);

    if (csk < 0 || isk < 0) {
        fprintf(stderr, "Unable to listen on HID PSMs.\n");
        exit(EXIT_FAILURE);
    }
    LOG("%s", "Waiting for Bluetooth connections.\n");
}

static void ps3Controller_handleConnection()
{
    LOG("%s", "New Device\n");
    controller_t c = malloc(sizeof(struct controller_s));
    c->index = 0;
    if((c->connection.csk = accept(csk, NULL, NULL)) < 0)
        fatal("accept(CTRL)");
    if((c->connection.isk = accept(isk, NULL, NULL)) < 0)
        fatal("accept(INTR)");

    struct sockaddr_l2 addr;
    socklen_t addrlen = sizeof(addr);
    if (getpeername(c->connection.isk, (struct sockaddr *)&addr, &addrlen) < 0)
        fatal("getpeername");
    c->connection.addr = addr.l2_bdaddr;

    unsigned char resp[64];
    char get03f2[] = { HIDP_TRANS_GET_REPORT | HIDP_DATA_RTYPE_FEATURE | 8,
        0xf2, sizeof(resp), sizeof(resp)>>8 };
    (void)send(c->connection.csk, get03f2, sizeof(get03f2), 0);
    (void)recv(c->connection.csk, resp, sizeof(resp), 0);

    pthread_mutex_init(&c->mutex, NULL);
    c->next = controllerList_g;
    controllerList_g = c;
    ps3Controller_setupDevice(c->connection.csk);
}

static void ps3Controller_handleDisconnection()
{

}

static void ps3Controller_handleReport(unsigned char buf[static 49], int len,
        controller_t c)
{
    if (buf[0] != 0xa1)
        return;

    pthread_mutex_lock(&c->mutex);
    /* Digital */
    c->buttons.digitalInput.select   = (buf[3] & 0x01) >> 0;
    c->buttons.digitalInput.l3       = (buf[3] & 0x02) >> 1;
    c->buttons.digitalInput.r3       = (buf[3] & 0x04) >> 2;
    c->buttons.digitalInput.start    = (buf[3] & 0x08) >> 3;
    c->buttons.digitalInput.up       = (buf[3] & 0x10) >> 4;
    c->buttons.digitalInput.right    = (buf[3] & 0x20) >> 5;
    c->buttons.digitalInput.down     = (buf[3] & 0x40) >> 6;
    c->buttons.digitalInput.left     = (buf[3] & 0x80) >> 7;
    c->buttons.digitalInput.l2       = (buf[4] & 0x01) >> 1;
    c->buttons.digitalInput.r2       = (buf[4] & 0x02) >> 2;
    c->buttons.digitalInput.l1       = (buf[4] & 0x04) >> 3;
    c->buttons.digitalInput.r1       = (buf[4] & 0x08) >> 4;
    c->buttons.digitalInput.triangle = (buf[4] & 0x10) >> 5;
    c->buttons.digitalInput.circle   = (buf[4] & 0x20) >> 6;
    c->buttons.digitalInput.cross    = (buf[4] & 0x40) >> 7;
    c->buttons.digitalInput.square   = (buf[4] & 0x80) >> 8;

    /* Sticks */
    c->buttons.stick.leftStick_x     = buf[7];
    c->buttons.stick.leftStick_y     = buf[8];
    c->buttons.stick.rightStick_x    = buf[9];
    c->buttons.stick.rightStick_y    = buf[10];

    /* Analog */
    c->buttons.analogInput.l2        = buf[19];
    c->buttons.analogInput.r2        = buf[20];
    c->buttons.analogInput.l1        = buf[21];
    c->buttons.analogInput.r1        = buf[22];
    c->buttons.analogInput.triangle  = buf[23];
    c->buttons.analogInput.circle    = buf[24];
    c->buttons.analogInput.cross     = buf[25];
    c->buttons.analogInput.square    = buf[26];

    /* Axis */
    c->buttons.axis.x                = buf[43];
    c->buttons.axis.y                = buf[45];
    c->buttons.axis.z                = buf[47];
    c->buttons.axis.gZ               = buf[47];

    pthread_mutex_unlock(&c->mutex);
}

static void* ps3Controller_mainLoop()
{
    LOG("%s", "MainLoop\n");
    while (1) {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(STDIN_FILENO, &fds);
        int fdmax = 0;
        FD_SET(csk, &fds);
        FD_SET(isk, &fds);
        fdmax = csk > isk ? csk : isk;

        for (controller_t c = controllerList_g; c; c = c->next) {
            FD_SET(c->connection.csk, &fds);
            if (c->connection.csk > fdmax)
                fdmax = c->connection.csk;
            FD_SET(c->connection.isk, &fds);
            if (c->connection.isk > fdmax)
                fdmax = c->connection.isk;
        }
        if (select(fdmax + 1, &fds, NULL, NULL, NULL) < 0)
            fatal("select");

        // new connection (new device)
        if (FD_ISSET(csk, &fds))
            ps3Controller_handleConnection();

        // data from a paired device
        for (controller_t c = controllerList_g; c; c = c->next) {
            if (FD_ISSET(c->connection.isk, &fds)) {
                unsigned char report[256];
                int nr = recv(c->connection.isk, report, sizeof(report), 0);
                if (nr <= 0) {
                    //TODO: handle disconnection
                }
                else
                    ps3Controller_handleReport(report, nr, c);
            }
        }

    }
}

static void ps3Controller_setupDevice(int sk)
{
    int index = 1; //FIXME
    char set03f4[] = { HIDP_TRANS_SET_REPORT | HIDP_DATA_RTYPE_FEATURE,
        0xf4, 0x42, 0x03, 0x00, 0x00 };
    // Enable reporting
    send(sk, set03f4, sizeof(set03f4), 0);
    unsigned char ack;
    int nr = recv(sk, &ack, sizeof(ack), 0);
    if (nr != 1 || ack != 0)
        fatal("ack");
    // Leds: Display 1+index in additive format.
    static const char ledmask[10] = { 1, 2, 4, 8, 6, 7, 11, 13, 14, 15 };
#define LED_PERMANENT 0xff, 0x27, 0x00, 0x00, 0x32
    char set0201[] = {
        HIDP_TRANS_SET_REPORT | HIDP_DATA_RTYPE_OUTPUT, 0x01,
        0x00, 0x00, 0x00, 0,0, 0x00, 0x00, 0x00,
        0x00, ledmask[index%10]<<1,
        LED_PERMANENT,
        LED_PERMANENT,
        LED_PERMANENT,
        LED_PERMANENT,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    set0201[3] = set0201[5] = 4;
    //SIXAXIS
    //set0201[5] = 0xff;  // Enable gyro
    //set0201[6] = 0x78;  // Constant bias (should adjust periodically ?)
    //DS3
    set0201[5] = 0;
    set0201[6] = 0x70;
    send(sk, set0201, sizeof(set0201), 0);
    nr = recv(sk, &ack, sizeof(ack), 0);
    if (nr != 1 || ack != 0)
        fatal("ack");
}

/*
   void dualshock_rumble(int csk, uint8_t left, uint8_t right, uint8_t t)
   {
   unsigned char buf[] = {
   0x52,
   0x01,
   0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x1E,
   0xff, 0x27, 0x10, 0x00, 0x32,
   0xff, 0x27, 0x10, 0x00, 0x32,
   0xff, 0x27, 0x10, 0x00, 0x32,
   0xff, 0x27, 0x10, 0x00, 0x32,
   0x00, 0x00, 0x00, 0x00, 0x00
   };
   unsigned char buf2[128];

   if(t < 10)
   t = 10;
   buf[3] = buf[5] = t;
   buf[4] = left;
   buf[6] = right;
   (void)send(csk, buf, sizeof(buf), 0);
   (void)recv(csk, buf2, sizeof(buf2), 0);
   }
   */
