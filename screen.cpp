#include <ncurses.h>
#include <string.h> 

int main () {
    initscr();
    char msg[] = "Hola";
    int row, col;
    if (!has_colors()) {
        endwin();
        printf("color fail");
        return 1;
    }
    start_color();
    init_pair(1, COLOR_WHITE, COLOR_BLUE);
    getmaxyx(stdscr, row, col);
    attron(COLOR_PAIR(1));
    wbkgd(stdscr, COLOR_PAIR(1));
    for (int i = 0; i < col; i++) {
        mvprintw(0, i, "%s", "#");
        mvprintw(row-1, i, "%s", "#");
    }
    for (int i = 1; i < row-1; i++) {
        mvprintw(i, 0, "%s", "#");
        mvprintw(i, col-1, "%s", "#");
    }
     mvprintw(row/2, (col - strlen(msg))/2, "%s", msg);
    attroff(COLOR_PAIR(1));
    refresh();
    getch();
    endwin();
    return 0;
}
