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
#include "label.h"
#include "vars.h"

/* Lookup variable name in variable table   *
 * Returns index into varnam array        *
 *         FALSE if variable was not found  */
int fndvar(char *name) 
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
  if (fndvar(name) > -1)
    return TRUE;
  else
    return FALSE;
}  

/* Check for variable                       *
 * Generates error if variable is undefined *
 * Args: alwreg - allow register name       *
 *       name - variable name               */
void chksym(int alwreg, char *name) 
{
  if (strlen(name) == 1 && strchr("AXY", name[0])) {
    if (alwreg) return;
    else ERROR("Illegal reference to register %s\n", name, EXIT_FAILURE);
  }
  if (!symdef(name))
    ERROR("Undeclared variable '%s' encountered\n", name, EXIT_FAILURE);
}


/* Parse Variable Name                         *
 * Parameters: alwary - Allow Array Reference *
 * Sets: vrname - operand for LDA/STA/LDY/STY */
void reqvar(int alwary)
{
  prsvar(FALSE);
  if (!alwary)
    if (valtyp != VARIABLE) 
      expctd("Variable");
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
      sprintf(value, "%d", prsnum(0xFF) + 1);
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
  prscon();
}

/* Parse Data Array */
void prsdta()
{
  dtype = DTARRY;
  expect('{');
  dlen = 0;
  while (TRUE) {
    prscon();
    dattmp[dlen++] = cnstnt;
    if (!look(',')) 
      break;  
  }
  expect('}');
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
  else if (dtype == DTARRY) {
    DEBUG("Setting variable data to array of length %d\n", dlen);
    for (i=0; i<dlen; i++) 
      datvar[dsize++] = dattmp[i];   
  }
  else {
    DEBUG("Setting variable data to '%s'\n", value);
    dlen = strlen(value);
    for (i=0; i<dlen; i++) 
      datvar[dsize++] = value[i];   
  } 
  datlen[varcnt] = dlen;
  dattyp[varcnt] = dtype;
  DEBUG("Total data alllocated: %d bytes\n", dsize);  
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
  if (iscpre())
    prsdtc();   //Parse Data Constant
  else if (match('"'))
    prsdts();       //Parse Data String
  else if (match('{'))
    prsdta();       //Parse Data Array
  else
    expctd("numeric or string constant");
  setdat();   //Store Data Value
}

/* Add Variable to Variable table *
 * Uses: word - variable name     *
 *       value - variable size    */
void setvar(int m, int t) 
{
  DEBUG("Added variable '%s' ", word);
  strncpy(varnam[varcnt], vrname, VARLEN);
  varmod[varcnt] = m;
  vartyp[varcnt] = t;
  strncpy(varsiz[varcnt], value, 3);
  DETAIL("at index %d\n", varcnt);
}

/* Parse and Compile Variable Declaration *
 * Uses: word - variable name     */
void addvar(int m, int t) 
{
  strcpy(vrname, word); //Save Variable Name
  if (symdef(vrname))
    ERROR("Duplicate declaration of variable '%s\n", word,EXIT_FAILURE);
  if (t == VTVOID)
    ERROR("Illegal Variable Type\n", 0, EXIT_FAILURE);
  if (m == MTZP) {
    setlbl(vrname);
    sprintf(word, "$%hhX", zpaddr++);
    asmlin(EQUOP, word);
    strcpy(value, "*"); //Set Variable Type to Zero Page  
  }
  else
    pvarsz();   //Check for Array Declaration and Get Size
    setvar(m, t);  //Add to Variable Table  
  if (m != MTZP)
    prsdat();   //Parse Variable Data
  varcnt++;   //Increment Variable Counter
}

/* Add Function Definition */
void addfnc()
{
  ACMNT(word);
  expect('(');
  strcpy(fncnam, word);   //Save Function Name
  prmcnt = 0;             //Set Number of Parameters
  skpspc();               //Skip Spaces
  if (isalph()) {         //Parse Parameters
    reqvar(FALSE);        //Get First Parameter
    strcpy(prmtra, value);
    prmcnt++;     
    if (look(',')) {
      reqvar(FALSE);      //Get Second Parameter
      strcpy(prmtry, value);
      prmcnt++;     
      if (look(',')) {
        reqvar(FALSE);    //Third Parameter
        strcpy(prmtrx, value);
        prmcnt++;     
      }
    }
  }
  expect(')');
  if (look(';'))          //Forward Definition
    return;
  setlbl(fncnam);         //Set Function Entry Point
  if (prmcnt > 0)       
    asmlin("STA", prmtra); //Store First Parameter
  if (prmcnt > 1)
    asmlin("STY", prmtry); //Store Second Parameter
  if (prmcnt > 2)
    asmlin("STX", prmtrx); //Store Third Parameter
  endlbl[0] = 0;          //Create Dummy End Label
  pshlbl(LTFUNC, endlbl); //and Push onto Stack
  bgnblk(TRUE);           //Start Program Block
}

/* (Check For and) Parse Variable Declaration*/
void pdecl(int m, int t)
{
  DEBUG("Processing variable declarations(s) of type %d\n", t);
  while(TRUE) {
    getwrd();
    if (match('(')) {
      if (m != MTNONE) {
        ERROR("Illegal Modifier %d in Function Definion", m, EXIT_FAILURE);
      }
      addfnc();  //Add Function Call
      return;
    }  
    addvar(m, t);
    if (!look(','))
      break;
  }    
  expect(';');
  DEBUG("Variable Declaration Completed\n", 0);
  SCMNT("");  //Clear Assembler Comment
}

/* Check for and Parse Type Keyword */
int ptype(int m) 
{
  int result = TRUE;
  if (wordis("VOID"))
    pdecl(m, VTVOID);   //Parse 'void' declaration
  else if (wordis("CHAR"))
    pdecl(m, VTCHAR);   //Parse 'char' declaration
  else
    result = FALSE;
  //DEBUG("Returning %d from function ptype\n", result)'
  return result;
}

/* Check for and Parse Modifier */
int pmodfr() 
{
  DEBUG("Parsing modifier '%s'\n", word);
  int result = TRUE;
  if (wordis("ALIGNED")) {
    getwrd();
    ptype(MTALGN);  
  }
  else if (wordis("ZEROPAGE")) {
    getwrd();
    ptype(MTZP);  
  }
  else
    result = FALSE;
  return result;
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
    if (varmod[i] == MTALGN) {
      DEBUG("Alligning variable '%s'\n", varnam[i]);
      asmlin(ALNOP, "256");
    }
    if (datlen[i])
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
  vrwrtn = TRUE;
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


