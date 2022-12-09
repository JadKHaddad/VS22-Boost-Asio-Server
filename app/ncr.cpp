#include "ncr.hpp"
#include <ncurses.h>

void ncr::init() {
    initscr();
    refresh();
}

void ncr::end() {
    endwin();
}

void ncr::print_top_bar(const char *fmt) {
    move(0, 0);
    clrtoeol();
    printw(fmt);
    refresh();
}

void ncr::print_bottom_bar(const char *fmt) {
    move(HEIGHT + 3, 0);
    clrtoeol();
    printw(fmt);
    refresh();
}