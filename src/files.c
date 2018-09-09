/******************************
 * C02 File Handling Routines *
 ******************************/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>

#include "common.h"
#include "files.h"

/* Error - Print textual description of system error *
 *         and exit with system error code           */
void extsys(char *s) {
  perror(s);
  exterr(errno);
}

/* Open Source File                       *
 * Uses: srcnam - Source File Name      *
 * Sets: srcfil - Source File Handle    */
void opnsrc(void) {
  DEBUG("Processing Source File Name '%s'\n", srcnam)
  if (strrchr(srcnam, '.') == NULL) strcat(srcnam, ".c02"); //if no extension. add ".c02"
  DEBUG("opening Source File '%s'\n", srcnam)
  srcfil = fopen(srcnam, "r");      //open file
  if (srcfil == NULL) extsys(srcnam);
}

/* Close Source File */
void clssrc(void) { fclose(srcfil); }

/* Open Output File                    *
 * Uses: outnam - Output File Name  *
 * Sets: outfil - Output File Handle    */
void opnout(void) {
  DEBUG("Processing Output File Name '%s'\n", outnam)
  if (strlen(outnam) == 0)  //if Output File not specified
  {
    strcpy(outnam, srcnam);         //copy Source Name to Ouput Name
    char *dot = strrchr(outnam, '.'); //find extension
    if (dot != NULL) *dot = 0;          //and remove it
    DEBUG("Set Output File Name to '%s'\n", outnam)
  } 
  if (strrchr(outnam, '.') == NULL) strcat(outnam, ".asm"); //if no extension, add ".asm"
  DEBUG("Opening Output File '%s'\n", outnam)
  outfil = fopen(outnam, "w");      //open file
  if (outfil == NULL) extsys(outnam);
}

/* Close Output File */
void clsout(void) { 
  fprintf(outfil, "\n");
  fclose(outfil); 
}

/* Open Log File                     *
 * Uses: srcnam - Source File Name   *
 * Sets: logfil - Log File Handle    */
void opnlog(void) {
  strcpy(lognam, srcnam); //set Log File Name to Source File Name
  char *dot = strrchr(lognam, '.'); //find file extension
  if (dot != NULL) *dot = 0;        //and remove it
  strcat(lognam, ".log");           //add extension ".log"
  DEBUG("Opening Log File '%s'\n", lognam)
  logfil = fopen(lognam, "w");
  if (logfil == NULL)  extsys(lognam);
}

/* Close Log File                 *
 * Uses: logfil - Log File Handle */
void clslog(void) { fclose(logfil); }

/* Open Include file                               *
 * Uses: incnam - Include File Name                *
 *       subnam - Include File Name (Subdirectory) *
 * Sets: incfil - Include File Handle              */
void opninc(void)
{
  if (subnam[0]) {
    DEBUG("Attempting to open include file '%s'\n", subnam)
    incfil = fopen(subnam, "r");
    if (incfil == NULL) DEBUG("Open failed\n", 0)
    else {
      strcpy(incnam, subnam);
      DEBUG("INCNAM set to '%s'\n", incnam);
	  subnam[0] = 0;
      return;
    }
  }

  DEBUG("Opening include file '%s'\n", incnam)
  incfil = fopen(incnam, "r");
  if (incfil == NULL) extsys(incnam);
}

/* Close Include File                      *
 * Uses: incfil - Include File Handle    */
void clsinc(void) { fclose(incfil); }
