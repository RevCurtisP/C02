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
  skpspc();
  dlmtr = getnxt();
  if (dlmtr == '<') {
    strcpy(incnam, incdir);
    inclen = strlen(incnam);
    dlmtr = '>';
  }
  else if (dlmtr != '"')
    ERROR("Unexpected character '%c' after include\n", dlmtr, EXIT_FAILURE)
  while (!match(dlmtr))
  {
    incnam[inclen++] = nxtchr;
    skpchr();
  }
  skpchr(); //skip end dlmtr
  incnam[inclen] = 0;
}

/* Process assembly language include file  */
void incasm(void) {
  opninc();
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
  ERROR("Define directive not implemented\n", 0, EXIT_FAILURE)
}

/* Parse ASCII Subdirective */
void pascii(void) {
  getwrd(); //Get Pragma Subdirective
  if (wordis("INVERT"))
    invasc = TRUE;
  if (wordis("HIGH"))
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

/* Parse Zeropage Subdirective */
void prszpg(void) {
  zpaddr = prsnum(0xFF); //Set Zero Page Address to Literal
  DEBUG("Set zero page address to %d\n", zpaddr)
}

/* Process Vartable Subdirective */
void pvrtbl(void) {
  if (vrwrtn) 
    ERROR("Variable table already written", 0, EXIT_FAILURE)
  
  vartbl(); //Write Variable Table
}

/* Parse Pragma Directive */
void pprgma(void) {
  getwrd(); //Get Pragma Subdirective
  DEBUG("Parsing pragma directive '%s'\n", word)
  if (wordis("ASCII"))
    pascii(); //Parse Ascii
  else if (wordis("ORIGIN"))
    porign(); //Parse Origin
  else if (wordis("VARTABLE"))
    pvrtbl(); //Parse Vartable
  else if (wordis("ZEROPAGE"))
    prszpg(); //Parse Origin
  else 
    ERROR("Illegal pragma subdirective '%s'\n", word, EXIT_FAILURE)
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
  if (!ptype(MTNONE)) 
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
void inchdr(void) {
  savsrc();  //Save Source File Information
  opninc();  //Open Include File
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
      skpcmt();
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
  inchdr();
  setinm(".a02");
  incasm();
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
    incasm();
  if (strcmp(dot, ".asm") == 0) 
    incasm();
  else if (strcmp(dot, ".h02") == 0) { 
    inchdr();  //Process Header File
    strcpy(dot, ".a02");
    incasm();  //Process Assembly File with Same Name
  }
  else {
    ERROR("Unrecognized include file extension '%s'\n'", dot, EXIT_FAILURE)
  }   
}

/* Print Constant Table to Log File */
void logcon(void) {
  int i;
  fprintf(logfil, "\n%-31s %5s\n", "Definition", "Value");
  for (i=0; i<concnt; i++)
  {
    fprintf(logfil, "%-31s %5d\n", connam[i], conval[i]);
  }
}
