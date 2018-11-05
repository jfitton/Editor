#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/time.h>
#include <vector>

#define MAX_LINE_BUFFER 2048


struct termios original_tty;
int mode = 1;                                                       // current input mode (0-base, 1-write)

std::vector<char *> lines;                                          // vector consisting of the lines of the text file
std::vector<int> line_len;                                          // vector consisting of the lengths of the lengths of each of the lines of text
int currLine;                                                       // index of the current line
int linePos;                                                        // cursor position in the current line

char * fileName;                                                    // name of the file that was opened


int reset_tty(void) {
    if(tcsetattr(0,TCSAFLUSH,&original_tty) < 0) return -1;
    return 0;
}

void tty_at_exit(void) {
    reset_tty();
}

void tty_raw_mode(void) {
    tcgetattr(0,&original_tty);
    atexit(tty_at_exit);

    struct termios tty_attr;
    tcgetattr(0,&tty_attr);

    tty_attr.c_lflag &= (~(ICANON|ECHO));
    tty_attr.c_cc[VTIME] = 0;
    tty_attr.c_cc[VMIN] = 1;

    tcsetattr(0, TCSANOW, &tty_attr);
}

void clrscr() {                                                     // clears the screen of text (need to shift up 1 line after to have cursor 
    printf("\e[1;1H\e[2J");
    fflush(0);
}

void writeInput(char ch){
    if (32 <= ch && ch <= 126){                                     // writes character typed to stdout if it is a printable character

        write(1,&ch,1);
    }
}

void escape(){                                                      // executes escape sequences such as cursor movement
    return;
}

void execInput(char ch){


    if (ch == 27) {                                                 // checks for escape character (ch == 27) and for other characters immediately after for escape sequences
        struct timeval timeout;
        timeout.tv_usec = 250;
        timeout.tv_sec = 0;
        fd_set input_set;
        FD_ZERO(&input_set);
        FD_SET(0,&input_set);
        int sequence = select(1,&input_set,NULL,NULL, &timeout);


        if (sequence == 0){                                         // no escape sequence, sets mode to 0 (base mode)
            mode = 0;
        } else {                                                    // escape sequence detected, performs action based on sequence
            escape();
        }


    }


    if (mode == 0) {
    } else if (mode == 1) {
        writeInput(ch);
    }
}

void read(){
    while(1){
        char ch;
        read(0,&ch,1);
        execInput(ch);
//        printf("%d\n",ch);                                          // prints the numerical value of the character(s) typed (for error checking purposes)
        if(ch == 127) {
            clrscr();
        }else {
            write(1,&ch,1);
        }
    }
}

int main(int argc, char * argv[]) {
    tty_raw_mode();
    clrscr();

    if (lines.size() == 0) {
        char * line  = char[MAX_LINE_BUFFER];
        lines.push_back(line);
    }
    read();
}



