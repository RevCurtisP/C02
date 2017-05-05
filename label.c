/******************************************************
 * C02 Label Parsing, Generation, and Lookup Routines *
 ******************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>

#include "common.h"
#include "asm.h"
#include "parse.h"
#include "label.h"

/* Find Last Label of Specified Type *
 * Args: lbtype: Label type          *
 * Sets: tmplbl - Label name         *
 * Returns: Index into label table   *
 *          (-1 if not found)        */
int lstlbl(int lbtype)
{
  int i;
  DEBUG("Searching for label type %d\n", lbtype);
  for (i = lblcnt - 1; i>-1; i--) 
    if (lbltyp[i] == lbtype) break;
  DEBUG("Search produced label index %d\n", i);
  if (i>=0) 
    strcpy(tmplbl, lblnam[i]);
  return i;
}

/* Set Block Flag for Last Label */
void setblk(int blkflg)
{
  lblblk[lblcnt-1] = blkflg;
}

/* Set label for next line of *
 * Assembly Language Code     *
 * to word                    */
void setlbl(char *lblset)
{
  DEBUG("Setting Label '%s'\n", lblset);
  if (strlen(lblasm) > 0)
    asmlin("",""); //Emit Block End Label on it's own line
  if (strlen(lblset) > LABLEN) 
    ERROR("Label '%s' exceeds maximum size\n", word, EXIT_FAILURE);
  strcpy(lblasm, lblset);
}

/* parse label in code */
void prslbl()
{
  DEBUG("Parsing Label '%s''\n", word);
  skpchr(); //skip ':'
  setlbl(word);
}

/* generate new label */
void newlbl() 
{
  sprintf(curlbl, LABFMT, lblnxt++);
  DEBUG("Generated new label '%s'\n", curlbl);
}

/* Pop Label from Stack and Emit on Next Line */
int poplbl() 
{
  int lbtype = lbltyp[--lblcnt];
  DEBUG("Popped label type %d\n", lbtype);
  if (lbtype == LTLOOP)
    asmlin("JMP", lblnam[lblcnt--]); //Jump to Beginning of Loop   
  if (lbtype == LTDO)
    strcpy(loplbl, lblnam[lblcnt]);
  else
    setlbl(lblnam[lblcnt]);
  inblck = lblblk[lblcnt-1];
  return lbtype;
}

/* Push Label onto Stack        *
 * Args: lbltyp - Label type    *
 * Uses: curlbl - Label to push */
void pshlbl(int lbtype) 
{
  DEBUG("Pushing label type %d\n", lbtype);
  strcpy(lblnam[lblcnt], curlbl);
  lbltyp[lblcnt] = lbtype;
  lblblk[lblcnt++] = FALSE;
  DEBUG("Pushed label '%s' onto stack\n", curlbl);
}

