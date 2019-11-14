/*************************************
 * C02 Include File Parsing Routines *
 *************************************/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>

#include "common.h"
#include "files.h"
#include "asm.h"
#include "parse.h"
#include "label.h"
#include "vars.h"
#include "stmnt.h"
#include "dclrtn.h"
#include "include.h"

/* Read next include file name from Source File  *
 * Sets: incnam - the include file name         */
void pincnm(void) {
  char dlmtr;
  int inclen = 0;
  int sublen[SUBMAX];
  skpspc();
  dlmtr = getnxt();
  if (dlmtr == '<') {
    strcpy(incnam, incdir);
    inclen = strlen(incnam);
    if (subcnt) {
      for (subidx = 0; subidx < subcnt; subidx++) {
	    sublen[subidx] = 0;
        strcpy(subnam[subidx], incdir);
        strcat(subnam[subidx], subdir[subidx]);
        sublen[subidx] = strlen(subnam[subidx]);
        subnam[subidx][sublen[subidx]++] = '/';
      }
	}
    dlmtr = '>';
  }
  else if (dlmtr != '"')
    ERROR("Unexpected character '%c' after include\n", dlmtr, EXIT_FAILURE)
  while (!match(dlmtr))
  {
    incnam[inclen++] = nxtchr;
    for (int subidx = 0; subidx < subcnt; subidx++) {
      if (sublen[subidx]) subnam[subidx][sublen[subidx]++] = nxtchr;
	}
    skpchr();
  }
  skpchr(); //skip end dlmtr
  incnam[inclen] = 0;
  DEBUG("Set INCNAM to '%s'\n", incnam);
  for (int subidx = 0; subidx < subcnt; subidx++) {
    subnam[subidx][sublen[subidx]] = 0;
    DEBUG("Set SUBNAM[%d] ", subidx)
    DEBUG("to '%s'\n", subnam[subidx]);
  }
}

/* Process assembly language include file  */
void incasm(int chksub) {
  opninc(chksub);
  setcmt("======== Assembler File ");
  addcmt(incnam);
  addcmt(" =======");
  cmtlin();
  while (fgets(line, sizeof line, incfil) != NULL) {
    DEBUG("Writing line: %s", line)
    fputs(line, outfil);
  }
  setcmt("==========================================");
  cmtlin();
  clsinc();
}

/* Process define directive */
void pdefin(void) {
  DEBUG("Processing DEFINE directive\n", 0)
  getwrd();     //Get constant name
  strncpy(defnam, word, CONLEN);
  addcon(prsbyt()); //Get Value and Add Constant
}

/* Parse ASCII Subdirective */
void pascii(void) {
  getwrd(); //Get Subdirective Argument
  DEBUG("Parsing subdirective '%s'\n", word)
  if (wordis("INVERT"))
    invasc = TRUE;
  else if (wordis("HIGH"))
    mskasc = TRUE;
  else 
    ERROR("Unrecognized option '%s'\n", word, EXIT_FAILURE)
}

/* Parse Origin Subdirective */
void porign(void) {
  prsnum(0xFFFF); //Get Origin Address
  asmlin(ORGOP, value); //Emit Origin Instruction
  DEBUG("Set origin to %s\n", value)
}

/* Parse Padding Subdirective */
void ppddng(void) {
  padcnt = prsnum(0xFF); //Get Number of Padding Bytes
  DEBUG("Set padding to %d\n", padcnt)
}

/* Parse RamBase Subdirective */
void prambs(void) {
  rambas = prsnum(0xFFFF); //Set Ram Base Address to Literal
  DEBUG("Set ram base address to %d\n", rambas)
}

/* Parse WriteBase Subdirective */
void pwrtbs(void) {
  if (!rambas) ERROR("RAM Base must be set prior to Write Base\n", 0, EXIT_FAILURE);
  wrtbas = prsnum(0xFFFF); //Set Ram Base Address to Literal
  DEBUG("Set write base address to %d ", wrtbas)
  if (rambas && wrtbas) sprintf(wrtofs, "%+d", wrtbas - rambas);
  else wrtofs[0] = 0;
  DETAIL("and write offset to '%s'\n", wrtofs)
}

/* Parse Zeropage Subdirective */
void pzropg(void) {
  zpaddr = prsnum(0xFF); //Set Zero Page Address to Literal
  DEBUG("Set zero page address to %d\n", zpaddr)
}

/* Process Vartable Subdirective */
void pvrtbl(void) {
  if (vrwrtn) ERROR("Variable table already written", 0, EXIT_FAILURE)
  wvrtbl(); //Write Variable Table
}

