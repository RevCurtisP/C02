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
#include "label.h"
#include "expr.h"
#include "stmnt.h"

/* Process Assignment */
void prcasn(char trmntr)
{
  DEBUG("Processing assignment of variable '%s'\n", asnvar);
  expect('=');
  if (look('(')) {          //Parse Shortcut If 
    newlbl(cndlbl);         //Create Label for "if FALSE" expression
    prscnd(')', FALSE);     //Parse Condition
    expect('?');          
    prsxpr(':');            //Parse "if TRUE" expression
    newlbl(tmplbl);         //Create End of Expression Label
    asmlin("JMP", tmplbl);  //Jump over "if FALSE" expression
    setlbl(cndlbl);         //Emit "if FALSE" label
    prsxpr(trmntr);         //Parse "if FALSE" expression
    setlbl(tmplbl);         //Emit End of Expression Label
  }
  else
    prsxpr(trmntr);
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
void prspop(char trmntr) {
  oper = getnxt();
  CCMNT(oper); CCMNT(oper);
  DEBUG("Checking for post operation '%c'\n", oper);
  if (nxtchr == oper) {
    skpchr();
    prcpop();  //Process Post-Op
    expect(trmntr);
  }
  else
    expctd("post operator"); 
}

/* Process Variable at Beginning of Statement */
void prcvar(char trmntr)
{
  chksym(word);
  strcpy(asnvar, word);  //sav variable to assign to
  if (valtyp == ARRAY) {
    prsidx();  //Parse Array Index
    strncpy(asnidx, value, VARLEN);
  }
  else
    asnidx[0] = 0;
  if (ispopr(nxtchr))
    prspop(trmntr);  //Parse Post Operator
  else
    prcasn(trmntr);
}

/* Begin Program Block */
void bgnblk(int blkflg)
{
  DEBUG("Begining program block\n", 0);
  if (blkflg) {
    expect('{');
    inblck = TRUE;
  }
  else
    inblck = look('{');
  DEBUG("Set inblck to %d\n", inblck);
  setblk(inblck);
}

/* Parse and Compile and Assignment */
void prsasn(char trmntr) 
{
  getwrd();               //Get Variable to be Assigned
  ACMNT(word);
  prcvar(trmntr);
}

/* parse and compile 'break'/'continue' statement */
void pbrcnt(int lbtype) {
  DEBUG("Parsing BREAK/CONTINUE statement\n", 0);
  if (lstlbl(lbtype) < 0)
    ERROR("Break/continue statement outside of loop\n", 0, EXIT_FAILURE);
  DEBUG("Found Label '%s'\n", tmplbl);
  asmlin("JMP", tmplbl);
  expect(';'); 
}

/* parse and compile 'do' statement */
void pdo() {
  DEBUG("Parsing DO statement '%c'\n", nxtchr);
  newlbl(loplbl);          //Create Do Loop Label
  pshlbl(LTDO, loplbl);    //Push onto Stack
  setlbl(loplbl);          //Set Label to Emit on Next Line
  bgnblk(FALSE);           //Check For and Begin Block
}

/* parse and compile 'while' after 'do' statement */
void pdowhl() {
  DEBUG("Parsing WHILE after DO '%c'\n", nxtchr);
  getwrd();                //Check for While
  ACMNT(word);
  if (!wordis("while"))
     expctd("while statement");
  expect('(');
  newlbl(cndlbl);           //Create Skip Label
  prscnd(')', FALSE);      //Parse Conditional Expession
  asmlin("JMP", loplbl);   //Emit Jump to Beginning of Loop
  setlbl(cndlbl);          //and Set Label to Emit on Next Line
  expect(';');             //Check for End of Statement
}


/* parse and compile for statement */
void pfor() {
  DEBUG("Parsing FOR statement '%c'\n", nxtchr);
  expect('(');
  prsasn(';');            //Process Initial Assignment     
  newlbl(forlbl);         //Create For Loop Conditional Label
  setlbl(forlbl);         //and Set to Emit on Next Line
  newlbl(endlbl);         //Create End Label
  pshlbl(LTEND, endlbl);  //and Push onto Stack
  newlbl(loplbl);         //Create Loop Label
  pshlbl(LTLOOP, loplbl); //and Push onto Stack
  newlbl(cndlbl);         //Create Conditional Label
  prscnd(';', TRUE);      //Parse Conditional Expession
  asmlin("JMP", endlbl);  //Jump over Increment
  setlbl(loplbl);         //Set to Emit on Next Line
  prsasn(')');            //Parse Increment Assignment
  asmlin("JMP", forlbl);  //Jump to Conditional
  setlbl(cndlbl);         //Emit Label at Start of Loop  
  bgnblk(FALSE);          //Check For and Begin Block
}

/* parse and compile if statement */
void pif() {
  DEBUG("Parsing IF statement '%c'\n", nxtchr);
  expect('(');
  newlbl(cndlbl);      //Create New Label
  pshlbl(LTIF,cndlbl); //Push Onto Stack
  prscnd(')', FALSE);  //Parse Conditional Expession
  bgnblk(FALSE);       //Check For and Begin Block
}

/* parse and compile else statement */
void pelse() {
  DEBUG("Parsing ELSE statement '%c'\n", nxtchr);
  strcpy(lbltmp, lblasm);   //Save Line Label
  lblasm[0] = 0;            //and Clear It
  newlbl(skplbl);           //Create Skip Label
  pshlbl(LTIF, skplbl);     //Push Onto Stack
  asmlin("JMP", skplbl);    //Emit Jump over Block Code
  strcpy(lblasm, lbltmp);   //Restore Line Label
  bgnblk(FALSE);            //Check For and Begin Block
}

/* parse and compile if statement */
void pgoto() {
  DEBUG("Parsing GOTO statement\n", 0);
  getwrd();
  ACMNT(word);
  asmlin("JMP", word);
  expect(';');  
}

/* parse and compile return statement */
void pretrn() {
  DEBUG("Parsing RETURN statement\n", 0);
  if (!look(';'))
    prsxpr(';');
  asmlin("RTS", "");
  lsrtrn = TRUE;  //Set RETURN flag
}

/* parse and compile switch statement */
void pswtch() {
  DEBUG("Parsing SWITCH statement\n", 0);
  expect('(');
  prsxpr(')');
  newlbl(endlbl);                
  pshlbl(LTEND, endlbl);           
  bgnblk(TRUE);
  strcpy(xstmnt,"case");
}

/* parse and compile case statement */
void pcase() {
  DEBUG("Parsing CASE statement\n", 0);
  prscon(0xff);         //Parse Constant
  asmlin("CMP", value); 
  newlbl(skplbl);
  pshlbl(LTCASE, skplbl);
  asmlin("BNE", skplbl);
  expect(':');
}

void pdeflt() {
  DEBUG("Parsing DEFAULT statement\n", 0);
  expect(':');
  if (poplbl() != LTCASE) 
    ERROR("Encountered default without case\n", 0, EXIT_FAILURE);
  
}


/* parse and compile while statement */
void pwhile() {
  DEBUG("Parsing WHILE statement '%c'\n", nxtchr);
  expect('(');
  newlbl(endlbl);          //Create End Label
  pshlbl(LTEND, endlbl);   //and Push onto Stack
  newlbl(loplbl);          //create Loop Label
  setlbl(loplbl);          //Set to Emit on Next Line
  pshlbl(LTLOOP, loplbl);  //Push onto Stack
  newlbl(cndlbl);          //Create Conditional Skip Label
  prscnd(')', TRUE);       //Parse Conditional Expession
  asmlin("JMP", endlbl);   //Emit Jump to End of Loop
  setlbl(cndlbl);          //and Set Label to Emit on Next Line
  bgnblk(FALSE);           //Check For and Begin Block
}

/* generate unimplemented statement error */
void punimp() {
  ERROR("Unimplemented statement '%s' encountered\n", word, EXIT_FAILURE);  
}


/* Parse Function Call as Statement */
void prsfns()
{
  strcpy(term, word);  //Copy Function Name
  prsfnc();            //Parse Function Call
  expect(';');
  return;
}

/* parse and compile identifier (variable or function call) */
void prssym()
{
  DEBUG("Parsing Identifier %s\n", word);
  valtyp = gettyp();
  if (valtyp == FUNCTION)
    prsfns();  //Parse Statement Function Call
  else
    prcvar(';'); //Parse Assignment
}

/* End Program Block                    *
 * Args: blkflg: End of Multiline Block */
void endblk(int blkflg)
{
  int lbtype;
  DEBUG("Ending program block with flag %d\n", blkflg);
  expect('}'); //Block End Character
  DEBUG("Found inblck set to %d\n", inblck);
  if (inblck != blkflg)
    ERROR("Encountered '}' without matching '{'\n", 0, EXIT_FAILURE);
  lbtype = poplbl();
  if (lbtype == LTCASE)
    ERROR("Ended switch without default\n", 0, EXIT_FAILURE);
  if (lbtype == LTDO)
    pdowhl(); //Parse While at End of Do Loop
}

/* parse and compile program statement */
void pstmnt()
{
  DEBUG("Parsing statement '%s'\n", word);
  if (wordis("do")) {
    pdo();
    return;
  }
  if (wordis("else")) {
    pelse();
    return;
  }
  if (wordis("for")) {
    pfor();
    return;  
  }
  if (wordis("if")) {
    pif();
    return;
  }
  if (wordis("switch")) {
    punimp();
    return;  
  }
  if (wordis("case")) {
    punimp();
    return;  
  }
  if (wordis("default")) {
    punimp();
    return;  
  }
  if (wordis("while")) {
    pwhile();
    return;  
  }
  if(match(':')) {
    prslbl();  //Parse Label
    return;
  }
  if (wordis("break"))
    pbrcnt(LTEND);
  else if (wordis("continue"))
    pbrcnt(LTLOOP);
  else if (wordis("goto"))
    pgoto();
  else if (wordis("return"))
    pretrn();
  else
    prssym();
  if (lblcnt && !inblck) {
    if (poplbl() == LTDO)
      pdowhl(); //Parse While at End of Do Loop
  }
}

