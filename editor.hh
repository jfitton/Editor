#ifndef editor_hh
#define editor_hh

// implemented (partially or completely)

int reset_tty(void);
void tty_at_exit(void);
void tty_raw_mode(void);
void clrscr();
void closeProgram();
void writeInput(char);
void escape();
void changeMode(char);
void execInput(char);
void readInput();
void cursorUp();
void cursorDown();
void cursorLeft();
void cursorRight();

// currently unimplemented

void cursorHome();
void cursorEnd();

#endif