/* Parse Pragma Directive */
void pprgma(void) {
  getwrd(); //Get Pragma Subdirective
  DEBUG("Parsing pragma directive '%s'\n", word)
  if (wordis("ASCII"))
    pascii(); //Parse Ascii
  else if (wordis("ORIGIN"))
    porign(); //Parse Origin
  else if (wordis("PADDING"))
    ppddng(); //Parse Padding
  else if (wordis("RAMBASE"))
    prambs(); //Parse RamBase
  else if (wordis("VARTABLE"))
    pvrtbl(); //Parse VarTable
  else if (wordis("WRITEBASE"))
    pwrtbs(); //Parse RamBase
  else if (wordis("ZEROPAGE"))
    pzropg(); //Parse ZeroPage
  else 
    ERROR("Illegal pragma subdirective '%s'\n", word, EXIT_FAILURE)
  cmtlin(); //Write Comment Line
}

/* Process Include File Directive */
void pincdr(void) {
  skpchr();            //skip '#'
  getwrd();            //read directive into word
  DEBUG("Processing include file directive '%s'\n", word)
  if (wordis("DEFINE")) 
    pdefin();
  else if (wordis("PRAGMA")) 
    pprgma();
  else 
	ERROR("Unrecognized directive '%s'\n", word, EXIT_FAILURE)
}

/* Parse Header Word */
void phdwrd(void) {
  getwrd();
  if (match(':')) prslab();
  else if (!ptype(MTNONE)) 
    ERROR("Unexpected word '%s' in header\n", word, EXIT_FAILURE)
}

/* Save Source File Information */
void savsrc(void) {
  savchr = nxtchr;
  savcol = curcol;
  savlin = curlin;
}

/* Set Include File Information */
void setinc(void) {
  curcol = 0;
  curlin = 0;
  inpfil = incfil;
  strcpy(inpnam, incnam);
  alcvar = FALSE;
}

/* Set Include File Name */
void setinm(char* filext) {
  strcpy(incnam, incdir);
  strcat(incnam, hdrnam);
  strcat(incnam, filext);
}

/* Set Input to Source File */
void setsrc(void) {
  inpfil = srcfil;
  strcpy(inpnam, srcnam);
}

/* Restore Source File Pointer*/
void rstsrc(void) {
  nxtchr = savchr;
  nxtupc = toupper(nxtchr);
  curcol = savcol;
  curlin = savlin;
  setsrc();
  alcvar = TRUE;
}

/* Process header include file  */
void inchdr(int chksub) {
  savsrc();  //Save Source File Information
  opninc(chksub);  //Open Include File
  setinc();  //Set Include File Information
  skpchr();  
  while (TRUE)  
  {
    skpspc();
    if (match(EOF)) break;
    DEBUG("Checking next character '%c'\n", nxtchr)
    if (match('#'))
      pincdr();
    else if (match('/')) 
      skpcmt(TRUE);
    else if (isalph()) 
      phdwrd();
    else {
      ERROR("Unexpected character '%c'\n", nxtchr, EXIT_FAILURE)
	}
  }    
  clsinc();
  rstsrc();
  nxtchr = savchr;
}

/* Process Header File specified on Command Line */
void phdrfl(void) {
  if (hdrnam[0] == 0) return;
  DEBUG("Processing Header '%s'\n", hdrnam)
  setinm(".h02");
  inchdr(TRUE);
  setinm(".a02");
  incasm(FALSE);
}

/* Process include file                    */
void pincfl(void) {
  pincnm(); //Parse Include File Name
  DEBUG("Processing include file '%s'\n", incnam)
  char *dot = strrchr(incnam, '.'); //find extension
  if (dot == NULL) {
    ERROR("Invalid include file name '%sn", incnam, EXIT_FAILURE)
  }
  if (strcmp(dot, ".a02") == 0) 
    incasm(TRUE);
  if (strcmp(dot, ".asm") == 0) 
    incasm(TRUE);
  else if (strcmp(dot, ".h02") == 0) { 
    inchdr(TRUE);  //Process Header File
    dot = strrchr(incnam, '.'); //find extension
    strcpy(dot, ".a02");
    DEBUG("INCNAM set to '%s'\n", incnam)
    incasm(FALSE);  //Process Assembly File with Same Name
  }
  else {
    ERROR("Unrecognized include file extension '%s'\n'", dot, EXIT_FAILURE)
  }   
}

/* Print Constant Table to Log File */
void logcon(void) {
  int i;
  fprintf(logfil, "\n%-10s %5s\n", "Constant", "Value");
  for (i=0; i<concnt; i++) {
    fprintf(logfil, "%-10s %5d\n", connam[i], conval[i]);
  }
}
