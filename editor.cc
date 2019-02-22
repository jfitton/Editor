#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/time.h>
#include <strings.h>
#include <string.h>
#include <vector>
#include <string>

#include "editor.hh"

#define MAX_LINE_BUFFER 2048

using namespace std;

struct termios original_tty;
int mode = 1;                                                       // current input mode (0-base, 1-write)

std::vector<char *> lines;                                          // vector consisting of the lines of the text file
std::vector<int> line_len;                                          // vector consisting of the lengths of the lengths of each of the lines of text
int currLine;                                                       // index of the current line
int linePos;                                                        // cursor position in the current line

char * fileName;                                                    // name of the file that was opened
FILE * file;


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

bool affirm(const char * prompt) {
    printf("%s (Y/N)\n", prompt);
    char c = 0;
    while(c != EOF) {
        read(0,&c,1);
        if (c == 'n' || c == 'N')
            return false;
        else if (c == 'y' || c == 'Y')
            return true;
        else 
            return affirm("Please respond with a valid response.");
    }
}

void clrscr() {                                                     // clears the screen of text (need to shift up 1 line after to have cursor 
    printf("\e[1;1H\e[2J");
    fflush(0);
    currLine = 0;
}

void alignCursor() {
    while (linePos > line_len[currLine]) {
        cursorLeft();
    }
}

bool cursorUp(){
    if (currLine <= 0) return false;
    char ch;
    ch = 27;
    write(1,&ch,1);
    ch = '[';
    write(1,&ch,1);
    ch = 'A';
    write(1,&ch,1);
    currLine--;
    alignCursor();
    return true;
}

bool cursorDown(){
    if (currLine >= lines.size()-1) return false;
    char ch;
    ch = 27;
    write(1,&ch,1);
    ch = '[';
    write(1,&ch,1);
    ch = 'B';
    write(1,&ch,1);
    currLine++;
    alignCursor();
    return true;
}

bool cursorRight(){
    if (linePos < line_len[currLine]){
        char ch;
        ch = 27;
        write(1,&ch,1);
        ch = '[';
        write(1,&ch,1);
        ch = 'C';
        write(1,&ch,1);
        linePos++;
        return true;
    }
    return false;
}

bool cursorLeft(){
    if(linePos > 0){
        char ch;
        ch = 27;
        write(1,&ch,1);
        ch = '[';
        write(1,&ch,1);
        ch = 'D';
        write(1,&ch,1);
        linePos--;
        return true;
    }
    return false;
}

bool saveFile(){
}

void closeProgram(){
    clrscr();
    for (int i = 0; i < lines.size(); i++){
        printf("%s\n",lines[i]);
    }
    exit(0);
}

void clearLine(int startPos) {
    while(linePos > 0)
        cursorLeft();
    char ch = ' ';
    for(int i = 0; i<line_len[currLine]; i++){
        write(1,&ch,1);
        linePos++;
    }
    while(linePos!=startPos)
        cursorLeft();
}

void clear(int startPos){
    if(startPos==0)
        clearLine(linePos);
}

void clear(){
    clearLine(linePos);
}

void shiftLines(int startPos){
    int startLine = currLine;
    if(startPos==0)
        clear();
    else{

    }
    for(int i=currLine;i<lines.size();i++){
        cursorDown();
        clear(0);

        for(int j=0;j<line_len[i];j++){
            char ch = lines[i][j];
            write(1,&ch,1);
            linePos++;
        }

        for(int j=0;j<line_len[i];j++){
            cursorLeft();
        }

    }
    while(currLine!=startLine){
        bzero(lines[currLine], MAX_LINE_BUFFER);
        strcpy(lines[currLine], lines[currLine-1]);
        line_len[currLine] = line_len[currLine-1];
        cursorUp();
    }
    bzero(lines[currLine], MAX_LINE_BUFFER);
    line_len[currLine] = 0;
}

void shiftLines(){
    shiftLines(0);
}

void newLine(){
    if(currLine == lines.size()-1){
    } else if(currLine == 0) {
    }else {
        if(linePos == 0) {
            //            clearLine(linePos);

            char newLine[MAX_LINE_BUFFER];
            bzero(newLine, MAX_LINE_BUFFER);
            char * lineCopy = (char *)malloc(sizeof(char) * MAX_LINE_BUFFER);
            strcpy(lineCopy, newLine);
            lines.push_back(lineCopy);
            line_len.push_back(0);
            shiftLines();
        }
    }
}

