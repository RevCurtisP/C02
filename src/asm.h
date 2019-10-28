/*************************************
 * C02 Assembly Language    Routines *
 *************************************/

char lblasm[LBLLEN+2];  //Label to emit on next asm line

void asmlin(char *opcode, char *oprnd); //Output a line of assembly code
void cmtlin();          //Output a comment lines
void prccmt();          //Process comment
