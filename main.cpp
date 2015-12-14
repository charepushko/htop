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

#include <termios.h>
#include <term.h>

using namespace std;

//kbhit

static struct termios initial_settings, new_settings;
static int peek_character = -1;

int _kbhit()
{
    char ch;
    int nread;

    if(peek_character != -1)
        return 1;
    new_settings.c_cc[VMIN]=0;
    tcsetattr(0, TCSANOW, &new_settings);
    nread = read(0,&ch,1);
    new_settings.c_cc[VMIN]=1;
    tcsetattr(0, TCSANOW, &new_settings);

    if(nread == 1) {
        peek_character = ch;
        return 1;
    }
    return 0;
}


// end of kbhit

struct Process {
    unsigned long int pid;
    string name;
    char status = 'S';
    unsigned long int virt = 0;
    string cmdline;
};

vector<Process> list;


enum flag {PID, NAME, VIRT, CMD};

flag curr_fl = VIRT;



//comparators

bool cmpname(Process s1, Process s2) {
    return strcasecmp(s1.name.data(), s2.name.data()) < 0;
}

bool cmppid(Process s1, Process s2) {
    return s1.pid < s2.pid;
}

bool cmpcmd(Process s1, Process s2) {
    return strcasecmp(s1.cmdline.data(), s2.cmdline.data()) < 0;
}

bool cmpvirt(Process s1, Process s2) {
    return s1.virt < s2.virt;
}

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

        closedir(fld);
}


int count1 = 0;

int render() {
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
mvprintw(1, 18, "%d", count1);
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
}

bool closer = 0;

int main() {
        noecho();
        keypad(stdscr, TRUE);

        while (true) {
            count1++;
            process_list();
            if(_kbhit()){
                char ch = getch();

                switch(ch) {
                    case 'p': curr_fl = PID; break;
                    case 'n': curr_fl = NAME; break;
                    case 'v': curr_fl = VIRT; break;
                    case 'c': curr_fl = CMD; break;

                    case 'q': closer = 1;  break;
                    default: break;
                }
            }
	    if (closer) { break;}

            switch(curr_fl) {
                case PID: sort(list.begin(), list.end(), cmppid);
                case NAME: sort(list.begin(), list.end(), cmpname);
                case VIRT: sort(list.begin(), list.end(), cmpvirt);
                case CMD: sort(list.begin(), list.end(), cmpcmd);
            }

            render();

            sleep(1);
        }

    endwin();
    return 0;
}
