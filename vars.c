/*************************************
 * C02 Variable Management Routines  *
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
#include "vars.h"

/* Lookup variable name in variable table   *
 * Returns index into varnam array        *
 *         FALSE if variable was not found  */
int lookup(char *name) 
{
  int i;
  DEBUG("Looking up variable '%s'\n", word);
  for (i=0; i<varcnt; i++) {
    if (strcmp(varnam[i], name) == 0)
      return i;
  }
  return -1;
}

/* Check if variable has been defined */
int symdef(char *name) 
{
  if (lookup(name) < 0)
    return FALSE;
  else
    return TRUE;
}  

/* Check for variable                       *
 * Generates error if variable is undefined */
void chksym(char *name) 
{
  if (!symdef(name))
    ERROR("Undeclared variable '%s' encountered\n", word, EXIT_FAILURE);
}

/* Check for Array specifier and get size *
 * Sets: value - array size (as string)   *
 *               "" if not an array       */
void pvarsz() 
{
  DEBUG("Checking for array definition\n", 0);
  value[0] = 0;
  if (match('[')) {
    skpchr();
    if (alcvar) {
      DEBUG("Parsing array size\n", 0);
      prsnum(0xFF);
    }
    expect(']');
  }
  if (!alcvar)
    strcpy(value, "*");  
}

/* Parse Data Constant */
void prsdtc()
{
  dtype = DTBYTE;
  prscon(0xff);
}

/* Parse Data String */
void prsdts()
{
  dtype = DTSTR;
  getstr();
  strcpy(value, word);
  DEBUG("Parsed Data String '%s'\n", value);
}

/* Store variable data              *
 * Uses: value    - Data to store   *
 * Sets: datvar[] - Variable Data   *
 *       datlen[] - Data Length     */
void setdat()
{
  int i;
  if (dtype == DTBYTE) {
    DEBUG("Setting variable data to '%d'\n", cnstnt);
    dlen = 1;
    datvar[dsize++] = cnstnt;
  }
  else {
    DEBUG("Setting variable data to '%s'\n", value);
    dlen = strlen(value);
    for (i=0; i<dlen; i++) 
      datvar[dsize++] = value[i];   
  } 
  datlen[varcnt] = dlen;
  dattyp[varcnt] = dtype;  
}

/* Parse and store variable data */
void prsdat()
{
  DEBUG("Checking for variable data\n", 0);
  if (!look('=')) {
    datlen[varcnt] = 0;
    return;
  }
  skpspc();
  if (isnpre())
    prsdtc(0xff);   //Parse Data Constant
  else if (match('"'))
    prsdts();       //Parse Data String
  else
    expctd("numeric or string constant");
  setdat();   //Store Data Value
}

/* Add Variable to Variable table *
 * Uses: word - variable name     *
 *       value - variable size    */
void setvar(int t) 
{
  DEBUG("Adding variable '%s'\n", word);
  strncpy(varnam[varcnt], word, VARLEN);
  vartyp[varcnt] = t;
  strncpy(varsiz[varcnt], value, 3);
  DEBUG("Added at index %d\n", varcnt);
}

/* Parse and Compile Variable Declaration *
 * Uses: word - variable name     */
void addvar(int t) 
{
  if (symdef(word))
    ERROR("Duplicate declaration of variable '%s\n", word,EXIT_FAILURE);
  if (t == VTVOID)
    ERROR("Illegal Variable Type\n", 0, EXIT_FAILURE);
  pvarsz();   //Check for Array Declaration and Get Size
  setvar(t);  //Add to Variable Table  
  prsdat();   //Parse Variable Data
  varcnt++;   //Increment Variable Counter
}

void addfnc()
{
  expect(')');
  if (alcvar)
    setlbl(word); 
}

/* (Check For and) Parse Variable Declaration*/
void pdecl(int t)
{
  DEBUG("Processing variable declarations(s) of type %d\n", t);
  while(TRUE) {
    getwrd();
    if (look('(')) {
      addfnc();
      break;
    }  
    addvar(t);
    if (!look(','))
      break;
  }    
  expect(';');
}

/* Write Variable Data */
void vardat(int i)
{
  int j;
  DEBUG("Building Data for Variable '%s'\n", varnam[i]);
  value[0] = 0;
  for (j=0; j<datlen[i]; j++) {
    if (j) strcat(value,",");
    sprintf(word, "$%hhX", datvar[dlen++]);
    strcat(value, word);  
  }
  if (dattyp[i] == DTSTR) strcat(value, ",$00");
  DEBUG("Allocating Data for Variable '%s'\n", varnam[i]);
  asmlin(BYTEOP, value);
}

/* Write Variable Table */
void vartbl()
{
  int i;
  DEBUG("Writing Variable Table", 0);
  dlen = 0;
  for (i=0; i<varcnt; i++) {
    strcpy(lblasm, varnam[i]);
    DEBUG("Set Label to '%s'\n", lblasm);
    if (strcmp(varsiz[i], "*") == 0)
      continue;
    else if (datlen[i])
      vardat(i);  //Write Variable Data
    else if (strlen(varsiz[i]) > 0) {
      DEBUG("Allocating array '%s'\n", varnam[i]);
      asmlin(STROP, varsiz[i]);
    }
    else {
      DEBUG("Allocating variable '%s'\n", varnam[i]);
      asmlin(BYTEOP, "0");
    }
  }
}

/* Print Variable Table to Log File */
void logvar()
{
  int i;
  fprintf(logfil, "\n%-31s %s %s %s\n", "Variable", "Type", "Size", "Data");
  for (i=0; i<varcnt; i++)
  {
    fprintf(logfil, "%-31s %4d %4s %1d-%d\n", varnam[i], vartyp[i], varsiz[i], dattyp[i], datlen[i]);
  }
}


