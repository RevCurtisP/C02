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

/* Set label for next line of *
 * Assembly Language Code     *
 * to word                    */
void setlbl(char *lblset)
{
  DEBUG("Setting Label '%s''\n", lblset);
  if (strlen(lblasm) > 0)
    asmlin("",""); //Emit Block End Label on it's own line
  if (strlen(word) > LABLEN) 
    ERROR("Label '%s' exceeds maximum size", word, EXIT_FAILURE);
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
void newlbl(int lbtype) 
{
  sprintf(curlbl, LABFMT, lblnxt++);
  DEBUG("Generated new label '%s'\n", curlbl);
  strcpy(lblnam[lblcnt++], curlbl);
  lbltyp[lblcnt] = lbtype;
}

/* set label to emit on next line and remove from stack */
void lbllin() 
{
  int lbtype = lbltyp[--lblcnt];
  if (lbtype == 1) {
      DEBUG("Ending Loop at Level %d'\n", lblcnt);
      strcpy(endlbl, lblnam[lblcnt]);  //Save End of Loop Label
      asmlin("JMP", lblnam[--lblcnt]); //Jump to Beginning of Loop   
      setlbl(endlbl);                  //Set End of Loop Label
  }
  else
    setlbl(lblnam[lblcnt]);
 DEBUG("Set label '%s' to emit on next line\n", lblasm);
}