bool writeInput(char ch){
    if (ch == '\n') {
        newLine();
        /*        lines.push_back(newLine);
                  clearLine();
                  for (int i = lines.size()-1; i > currLine; i--){
                  lines[i] = strcpy(lines[i], lines[i-1]);
                  line_len[i] = line_len[i-1];
                  bzero(lines[i], MAX_LINE_BUFFER);
                  }
                  line_len[currLine] = 0;
                  for (int i=currLine; i<lines.size();i++) {
                  cursorDown();
                  clearLine();
                  stepLeft(line_len[i]);
                  printf("%s",lines[i]);
                  stepLeft(line_len[i]);
                  }
                  line_len[currLine] = 0;*/
        /*        for (int i = 0; i < lines.size(); i++){
                  printf("%s\n",lines[i]);
                  }*/
    }
    if (32 <= ch && ch <= 126){                                     // writes character typed to stdout if it is a printable character
        line_len[currLine]++;
        int tmpPos = line_len[currLine];
        while (tmpPos != linePos) {
            lines[currLine][tmpPos] = lines[currLine][tmpPos-1];
            tmpPos--;
        }

        lines[currLine][linePos] = ch;
        linePos++;
        write(1,&ch,1);
        tmpPos++;

        while (tmpPos != line_len[currLine]) {
            ch = lines[currLine][tmpPos];
            write(1,&ch,1);
            tmpPos++;
        }
        while (tmpPos!= linePos) {
            linePos++;
            cursorLeft();
            tmpPos--;
        }
        return true;
    }
    return false;
}

void escape(){                                                      // executes escape sequences such as cursor movement
    char ch1,ch2;
    read(0,&ch1,1);
    read(0,&ch2,1);
    if (ch1 == 91) {
        switch(ch2){
            case 'A':
                cursorUp();
                break;
            case 'B':
                cursorDown();
                break;
            case 'C':
                cursorRight();
                break;
            case 'D':
                cursorLeft();
                break;
        }
    }
}

void changeMode(char ch){
    if(ch == 'x'){
        closeProgram();                                             // special case for exitting program
    }
    if(ch=='p'){
        for(int i=0; i<lines.size();i++){
            printf("%s\n", lines[i]);
        }
    }
    if (ch == 'i'){                                                 // changes to write/insert mode
        mode = 1;
    }else if(ch = '0'){
        mode = 0;                                                   // changes to base mode
    }
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
            changeMode('0');
        } else {                                                    // escape sequence detected, performs action based on sequence
            escape();
        }


    }

    if (32 <= ch <= 126 || ch == '\n'){                                          
        if (mode == 0) {                                            // if input is a printable character and the program is in base mode, changes mode accordingly (more features in future)
            changeMode(ch);
        } else if (mode == 1) {                                     // if input is a printable character and the program is in write/insert mode, writes the input to terminal and file
            writeInput(ch);
        }
    }
}

void readInput(){
    while(1){
        char ch;
        read(0,&ch,1);
        execInput(ch);
        //        printf("%d\n",ch);                                          // prints the numerical value of the character(s) typed (for error checking purposes)
        if(ch == 127) {
            clrscr();
        }
    }
}

/* When given the name of a file, checks whether or not the file exists. */

bool fileExists(char * filename) {
    file = fopen(filename, "r+");
    if (file == NULL) {
        return false;
    }
    return true;
}

void readFile(char * filename) {
    char * fileLine = new char[MAX_LINE_BUFFER];
    bzero(fileLine, MAX_LINE_BUFFER);
    int i = 0;
    char ch;
    while ((ch = getc(file)) != EOF){
        if (ch != '\n') fileLine[i] = ch;
        else {
            char * newLine;
            newLine = (char *)malloc(sizeof(char) * MAX_LINE_BUFFER);
            bzero(newLine, MAX_LINE_BUFFER);
            strcpy(newLine, fileLine);
            lines.push_back(newLine);
            line_len.push_back(strlen(newLine));
            bzero(fileLine,MAX_LINE_BUFFER);
            i = -1;
        }
        i++;
    }
}

FILE * createNewFile(char * filename){
    return fopen(filename, "a+");
}

int main(int argc, char * argv[]) {

    tty_raw_mode();

    if (argc >= 2) {
        if (!fileExists(argv[1])){
            clrscr();
            std::string filen = argv[1];
            std::string fileNotFound("File \'" + filen + "\' could not be found.\n");
            printf("%s", fileNotFound.c_str());
            std::string fileCreation("Would you like to create file " + filen + "?");
            if(affirm(fileCreation.c_str())) file = createNewFile(argv[1]);
            else exit(0);
        }
        readFile(argv[1]);
    }
    clrscr();
    for (int i = 0; i < lines.size(); i++){
        printf("%s\n", lines[i]);
        currLine++;
    }
    while (currLine != 0) {
        cursorUp();
    }
    //    exit(0);

    if (lines.size() == 0) {
        char * line = new char[MAX_LINE_BUFFER];
        int len = 0;
        lines.push_back(line);
        line_len.push_back(len);
    }
    readInput();
}

