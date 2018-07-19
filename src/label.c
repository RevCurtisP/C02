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

const char lblflg[] = {LFNONE, LFNONE, LFNONE, LFBGN, LFEND, LFBGN, LFEND, LFEND, LFNONE, LFNONE}; //Label Type Flags
//        enum ltypes {LTNONE, LTIF, LTELSE, LTLOOP, LTEND, LTDO, LTDWHL, LTSLCT, LTCASE, LTFUNC}; //Label Types

/* Find Last Label of Specified Types *
 * Args: lbtyp1: First label type     *
 *       lbtyp2: Second label type    *
 * Sets: tmplbl - Label name          *
 * Returns: Index into label table    *
 *          (-1 if not found)         */
int lstlbl(int lbflag) {
  int i;
  DEBUG("Searching for label flag %d\n", lbflag)
  for (i = lblcnt - 1; i>-1; i--)
    if (lblflg[lbltyp[i]] == lbflag) break;
  DEBUG("Search produced label index %d\n", i)
  if (i>=0) strcpy(tmplbl, lblnam[i]);
  return i;
}

/* Set Block Flag for Last Label */
void setblk(int blkflg) { lblblk[lblcnt-1] = blkflg; }

/* Set label for next line of *
 * Assembly Language Code     *
 * to word                    */
void setlbl(char *lblset) {
  DEBUG("Setting Label '%s'\n", lblset)
  if (strlen(lblasm) > 0) {
    DEBUG("Emitting Label '%s'\n'", lblasm);
    asmlin("",""); //Emit Block End Label on it's own line
  }
  if (strlen(lblset) > LABLEN) ERROR("Label '%s' exceeds maximum size\n", word, EXIT_FAILURE)
  strcpy(lblasm, lblset);
}

/* parse label in code */
void prslbl(void) {
  DEBUG("Parsing Label '%s''\n", word)
  CCMNT(nxtchr);
  skpchr(); //skip ':'
  setlbl(word);
}

/* generate new label */
void newlbl(char* lbname) {
  sprintf(lbname, LABFMT, lblnxt++);
  DEBUG("Generated new label '%s'\n", lbname)
}

/* Check Label Contents             *
 * If lbname is blank, generate new * 
 * label and copy into lbname       */
void chklbl(char* lbname) {
  if (lbname[0]) return;
  newlbl(lbname);
}

/* Require Label                               *
 * if label is already set, returns that label *
 * else generates new label and sets it        */
void reqlbl(char* lbname) {
  if (lblasm[0] == 0) newlbl(lbname);
  setlbl(lbname);
}

/* Pop Label from Stack and Emit on Next Line */
int poplbl(void) {
  int lbtype = lbltyp[--lblcnt];
  DEBUG("Popped label type %d\n", lbtype)
  switch (lbtype) {
    case LTFUNC: if (!lsrtrn) asmlin("RTS", ""); break;  //Return From Subroutine
    case LTDO:   strcpy(loplbl, lblnam[lblcnt]); break;  
    case LTDWHL: strcpy(endlbl, lblnam[lblcnt]); break;  
    case LTCASE: strcpy(cndlbl, lblnam[lblcnt]); break;  
    case LTLOOP: asmlin("JMP", lblnam[lblcnt--]);        //Jump to Beginning of Loop
    default:     setlbl(lblnam[lblcnt]);                 //Emit End of Loop Label
  }
  if (lbtype != LTCASE) inblck = lblblk[lblcnt-1];
  return lbtype;
}

/* Get Top Label and Return Type */
int toplbl(char *rtlbl) {
  if (lblcnt) {
    strcpy(rtlbl, lblnam[lblcnt-1]);
    DEBUG("Found top label %s\n", rtlbl)
    return lbltyp[lblcnt-1];
  }
  rtlbl[0] = 0; //Clear Label
  return LTNONE;
}

/* Push Label onto Stack        *
 * Args: lbltyp - Label type    *
 * Uses: curlbl - Label to push */
void pshlbl(int lbtype, char* lbname) {
  DEBUG("Pushing label type %d\n", lbtype)
  strcpy(lblnam[lblcnt], lbname);
  lbltyp[lblcnt] = lbtype;
  lblblk[lblcnt++] = FALSE;
  DEBUG("Pushed label '%s' onto stack\n", lbname)
}
