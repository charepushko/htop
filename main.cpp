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
using namespace std;

struct Process {
	unsigned long int pid;
	string name;
	char status = 'S';
	unsigned long int virt = 0;
	string cmdline;
};

bool cmpname(Process s1, Process s2) {
    return strcasecmp(s1.name.data(), s2.name.data()) < 0;
}


int main() {

	string path = "/proc";
		DIR* fld = opendir(path.data());

	if (fld == NULL) {
		return 1;
	}


	vector<Process> list;

	start:
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

		closedir(fld);

	    if (getch() == 'S') {
		sort(list.begin(), list.end(), cmpname);
	    }

	    initscr();
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
	                mvprintw(2, 1, "%s", "PID");
	                mvprintw(2, 11, "%s", "NAME");
	                mvprintw(2, 31, "%s", "STAT");
	                mvprintw(2, 37, "%s", "VIRT");
	                mvprintw(2, 52, "%s", "COMMAND LINE");
	    attroff(COLOR_PAIR(2));
	    attron(COLOR_PAIR(1));

	    unsigned n;
	    if (row < list.size()) {
		n = row-3;
	    } else { n = list.size()-3; }

		for (unsigned int i = 0; i < list.size(); i++) {
		        mvprintw(i+3, 1, "%lu", list[i].pid);
		        mvprintw(i+3, 11, "%s", list[i].name.data());
		        mvprintw(i+3, 31, "%c", list[i].status);
		        mvprintw(i+3, 37, "%lu", list[i].virt);
		        mvprintw(i+3, 52, "%s", list[i].cmdline.data());
		}

	    attroff(COLOR_PAIR(1));
	    refresh();
	    if (getch()) {
		    endwin();
	    } else { goto start;}


	return 0;
}
