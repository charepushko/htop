#include <iostream>
#include <sys/stat.h>
#include <dirent.h>
#include <strings.h>
#include <string>
#include <algorithm>
#include <unistd.h>
#include <vector>
#include <stdlib.h>
#include <fstream>
#include <ncurses.h>
#include <curses.h>
#include <stdio.h>
#include <signal.h>

#include <sys/time.h>
#include <sys/types.h>

#include <termios.h>
#include <term.h>

using namespace std;

struct Process {
    unsigned long int pid;
    string name;
    char status = 'S';
    unsigned long int virt = 0;
    string cmdline;
};

vector<Process> list;

Process get_proc(unsigned long int pid) {
	Process res;
	for (int i = 0; i < list.size(); i++) {
		if (list[i].pid == pid) {
			res = list[i];
		}
	}
	return res;
}



enum flag {PID, NAME, VIRT, CMD};

flag curr_fl = VIRT;



//comparators

struct sort_name {
	bool operator() (Process s1, Process s2) {
	    return strcasecmp(s1.name.data(), s2.name.data()) < 0;
	}
} cmpname;

struct sort_pid {
	bool operator() (Process s1, Process s2) {
	    return (s1.pid < s2.pid);
	}
} cmppid;

struct sort_cmd {
	bool operator() (Process s1, Process s2) {
	    return strcasecmp(s1.cmdline.data(), s2.cmdline.data()) < 0;
	}
} cmpcmd;

struct sort_virt {
	bool operator() (Process s1, Process s2) {
	    return (s1.virt < s2.virt);
	}
} cmpvirt;

// end cmpr


int process_list() {
    string path = "/proc";
        DIR* fld = opendir(path.data());

    if (fld == NULL) {
        return 1;
    }
    list.clear();
    while(struct dirent* dir = readdir(fld)) {
        string new_name(dir->d_name);
        string new_path =  path + '/' + new_name;
        struct stat well;
        if ((lstat(new_path.data(), &well) == 0) && (S_ISDIR(well.st_mode)) && (new_name != ".") && (new_name != "..") && (atoi(new_name.data()) > 0)) {
            ifstream cmd (new_path + "/" + "cmdline");
            Process proc;
            string well;
            cmd >> well;
            proc.cmdline = well;

            cmd.close();

            ifstream stat (new_path + "/" + "stat");
            stat >> well;
            proc.pid = atoi(well.data());
            stat >> proc.name;
            stat >> well;
            proc.status = well[0];
            for (int i = 0; i < 19; i++) { stat >> well;}
            stat >> well;
            proc.virt = atoi(well.data());
            if  (proc.virt != 0) {
                list.push_back(proc);
            }

            stat.close();
        }
    }


            switch(curr_fl) {
                case PID: sort(list.begin(), list.end(), cmppid); break;
                case NAME: sort(list.begin(), list.end(), cmpname); break;
                case VIRT: sort(list.begin(), list.end(), cmpvirt); break;
                case CMD: sort(list.begin(), list.end(), cmpcmd); break;
            }

    closedir(fld);
}

int cursor = 3;
unsigned long int cursor_pid;
int count1 = 0;

