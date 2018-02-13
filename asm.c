/*************************************
 * C02 Assembly Language    Routines *
 *************************************/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>

#include "common.h"
#include "files.h"
//#include "parse.h"
#include "asm.h"

/* Process comment */
void prccmt()
{
  if (strlen(cmtasm)) {
    strcpy(asmcmt, ";");
    strcat(asmcmt, cmtasm);
  }
  else
    asmcmt[0] = 0;
  setcmt("");
}

/* output a single line of assembly code */
void asmlin(char *opcode, char *oprnd)
{
  if (strlen(lblasm)) strcat(lblasm, LABSFX);
  prccmt();
  fprintf(outfil, ASMFMT, lblasm, opcode, oprnd, asmcmt);
  if (debug) printf(ASMFMT, lblasm, opcode, oprnd, asmcmt);
  lblasm[0] = 0;
}

/* output a single comment line */
void cmtlin()
{
  DEBUG("Writing Comment Line: %s\n", cmtasm);
  fprintf(outfil, "; %s\n", cmtasm);
  setcmt("");
}

/* Output a variable definition line *
 * Uses: word - assembly oprnd     */
void equlin()
{
  fprintf(outfil, ASMFMT, lblasm, EQUOP, word, "");
  if (debug) printf(ASMFMT, lblasm, EQUOP, word, "");
  lblasm[0] = 0;
}

void prolog()
{
  DEBUG("Writing Assembly Prolog\n", 0);
  asmlin(CPUOP,CPUARG);
  strcpy(cmtasm, "Program ");
  strcat(cmtasm, srcnam);
  cmtlin();
}
