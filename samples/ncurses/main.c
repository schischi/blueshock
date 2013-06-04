#define _BSD_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <curses.h>
#include <unistd.h>
#include <limits.h>
#include <blueshock.h>

#define REFRESH_RATE 60

char array[18][59] = {
    { "          L2: XXX                         R2: XXX         " },
    { "          L1: XXX                         R1: XXX         " },
    { "          _______                         _______         " },
    { "       __|       |__xxxo_________________|       |__      " },
    { "      /                                             \\     " },
    { "     /      X                                XXX     \\    " },
    { "    /                Select     Start                 \\   " },
    { "   |    X   +   X      X           X     XXX  +  XXX   |  " },
    { "   \\                                                   /  " },
    { "    \\       X               PS               XXX      /   " },
    { "     \\                                               /    " },
    { "     /         _   X: XXX   ___   X: XXX   _         \\    " },
    { "    /         / \\  Y: XXX  /   \\  Y: XXX  / \\         \\   " },
    { "   /         /   \\________/     \\________/   \\         \\  " },
    { "  /         /                                 \\         \\ " },
    { " /   XXX   /       L3: X          R3: X        \\   XXX   \\" },
    { " \\        /                                     \\        /" },
    { "  \\______/                                       \\______/ " }
};
char loadingCursor[] = "|/-\\|/-\\";
WINDOW *mainwin, *cwin, *gwin, *lwin;
struct dualshock3_s b;

void init_ui()
{
    if ((mainwin = initscr()) == NULL) {
        fprintf(stderr, "Error initialising ncurses.\n");
        exit(EXIT_FAILURE);
    }
    if(LINES < 30) {
        fprintf(stderr, "Term too small :(\n");
        exit(EXIT_FAILURE);
    }
    noecho();
    curs_set(0);
    keypad(mainwin, TRUE);
    mvwaddstr(mainwin, 0, (COLS - 9) / 2, "Dualshock");

    lwin = subwin(mainwin, 4, 36, LINES / 2, (COLS - 36) / 2);
    box(lwin, 0, 0);

    refresh();
}

void init_mainUI()
{
    cwin = subwin(mainwin, 20, COLS, 1, 0);
    box(cwin, 0, 0);
    mvwaddstr(cwin, 0, 4, " Dualshock sixaxis ");

    gwin = subwin(mainwin, LINES - 21, COLS, 21, 0);
    box(gwin, 0, 0);
    mvwaddstr(gwin, 0, 4, " Axis ");

    delwin(lwin);

    refresh();
}

void refresh_ui()
{
    int i;

    /* Controller */
    int offset = (COLS - 59) / 2;
    for(i = 1; i < 19; ++i)
        mvwprintw(cwin, i, offset, "%s", array[i-1]);
    mvwprintw(cwin,  1, 14 + offset, "%.3d", b.analogInput.l2);
    mvwprintw(cwin,  1, 46 + offset, "%.3d", b.analogInput.r2);
    mvwprintw(cwin,  2, 14 + offset, "%.3d", b.analogInput.l1);
    mvwprintw(cwin,  2, 46 + offset, "%.3d", b.analogInput.r1);
    mvwprintw(cwin,  6, 12 + offset, "%.1d", b.digitalInput.up);
    mvwprintw(cwin,  8,  8 + offset, "%.1d", b.digitalInput.left);
    mvwprintw(cwin,  8, 16 + offset, "%.1d", b.digitalInput.right);
    mvwprintw(cwin,  8, 23 + offset, "%.1d", b.digitalInput.select);
    mvwprintw(cwin,  8, 35 + offset, "%.1d", b.digitalInput.start);
    mvwprintw(cwin,  6, 45 + offset, "%.3d", b.analogInput.triangle);
    mvwprintw(cwin,  8, 41 + offset, "%.3d", b.analogInput.square);
    mvwprintw(cwin,  8, 49 + offset, "%.3d", b.analogInput.circle);
    mvwprintw(cwin, 10, 12 + offset, "%.1d", b.digitalInput.down);
    mvwprintw(cwin, 10, 45 + offset, "%.3d", b.analogInput.cross);
    mvwprintw(cwin, 12, 22 + offset, "%.3d", b.stick.leftStick_x);
    mvwprintw(cwin, 12, 37 + offset, "%.3d", b.stick.rightStick_x);
    mvwprintw(cwin, 13, 22 + offset, "%.3d", b.stick.leftStick_y);
    mvwprintw(cwin, 13, 37 + offset, "%.3d", b.stick.rightStick_y);
    mvwprintw(cwin, 16, 23 + offset, "%.1d", b.digitalInput.l3);
    mvwprintw(cwin, 16, 38 + offset, "%.1d", b.digitalInput.r3);
    wrefresh(cwin);

    /* Axis */
    int mx = COLS / 2;
    int my = (LINES - 21) / 2;
    char c;
    mvwprintw(gwin, 1, 1, "%s", "            ");
    mvwprintw(gwin, 2, 1, "%s", "            ");
    mvwprintw(gwin, 2, 1, "aY: %d", b.axis.y);
    mvwprintw(gwin, 1, 1, "aX: %d", b.axis.x);
    mvwprintw(gwin, 2, 1, "aY: %d", b.axis.y);
    mvwprintw(gwin, my, mx, "%c", '+');
    /* X axis */
    int units = mx - 2;
    int n = SCHAR_MAX / units;
    int absX = abs(b.axis.x);
    offset = (b.axis.x > 0 ? mx * 2 - 1 : 1);
    for(i = 1; i <= units; ++i) {
        if(absX > ((units - i) * n))
            c = '-';
        else
            c = ' ';
        mvwprintw(gwin, my, offset + (i * (b.axis.x < 0 ? 1 : -1)), "%c", c);
    }
    /* Y axis */
    int absY = abs(b.axis.y);
    units = my - 2;
    n = SCHAR_MAX / units;
    offset = (b.axis.y < 0 ? my * 2 - 2 : 2);
    for(i = 0; i < units; ++i) {
        if(absY > ((units - i) * n))
            c = '|';
        else
            c = ' ';
        mvwprintw(gwin, offset + (i * (b.axis.y < 0 ? -1 : 1)), mx, "%c", c);
    }
    wrefresh(gwin);
}

void loadingScreen()
{
    static int i = 0;
    mvwprintw(lwin, 1, 2, "%s", " Waiting for a Bluetooth Device ");
    mvwprintw(lwin, 2, 18, "%c", loadingCursor[i++]);
    if(i >= sizeof(loadingCursor))
        i = 0;
    wrefresh(lwin);
}

int main()
{

    if(blueshock_start() == -1) {
        exit(EXIT_FAILURE);
    }
    init_ui();
    while(blueshock_get(0, &b) == -1) {
        loadingScreen();
        usleep(1000 * 250);
    }

    init_mainUI();
    while(1) {
        if(!blueshock_get(0, &b)) {
            refresh_ui();
        }
        usleep(1000 * 1000 / REFRESH_RATE);
    }
    delwin(cwin);
    delwin(gwin);
    delwin(mainwin);
    endwin();
    refresh();
    return EXIT_SUCCESS;
}
