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
void bgnblk()
{
  DEBUG("Begining program block\n", 0);
  inblck = look('{');  
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
  newlbl();                //Create Do Loop Label
  pshlbl(LTDO);            //Push onto Stack
  setlbl(curlbl);          //Set Label to Emit on Next Line
  bgnblk();                //Check For and Begin Block
}

/* parse and compile 'while' after 'do' statement */
void pdowhl() {
  DEBUG("Parsing WHILE after DO '%c'\n", nxtchr);
  getwrd();                //Check for While
  ACMNT(word);
  if (!wordis("while"))
     expctd("while statement");
  expect('(');
  newlbl();                //Create Skip Label
  prscnd(')', FALSE);      //Parse Conditional Expession
  asmlin("JMP", loplbl);   //Emit Jump to Beginning of Loop
  setlbl(curlbl);          //and Set Label to Emit on Next Line
  expect(';');             //Check for End of Statement
}


/* parse and compile for statement */
void pfor() {
  DEBUG("Parsing FOR statement '%c'\n", nxtchr);
  expect('(');
  prsasn(';');            //Process Initial Assignment     
  newlbl();               //Create Temporary Label
  setlbl(curlbl);         //Set to Emit on Next Line
  strcpy(tmplbl, curlbl); //and Save it
  newlbl();               //Create End Label
  pshlbl(LTEND);          //and Push onto Stack
  strcpy(endlbl, curlbl); //and Save it
  newlbl();               //Create Loop Label
  strcpy(loplbl, curlbl); //and Save it
  pshlbl(LTLOOP);         //and Push onto Stack
  newlbl();               //Create Skip Increment Label
  strcpy(skplbl, curlbl); //and Save it
  prscnd(';', TRUE);     //Parse Conditional Expession
  asmlin("JMP", endlbl);  //Jump over Increment
  setlbl(loplbl);         //Set to Emit on Next Line
  prsasn(')');            //Parse Increment Assignment
  asmlin("JMP", tmplbl);  //Jump to Conditional
  setlbl(skplbl);         //Emit Label at Start of Loop  
  bgnblk();               //Check For and Begin Block
}

/* parse and compile if statement */
void pif() {
  DEBUG("Parsing IF statement '%c'\n", nxtchr);
  expect('(');
  newlbl();            //Create New Label
  pshlbl(LTIF);        //Push Onto Stack
  prscnd(')', FALSE);  //Parse Conditional Expession
  bgnblk();            //Check For and Begin Block
}

/* parse and compile else statement */
void pelse() {
  DEBUG("Parsing ELSE statement '%c'\n", nxtchr);
  strcpy(lbltmp, lblasm);
  lblasm[0] = 0;
  newlbl();      //Create New Label
  pshlbl(LTIF);  //Push Onto Stack
  asmlin("JMP", curlbl);
  strcpy(lblasm, lbltmp);
  bgnblk();                //Check For and Begin Block
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
  newlbl();                //Create End Label
  pshlbl(LTEND);           //Push onto Stack
  strcpy(endlbl, curlbl);  //and Save it
  newlbl();                //create Loop Label
  setlbl(curlbl);          //Set to Emit on Next Line
  pshlbl(LTLOOP);          //Push onto Stack
  newlbl();                //Create Conditional Skip Label
  prscnd(')', TRUE);       //Parse Conditional Expession
  asmlin("JMP", endlbl);   //Emit Jump to End of Loop
  setlbl(curlbl);          //and Set Label to Emit on Next Line
  bgnblk();                //Check For and Begin Block
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
  DEBUG("Ending program block with flag %d\n", blkflg);
  skpchr();  //Skip '}';
  DEBUG("Found inblck set to %d\n", inblck);
  if (inblck != blkflg)
    ERROR("Encountered '}' without matching '{'\n",0,EXIT_FAILURE);
  if (poplbl() == LTDO)
    pdowhl(); //Parse While at End of Do Loop
}

/* parse and compile program statement */
void pstmnt()
{
  DEBUG("Parsing statement '%s'\n", word);
  if(match(':')) {
    prslbl();  //Parse Label
    return;
  }
  if (wordis("do")) {
    pdo();
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
    pfor();
    return;  
  }
  else if (wordis("break"))
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

