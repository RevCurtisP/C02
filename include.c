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
void pincnm()
{
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
    ERROR("Unexpected character '%c' after include\n", dlmtr, EXIT_FAILURE);
  while (!match(dlmtr))
  {
    incnam[inclen++] = nxtchr;
    skpchr();
  }
  skpchr(); //skip end dlmtr
  incnam[inclen] = 0;
}

/* Process assembly language include file  */
void incasm() {
  opninc();
  setcmt("======== Assembler File ");
  addcmt(incnam);
  addcmt(" =======");
  cmtlin();
  while (fgets(line, sizeof line, incfil) != NULL) {
    DEBUG("Writing line: %s", line);
    fputs(line, outfil);
  }
  setcmt("==========================================");
  cmtlin();
  clsinc();
}

/* Process define directive */
void pdefin() 
{
  getwrd(); //get defined identifier
  DEBUG("Defining '%s'\n", word);
  strncpy(defnam[defcnt], word, VARLEN);
  setlbl(word); //Set label Assembler Line
  expect('=');
  defval[defcnt++] = prsbyt(); //Get Value
  asmlin(EQUOP, value); //Write Definition
  DEBUG("Defined as '%s'\n", value);
}

/* Parse ASCII Subdirective */
void pascii()
{
  getwrd(); //Get Pragma Subdirective
  if (wordis("INVERT"))
    invasc = TRUE;
  if (wordis("HIGH"))
    mskasc = TRUE;
  else {
    fprintf(stderr, "Unrecognized option '%s'\n", word);
    exterr(EXIT_FAILURE);
  }
}

/* Parse Origin Subdirective */
void porign()
{
  prsnum(0xFFFF); //Get Origin Address
  asmlin(ORGOP, value); //Emit Origin Instruction
  DEBUG("Set origin to %s\n", value);
}

/* Parse Zeropage Subdirective */
void prszpg()
{
  zpaddr = prsnum(0xFF); //Set Zero Page Address to Constant
  DEBUG("Set zero page address to %d\n", zpaddr);
}

/* Process Vartable Subdirective */
void pvrtbl()
{
  if (vrwrtn) {
    ERROR("Variable table already written", 0, EXIT_FAILURE);
  }
  vartbl(); //Write Variable Table
}

/* Parse Pragma Directive */
void pprgma()
{
  getwrd(); //Get Pragma Subdirective
  DEBUG("Parsing pragma directive '%s'\n", word);
  if (wordis("ASCII"))
    pascii(); //Parse Ascii
  else if (wordis("ORIGIN"))
    porign(); //Parse Origin
  else if (wordis("VARTABLE"))
    pvrtbl(); //Parse Vartable
  else if (wordis("ZEROPAGE"))
    prszpg(); //Parse Origin
  else 
    ERROR("Illegal pragma subdirective '%s'\n", word, EXIT_FAILURE);
}

/* Process Include File Directive */
void pincdr()
{
  skpchr();            //skip '#'
  getwrd();            //read directive into word
  DEBUG("Processing include file directive '%s'\n", word);
  if (wordis("DEFINE")) 
    pdefin();
  if (wordis("PRAGMA")) 
    pprgma();
  else {
    fprintf(stderr, "Unrecognized directive '%s'\n", word);
    exterr(EXIT_FAILURE);
  }
}

/* Parse Header Word */
void phdwrd() {
  getwrd();
  if (!ptype(MTNONE)) {
    fprintf(stderr, "Unexpected word '%s' in header\n", word);
    exterr(EXIT_FAILURE); 
  }
}

/* Save Source File Information */
void savsrc() {
  savchr = nxtchr;
  savcol = curcol;
  savlin = curlin;
}

/* Set Include File Information */
void setinc() {
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

/* Set Input to Souyrce File */
void setsrc() {
  inpfil = srcfil;
  strcpy(inpnam, srcnam);
}

/* Restore Source File Pointer*/
void rstsrc() {
  nxtchr = savchr;
  nxtupc = toupper(nxtchr);
  curcol = savcol;
  curlin = savlin;
  setsrc();
  alcvar = TRUE;
}

/* Process header include file  */
void inchdr() {
  savsrc();  //Save Source File Information
  opninc();  //Open Include File
  setinc();  //Set Include File Information
  skpchr();  
  while (TRUE)  
  {
    skpspc();
    if (match(EOF)) break;
    DEBUG("Checking next character '%c'\n", nxtchr);
    if (match('#'))
      pincdr();
    else if (match('/')) 
      skpcmt();
    else if (isalph()) 
      phdwrd();
    else {
      fprintf(stderr, "Unexpected character '%c'\n", nxtchr);	 
	  exterr(EXIT_FAILURE);
	}
  }    
  clsinc();
  rstsrc();
  nxtchr = savchr;
}

/* Process Header File specified on Command Line */
void phdrfl()
{
  if (hdrnam[0] == 0) return;
  DEBUG("Processing Header '%s'\n", hdrnam);
  setinm(".h02");
  inchdr();
  setinm(".a02");
  incasm();
}

/* Process include file                    */
void pincfl() 
{
  pincnm(); //Parse Include File Name
  DEBUG("Processing include file '%s'\n", incnam);
  char *dot = strrchr(incnam, '.'); //find extension
  if (dot == NULL) {
    fprintf(stderr, "Invalid include file name '%sn", incnam);
    exterr(EXIT_FAILURE);  
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
    fprintf(stderr, "Unrecognized include file extension '%s'\n'", dot);
    exterr(EXIT_FAILURE);
  }   
}

/* Print Definition Table to Log File */
void logdef()
{
  int i;
  fprintf(logfil, "\n%-31s %5s\n", "Definition", "Value");
  for (i=0; i<defcnt; i++)
  {
    fprintf(logfil, "%-31s %5d\n", defnam[i], defval[i]);
  }
}
