#include <stdlib.h>
#include <stdio.h>
#include <curses.h>
#include <unistd.h>
#include <ps3_controller.h>

int main(void)
{
    WINDOW *mainwin, *dwin, *awin, *pwin;
    int      ch;

    if ((mainwin = initscr()) == NULL) {
        fprintf(stderr, "Error initialising ncurses.\n");
        exit(EXIT_FAILURE);
    }
    noecho();
    keypad(mainwin, TRUE);
    mvwaddstr(mainwin, 0, (COLS - 9) / 2, "Dualshock");

    struct input_s b;

    ps3Controller_start();
    while(ps3Controller_count() == 0)
        ;

    dwin = subwin(mainwin, 6, COLS, 1, 0);
    box(dwin, 0, 0);
    mvwaddstr(dwin, 0, 4, " Digital Input ");

    awin = subwin(mainwin, 6, COLS / 2, 7, 0);
    box(awin, 0, 0);
    mvwaddstr(awin, 0, 4, " Analog Input ");

    pwin = subwin(mainwin, 6, COLS / 2, 7, COLS / 2);
    box(pwin, 0, 0);
    mvwaddstr(pwin, 0, 4, " Axis ");

    refresh();


    while(1) {
        if(!ps3Controller_get(0, &b)) {
            mvwprintw(dwin, 1, 2, "%-10s%d", "up:", b.digitalInput.up);
            mvwprintw(dwin, 2, 2, "%-10s%d", "right:", b.digitalInput.right);
            mvwprintw(dwin, 3, 2, "%-10s%d", "down:", b.digitalInput.down);
            mvwprintw(dwin, 4, 2, "%-10s%d", "left:", b.digitalInput.left);
            mvwprintw(dwin, 1, 20, "%-10s%d", "select:", b.digitalInput.select);
            mvwprintw(dwin, 2, 20, "%-10s%d", "start:", b.digitalInput.start);
            mvwprintw(dwin, 1, 38, "%-10s%d", "triangle:", b.digitalInput.triangle);
            mvwprintw(dwin, 2, 38, "%-10s%d", "circle:", b.digitalInput.circle);
            mvwprintw(dwin, 3, 38, "%-10s%d", "cross:", b.digitalInput.cross);
            mvwprintw(dwin, 4, 38, "%-10s%d", "square:", b.digitalInput.square);
            mvwprintw(dwin, 1, 56, "%-10s%d", "L1:", b.digitalInput.l1);
            mvwprintw(dwin, 2, 56, "%-10s%d", "L2:", b.digitalInput.l2);
            mvwprintw(dwin, 3, 56, "%-10s%d", "L3:", b.digitalInput.l3);
            mvwprintw(dwin, 1, 74, "%-10s%d", "R1:", b.digitalInput.r1);
            mvwprintw(dwin, 2, 74, "%-10s%d", "R2:", b.digitalInput.r2);
            mvwprintw(dwin, 3, 74, "%-10s%d", "R3:", b.digitalInput.r3);
            wrefresh(dwin);

            mvwprintw(awin, 1, 2, "%-10s%d", "triangle:", b.analogInput.triangle);
            mvwprintw(awin, 2, 2, "%-10s%d", "circle:", b.analogInput.circle);
            mvwprintw(awin, 3, 2, "%-10s%d", "cross:", b.analogInput.cross);
            mvwprintw(awin, 4, 2, "%-10s%d", "square:", b.analogInput.square);
        }
    }

    delwin(dwin);
    delwin(awin);
    delwin(pwin);
    delwin(mainwin);
    endwin();
    refresh();

    return EXIT_SUCCESS;
}