int render() {
	    clear();
            unsigned int row, col;
            if (!has_colors()) {
                endwin();
                printf("color fail");
                return 1;
            }
            start_color();
            init_pair(1, COLOR_WHITE, COLOR_BLUE);
            init_pair(2, COLOR_YELLOW, COLOR_BLACK);
            getmaxyx(stdscr, row, col);
            attron(COLOR_PAIR(1));
            wbkgd(stdscr, COLOR_PAIR(1));
            attroff(COLOR_PAIR(1));
            attron(COLOR_PAIR(2));
			for (int j = 0; j < col; j++) {
				mvprintw(2, j, "%c", ' ');
			}
                        mvprintw(2, 1, "%s", "PID");
                        mvprintw(2, 11, "%s", "NAME");
                        mvprintw(2, 31, "%s", "STAT");
                        mvprintw(2, 37, "%s", "VIRT");
                        mvprintw(2, 52, "%s", "COMMAND LINE");
            attroff(COLOR_PAIR(2));
            attron(COLOR_PAIR(1));
//mvprintw(1, 18, "%d", count1);
            unsigned n;
            if (row < list.size()) {
            n = row-3;
            } else { n = list.size()-3; }

	    init_pair(3, COLOR_BLACK, COLOR_GREEN);
            for (unsigned int i = 0; i < list.size(); i++) {
		if (i == cursor) {
			attron(COLOR_PAIR(3));
			cursor_pid = list[i].pid;
			for (int j = 0; j < col; j++){
				mvprintw(i+3, j, "%c", ' ');
			}
		}
                    mvprintw(i+3, 52, "%s", list[i].cmdline.data());
                    mvprintw(i+3, 0, "%s", "                     ");
                    mvprintw(i+3, 1, "%lu", list[i].pid);
                    mvprintw(i+3, 11, "%s", list[i].name.data());
                    mvprintw(i+3, 31, "%c", list[i].status);
                    mvprintw(i+3, 37, "%lu", list[i].virt);
		if (i == cursor) {
			attroff(COLOR_PAIR(3));
			attron(COLOR_PAIR(1));
		}

            }
	mvprintw(1, 1, "%c", ' ');

            attroff(COLOR_PAIR(1));
            refresh();
}

void pause_resume() {
	Process well = get_proc(cursor_pid);
	if (well.status == 'T') {
		kill(cursor_pid, SIGCONT);
	} else {
		kill(cursor_pid, SIGSTOP);
	}
}


void to_kill() {
	attron(COLOR_PAIR(2));
	unsigned int row, col;
	getmaxyx(stdscr, row, col);
	for (int i = (row/2 - 3); i < (row/2+3); i++) {
		for (int j = (col/2-17); j <= (col/2+17); j++) {
			mvprintw(i, j, "%c", ' ');
		}
	}

	mvprintw((row/2-2), (col/2-15), "%s", "Are you sure you want to kill");
	mvprintw((row/2-1), (col/2-15), "%s", "the process with pid        ?");
	init_pair(4, COLOR_WHITE, COLOR_BLACK);
	attroff(COLOR_PAIR(2));
	attron(COLOR_PAIR(4));
	mvprintw((row/2-1), (col/2+6), "%lu", cursor_pid);
	attroff(COLOR_PAIR(4));
	attron(COLOR_PAIR(3));
	mvprintw((row/2+1), (col/2-9), "%s", " YES (Y) ");
	mvprintw((row/2+1), (col/2+1), "%s", " NO (N) ");
	char g;
	while (g = getch()) {
		if (g == 'y') {
			kill(cursor_pid, SIGTERM);
			sleep(1);
			kill(cursor_pid, SIGKILL);
			mvprintw((row/2), (col/2-4), "%s", " DONE! ");
			sleep(2);
			break;
		}
		if (g == 'n') { break;}
	}
	attroff(COLOR_PAIR(3));
}


bool closer = 0;

int main() {
        initscr();
//	scrollok(stdscr, TRUE);
	keypad(stdscr, TRUE);
        noecho();

        fd_set open;
	int getval;
	struct timeval waiter;


        while (true) {
	    FD_ZERO(&open);
	    FD_SET(0, &open);

	    waiter.tv_sec = 0;
            waiter.tv_usec = 100;

            getval = select(2, &open, NULL, NULL, &waiter);
		keypad(stdscr, TRUE);
            count1++;
            process_list();
            if(getval){
                char ch = getch();

                switch(ch) {
                    case 'p': curr_fl = PID; break;
                    case 'n': curr_fl = NAME; break;
                    case 'v': curr_fl = VIRT; break;
                    case 'c': curr_fl = CMD; break;

		    case 'k': to_kill(); break;
		    case 't': pause_resume(); break;

		    case 'w':
		    case KEY_UP:  if(cursor > 0) { cursor--; } break;

		    case 's':
		    case KEY_DOWN:  if(cursor < list.size()-1) { cursor++; } break;

                    case 'q': closer = 1;  break;
                    default: break;
                }
            }
	    if (closer) { break;}

            render();

            usleep(100000);
        }
    echo();
    endwin();
    return 0;
}
