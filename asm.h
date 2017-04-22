/*************************************
 * C02 Assembly Language    Routines *
 *************************************/

char cmtasm[LINELEN];     //Assembly Language Comment Text
char lblasm[LABLEN+2];    //Label to emit on next asm line

void setcmt(char *s);     //Set Assembly Line Comment to String
void addcmt(char *s);     //Append String to Assembly Line Comment
void chrcmt(char c);      //Append Character to Assembly Line Comment

void asmlin(char *opcode, char *oprnd); //Output a line of assembly code
void cmtlin();            //Output a comment lines
void equlin();            //Output an equate line

void prolog();            //Write Assembly Language Initialization Code

