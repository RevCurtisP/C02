/*************************************
 * C02 Common Definitions & Routines *
 *************************************/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>

#include "common.h"

/* Error - Print Input File name & position and exit */
void exterr(int errnum)
{
  fprintf(stderr, "Line %d Column %d of File %s\n", curlin, curcol, inpnam);
  exit(errnum);
}

/* Error - print "Expected" error message            *
           and exit with general failure code        *
   Args: expected - Description of what was expected */ 
void expctd(char *expstr)
{
  fprintf(stderr, "Expected %s, but found '%c'\n", expstr, nxtchr);
  exterr(EXIT_FAILURE);
}

/* Print current position in file */
void prtpos() 
{
  printf("(%s: %d,%d) ", inpnam, curlin, curcol);
}


