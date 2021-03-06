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
int shift = 0;
unsigned int row;

int render() {
	    clear();
            unsigned int col;
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

            init_pair(6, COLOR_BLACK, COLOR_YELLOW);
            init_pair(7, COLOR_MAGENTA, COLOR_YELLOW);
	    attron(COLOR_PAIR(6));
		mvprintw(0, 0, "%s", "  INFO  STOP  KILL  sort by: NAME PID CMDLINE VIRTMEM  QUIT   ");
	    attroff(COLOR_PAIR(6));
	    attron(COLOR_PAIR(7));
		mvprintw(0, 2, "%c", 'I');
		mvprintw(0, 8, "%c", 'S');
		mvprintw(0, 14, "%c", 'K');
		mvprintw(0, 29, "%c", 'N');
		mvprintw(0, 34, "%c", 'P');
		mvprintw(0, 38, "%c", 'C');
		mvprintw(0, 46, "%c", 'V');
		mvprintw(0, 55, "%c", 'Q');
	    attroff(COLOR_PAIR(7));



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
//mvprintw(1, 18, "%d", shift);
//mvprintw(1, 23, "%d", cursor);
            unsigned n;
            if (row < list.size()) {
	            n = row-3;
            } else { n = list.size(); }

	    init_pair(3, COLOR_BLACK, COLOR_GREEN);
            for (unsigned int i = 0; i < n; i++) {
		if (list[i+shift].pid == cursor_pid) {
			attron(COLOR_PAIR(3));
//			cursor_pid = list[i].pid;
			for (int j = 0; j < col; j++){
				mvprintw(i+3, j, "%c", ' ');
			}
		}
                    mvprintw(i+3, 52, "%s", list[i+shift].cmdline.data());
                    mvprintw(i+3, 0, "%s", "                     ");
                    mvprintw(i+3, 1, "%lu", list[i+shift].pid);
                    mvprintw(i+3, 11, "%s", list[i+shift].name.data());
                    mvprintw(i+3, 31, "%c", list[i+shift].status);
                    mvprintw(i+3, 37, "%lu", list[i+shift].virt);
		if (list[i+shift].pid == cursor_pid) {
			attroff(COLOR_PAIR(3));
			attron(COLOR_PAIR(1));
		}

            }
	mvprintw(1, 1, "%c", ' ');

            attroff(COLOR_PAIR(1));
            refresh();
}



int process_info() {
    string path = "/proc";
        DIR* fld = opendir(path.data());

    if (fld == NULL) {
        return 1;
    }
    while(struct dirent* dir = readdir(fld)) {
        string new_name(dir->d_name);
        string new_path =  path + '/' + new_name;
        struct stat well;
        if ((lstat(new_path.data(), &well) == 0) && (S_ISDIR(well.st_mode)) && (new_name != ".") && (new_name != "..") && (atoi(new_name.data()) > 0)) {
            string well;

            ifstream stat (new_path + "/" + "stat");
            stat >> well;

//mvprintw(1, 25, "%d", cursor_pid);
//mvprintw(1, 35, "%d", atoi(well.data()));

            stat.close();

            if (cursor_pid == atoi(well.data())) {

// mvprintw(1, 45, "%d", 1);


	        int col;
	        getmaxyx(stdscr, row, col);
	        attron(COLOR_PAIR(2));

	        for (int i = 2; i < row-2; i++) {
	                for (int j = 3; j < col-3; j++) {
	                        mvprintw(i, j, "%c", ' ');
	                }
	        }
	        mvprintw(3, col/2-10, "%s", "PROCESS INFORMATION");

	        attroff(COLOR_PAIR(2));
	        init_pair(5, COLOR_WHITE, COLOR_BLACK);
	        attron(COLOR_PAIR(5));

	        int array[12] = {0, 1, 2, 4, 5, 7, 8, 9, 12, 17, 23, 24};
	        int r = 0;
		int roww = 5;
	        int n;
	        if (row >= 35) { n = 25; } else { n = row-9; }

	        ifstream status (new_path + "/" + "status");
	        string well1 = "hey";
	        for (int i = 0; i < n; i++) {
//	          status >> well1;
	          getline(status, well1);
	            if (i == array[r]) {
	                mvprintw(roww, 6, "%s", well1.data());
	                r++;
			roww++;
	            }
	        }
	        attroff(COLOR_PAIR(5));
	        attron(COLOR_PAIR(3));
	        mvprintw(row-4, col/2-4, "%s", "  Okay  ");

	        getch();

	            status.close();

	        attroff(COLOR_PAIR(3));
		break;

	    }

	}
    }
    closedir(fld);

}








void pause_resume() {
	Process well = get_proc(cursor_pid);
	if (well.status == 'T') {
		kill(cursor_pid, SIGCONT);
		sleep(1);
	} else {
		kill(cursor_pid, SIGSTOP);
		sleep(1);
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
                int ch = getch();

                switch(ch) {
                    case 'p': curr_fl = PID; break;
                    case 'n': curr_fl = NAME; break;
                    case 'v': curr_fl = VIRT; break;
                    case 'c': curr_fl = CMD; break;

		    case 'k': to_kill(); break;
		    case 's': pause_resume(); break;
		    case 'i': process_info(); break;

		    case KEY_UP:  if(cursor > 0) {
					if ((cursor == shift) && (shift > 0)) {
						shift -= 1;
					}
					cursor--;
					cursor_pid = list[cursor].pid;
				} break;

		    case KEY_DOWN:  if(cursor < list.size()-1) {
					if ((cursor + 5 > row) && (shift < (list.size() - row + 3))) {
						shift++;
					}
					cursor++;
					cursor_pid = list[cursor].pid;
				} break;

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
