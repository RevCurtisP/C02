/************************************
 * C02 Statement Compiling Routines *
 ************************************/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include "common.h"
#include "asm.h"
#include "parse.h"
#include "vars.h"
#include "expr.h"
#include "label.h"
#include "stmnt.h"

/* parse and compile if statement */
void pif() {
  DEBUG("Parsing IF statement '%c'\n", nxtchr);
  expect('(');
  newlbl(LTIF);
  prscnd(')');  //Parse Conditional Expession
}

/* parse and compile else statement */
void pelse() {
  DEBUG("Parsing ELSE statement '%c'\n", nxtchr);
  strcpy(lbltmp, lblasm);
  lblasm[0] = 0;
  newlbl(LTIF);
  asmlin("JMP", curlbl);
  strcpy(lblasm, lbltmp);
}

/* parse and compile for statement */
void pfor() {
  ERROR("Unsupported statement FOR encountered",0, EXIT_FAILURE);  
}

/* parse and compile if statement */
void pgoto() {
  DEBUG("Parsing GOTO statement\n", 0);
  getwrd();
  ACMNT(word);
  asmlin("JMP", word);
  expect(';');  
}

/* parse and compile if statement */
void pretrn() {
  DEBUG("Parsing RETURN statement\n", 0);
  if (!look(';'))
    prsxpr(';');
  asmlin("RTS", "");
}

/* parse and compile while statement */
void pwhile() {
  DEBUG("Parsing WHILE statement '%c'\n", nxtchr);
  expect('(');
  newlbl(LTLOOP);         //create Loop Label
  setlbl(curlbl);
  newlbl(LTEND);          //Create End Label
  prscnd(')');            //Parse Conditional Expession
}

/* generate unimplemented statement error */
void punimp() {
  ERROR("Unimplemented statement '%s' encountered", word, EXIT_FAILURE);  
}


/* Parse and compile assignment */
void prsasn()
{
  DEBUG("Parsing assignment of variable '%s''\n", asnvar);
  skpchr(); //skip equals sign
  ACMNT("=");
  prsxpr(';');
  if (strlen(asnidx)) {
    asmlin("LDX", asnidx);
    strcat(asnvar,",X");
  }
  asmlin("STA", asnvar);
}

void prcpop() {
  DEBUG("Processing post operation '%c'\n", oper);
  switch(oper)
  {
    case '+': 
      asmlin("INC", asnvar);
      break;
    case '-':
      asmlin("DEC", asnvar);
      break;
    case '<':
      asmlin("ASL", asnvar);
      break;
    case '>':
      asmlin("LSR", asnvar);
      break;
    default:
      printf("Unrecognized post operator '%c'\n", oper);
      exterr(EXIT_FAILURE);
  }
}

/* Parse Post Operator */
void prspop() {
  oper = getnxt();
  CCMNT(oper); CCMNT(oper);
  DEBUG("Checking for post operation '%c'\n", oper);
  if (nxtchr == oper) {
    skpchr();
    prcpop();  //Process Post-Op
    expect(';');
  }
  else
    expctd("post operator"); 
}

/* Parse function call in first expression  */
void prsfnc() 
{
  char fnname[255]; /*Function name*/ 
  DEBUG("Processing function call '%s'...\n", term);
  strcpy(fnname, word);
  skpchr(); //skip open paren
  CCMNT('(');
  skpspc();
  if (match(')'))    //if no arguments 
    skpchr();       //  skip close paren
  else {               //otherwise
    if (look('&')) {
      prsadr(')');
    }
    else
      prsxpr(')');  //parse expression
  }
  asmlin("JSR", fnname);
  expect(';');
}

/* parse and compile identifier (variable or function call) */
void prssym()
{
  DEBUG("Parsing Identifier %s\n", word);
  valtyp = get_vartyp();
  if (valtyp == FUNCTION) {
    prsfnc();  //Parse Function Call
    return;
  }
  chksym(word);
  strcpy(asnvar, word);  //sav variable to assign to
  if (valtyp == ARRAY) {
    prsidx();  //Parse Array Index
    strncpy(asnidx, value, VARLEN);
  }
  else
    asnidx[0] = 0;
  if (ispopr(nxtchr))
    prspop();  //Parse Post Operator
  else
  {
    skpspc();
    if (match('='))
      prsasn();      //Parse Assignment
    else
      expctd("=");
  }
}

/* parse and compile program statement */
void pstmnt()
{
  DEBUG("Parsing statement '%s'\n", word);
  if(match(':')) {
    prslbl();  //Parse Label
    return;
  }
  if (wordis("if")) {
    pif();
    return;
  }
  if (wordis("else")) {
    pelse();
    return;
  }
  if (wordis("while")) {
    pwhile();
    return;  
  }
  if (wordis("for")) {
    punimp();
    return;  
  }
  else if (wordis("goto"))
    pgoto();
  else if (wordis("return"))
    pretrn();
  else
    prssym();
  if (lblcnt) {
    lbllin();
  }
}

