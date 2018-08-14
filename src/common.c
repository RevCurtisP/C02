/*************************************
 * C02 Common Definitions & Routines *
 *************************************/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include "common.h"

struct timespec curtim; //Current Time

/* Error - Print Input File name & position and exit */
void exterr(int errnum) {
  fprintf(stderr, "Line %d Column %d of File %s\n", curlin, curcol, inpnam);
  exit(errnum);
}

/* Error - print "Expected" error message            *
           and exit with general failure code        *
   Args: expected - Description of what was expected */ 
void expctd(char *expstr) {
  fprintf(stderr, "Expected %s, but found '%c'\n", expstr, nxtchr);
  exterr(EXIT_FAILURE);
}

/* Print current position in file */
void prtpos(void) { if (inpnam[0]) printf("(%s: %d,%d) ", inpnam, curlin, curcol); }

/* Initialize elapsed time counter */
void initim(void) {
  timespec_get (&curtim, TIME_UTC);
  bgntim = curtim.tv_sec;
} 

/* Print elapsed time */
void prttim(void) {
  timespec_get (&curtim, TIME_UTC);
  printf("[%d", curtim.tv_sec - bgntim);
  printf(".%06d]",curtim.tv_nsec/1000);
}

/* Set comment to string */
void setcmt(char *s) { strcpy(cmtasm, s); }

/* Append string to comment */
void addcmt(char *s) { 
  if (strlen(cmtasm)+strlen(s)<73) strcat(cmtasm, s); 
}

/* Append character to comment */
void chrcmt(char c) {
  if (strlen(cmtasm)>72) return;
  if (cmtasm[0] == 0 && c == ' ') return;
  int i = strlen(cmtasm);
  cmtasm[i++] = c;
  cmtasm[i] = 0;
}
