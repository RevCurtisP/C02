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

/* Parse Shortcut If */
void prssif(char trmntr) {
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


/* Process Assignment */
void prcasn(char trmntr)
{
  expect('=');
  DEBUG("Processing assignment of variable '%s'\n", asnvar);
  if (look('(')) 
    prssif(trmntr); //Parse Shortcut If 
  else
    prsxpr(trmntr); //Parse Expression
  if (strcmp(asnvar, "X")==0)
    asmlin("TAX", "");
  else if (strcmp(asnvar, "Y")==0)
    asmlin("TAY", "");
  else if ((strcmp(asnvar, "A")!=0))
  {
    if (strlen(asnidx)) { 
      if (asnivt == CONSTANT) {
        strcat(asnvar, "+");
        strcat(asnvar, asnidx);
      }
      else {
        asmlin("LDX", asnidx);
        strcat(asnvar,",X");
      }
    }
    asmlin("STA", asnvar);
  }
}

void poperr() 
{
  printf("Illegal post-operation %c%c on register %s\n", oper, oper, asnvar);
  exterr(EXIT_FAILURE);
}

/* Process Variable at Beginning of Statement */
void prcvar(char trmntr)
{
  chksym(TRUE, word);
  strcpy(asnvar, word);  //sav variable to assign to
  if (valtyp == VARIABLE && look(';')) {
    asmlin("STA", asnvar);
    return;
  }
  if (valtyp == ARRAY) {
    prsidx();  //Parse Array Index
    asnivt = valtyp;
    if (asnivt == CONSTANT) 
	    strncpy(asnidx, word, VARLEN);
	else
    	strncpy(asnidx, value, VARLEN);
  }
  else
    asnidx[0] = 0;
  if (ispopr()) {
    if (prspst(trmntr, asnvar, asnidx)) //Parse Post Operator
      expctd("post operator");
  }
  else
    prcasn(trmntr);
}

/* Begin Program Block */
void bgnblk(char blkchr)
{
  DEBUG("Begining program block\n", 0);
  if (blkchr) {
    expect(blkchr);
    inblck = TRUE;
  }
  else
    inblck = look('{');
  DEBUG("Set inblck to %d\n", inblck);
  setblk(inblck);
}

/* Parse 'asm' String Parameter */
void pasmst(char trmntr)
{
  skpspc(); //Skip Spaces
  if (!match('"')) 
    expctd("string");
  getstr();
  skpspc();
  expect(trmntr);
}

/* Parse and Compile 'asm' statement */
void pasm()
{
  char opcode[LINELEN];
  expect('(');
  pasmst(',');    
  if (strlen(word)) setlbl(word);
  pasmst(',');
  strcpy(opcode, word);
  pasmst(')');
  expect(';');
  asmlin(opcode, word);
}

/* Parse and Compile and Assignment */
void prsasn(char trmntr) 
{
  getwrd();               //Get Variable to be Assigned
  ACMNT(word);
  prcvar(trmntr);
}

/* parse and compile 'break'/'continue' statement */
void pbrcnt(int lbflag) 
{
  DEBUG("Parsing BREAK/CONTINUE statement\n", 0);
  if (lstlbl(lbflag) < 0)
    ERROR("Break/continue statement outside of loop\n", 0, EXIT_FAILURE);
  DEBUG("Found Label '%s'\n", tmplbl);
  asmlin("JMP", tmplbl);
  expect(';'); 
}

/* parse and compile 'do' statement */
void pdo() 
{
  DEBUG("Parsing DO statement '%c'\n", nxtchr);
  newlbl(endlbl);          //Create End Label
  pshlbl(LTDWHL, endlbl);   //and Push onto Stack
  reqlbl(loplbl);          //Get or Create/Set Loop Label
  //newlbl(loplbl);          //Create Do Loop Label
  //setlbl(loplbl);          //Set Label to Emit on Next Line
  pshlbl(LTDO, loplbl);    //Push onto Stack
  bgnblk(FALSE);           //Check For and Begin Block
}

/* parse and compile 'while' after 'do' statement */
void pdowhl() {
  DEBUG("Parsing WHILE after DO '%c'\n", nxtchr);
  getwrd();                //Check for While
  ACMNT(word);
  if (!wordis("WHILE"))
     expctd("while statement");
  expect('(');
  //poplbl();               //Pop While Conditional Label
  strcpy(cndlbl, loplbl);   //Set Conditional Label to Loop Label
  prscnd(')', TRUE);       //Parse Conditional Expession
  //asmlin("JMP", loplbl); //Emit Jump to Beginning of Loop
  //setlbl(cndlbl);         //and Set Label to Emit on Next Line
  poplbl();               //Pop While Conditional Label
  setlbl(endlbl);         //and Set Label to Emit on Next Line
  expect(';');              //Check for End of Statement
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
  DEBUG("Parsing IF statement\n", 0);
  expect('(');
  newlbl(cndlbl);      //Create New Label
  pshlbl(LTIF,cndlbl); //Push Onto Stack
  prscnd(')', FALSE);  //Parse Conditional Expession
  bgnblk(FALSE);       //Check For and Begin Block
}

/* parse and compile else statement */
void pelse() {
  DEBUG("Parsing ELSE statement\n", 0);
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
  expect(';');  
  asmlin("JMP", word);
}

/* parse and compile inline statement */
void pinlne()
{
  DEBUG("Parsing INLINE statement\n", 0);
  do { 
    DEBUG("Parsing inline parameter\n", 0);
    if (look('&')) {
      reqvar(FALSE); //Get Variable Name
      strcpy(word, "<");
      strcat(word, value);
      strcat(word, ", >");
      strcat(word, value);
      asmlin(BYTEOP, word);
    }
    else if (look('"')) {
      value[0] = 0;
      while (!match('"')) {
        CCMNT(nxtchr);
        sprintf(word, "$%hhX,", getnxt());
        strcat(value, word);
      }
      strcat(value,"0");
      CCMNT(nxtchr);
      skpchr(); //Skip Terminating Quote
      asmlin(BYTEOP, value);
    }
    else {
      prscon(0xFF);
      sprintf(word, "$%hhX", cnstnt); //not needed?
      asmlin(BYTEOP, value);
    }
  } while (look(','));
  expect(';');
}

/* parse and compile pop statement */
void ppop()
{
  DEBUG("Parsing POP statement\n", 0);
  do {  
    asmlin("PLA", "");     //Pop Value off Stack
    if (!look('*')) {
      reqvar(TRUE);
      strcpy(term, value);
      chkidx();
      asmlin("STA", term);
    } 
  } while (look(','));
  expect(';');  
}

/* parse and compile push statement */
void ppush()
{
  DEBUG("Parsing PUSH statement\n", 0);
  do { 
    if (!chkadr(1)) {
      prsxpr(0);        //Parse Expression
      asmlin("PHA",""); //Push Result on Stack
    }
  } while (look(','));
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

/* parse and compile select statement */
void pslct() {
  DEBUG("Parsing SELECT statement\n", 0);
  expect('(');
  prsxpr(')');            //Parse Expression
  newlbl(endlbl);         //Create New Label
  pshlbl(LTSLCT,endlbl);   //Push Onto Stack
  bgnblk('{');            //Require Beginning of Block
  strcpy(xstmnt, "CASE"); //Require Next Statement to be CASE
}

/* process end of case block */
void ecase() {
  DEBUG("Processing end of CASE block\n", 0);
  if (poplbl(cndlbl) != LTCASE)
    ERROR("%s not at end of CASE block\n", word, EXIT_FAILURE);  
  if (toplbl(endlbl) != LTSLCT)
    ERROR("Illegal nesting in SELECT statement\n", 0, EXIT_FAILURE);  
  asmlin("JMP", endlbl);  //Emit jump over default case
  setlbl(cndlbl);  //Set entry point label to emit
}

/* parse and compile select statement */
void pcase() {
  int fcase = (strcmp(xstmnt, "CASE") == 0);
  if (fcase)
    xstmnt[0] = 0;  //Clear xstmnt
  else 
	ecase("CASE");  //Process end of case block
  skplbl[0] = 0;           //Clear Skip Label
  newlbl(cndlbl);          //Create Conditional Label
  pshlbl(LTCASE, cndlbl);  //and Push onto Stack
  while(TRUE) {
    prstrm();              //Parse CASE argument
    if (!fcase || valtyp != CONSTANT || cnstnt)
      asmlin("CMP", term); //Emit Comparison
    if (look(',')) {
      chklbl(skplbl);      //Emit skip to beginning of CASE block
      asmlin("BEQ", skplbl);
      fcase = 0;
	  continue;            //Parse next argument
    }
    bgnblk(':');           //Emit branch to end of CASE block
    asmlin("BNE", cndlbl); 
    break;
  }
  if (skplbl[0])
    setlbl(skplbl); //Set CASE block label if defined
}

/* parse and compile default statement */
void pdflt() {
  expect(':');
  ecase(); //Process end of case block
}

/* parse and compile while statement */
void pwhile() {
  DEBUG("Parsing WHILE statement '%c'\n", nxtchr);
  expect('(');
  newlbl(endlbl);          //Create End Label
  pshlbl(LTEND, endlbl);   //and Push onto Stack
  reqlbl(loplbl);           //Get or Create/Set Loop Label
  //newlbl(loplbl);          //create Loop Label
  //setlbl(loplbl);          //Set to Emit on Next Line
  pshlbl(LTLOOP, loplbl);  //Push onto Stack
  if (!look(')')) {  
    newlbl(cndlbl);          //Create Conditional Skip Label
    prscnd(')', TRUE);       //Parse Conditional Expession
    asmlin("JMP", endlbl);   //Emit Jump to End of Loop
    setlbl(cndlbl);          //and Set Label to Emit on Next Line
  }
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
  prsfnc(';');            //Parse Function Call
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
  if (lbtype == LTDO)
    pdowhl(); //Parse While at End of Do Loop
}

/* parse and compile program statement */
void pstmnt()
{
  DEBUG("Parsing statement '%s'\n", word);
  if (wordis("DO")) {
    pdo();
    return;
  }
  if (wordis("ELSE")) {
    pelse();
    return;
  }
  if (wordis("FOR")) {
    pfor();
    return;  
  }
  if (wordis("IF")) {
    pif();
    return;
  }
  if (wordis("SWITCH")) {
    ERROR("SWITCH not implemented. Use SELECT.\n", word, EXIT_FAILURE);  
  }
  if (wordis("SELECT")) {
    pslct();
    return;
  }
  if (wordis("CASE")) {
    pcase();
    return;  
  }
  if (wordis("DEFAULT")) {
    pdflt();
    return;  
  }
  if (wordis("WHILE")) {
    pwhile();
    return;  
  }
  if(match(':')) {
    prslbl();  //Parse Label
    return;
  }
  if (wordis("ASM"))
    pasm();
  else if (wordis("BREAK"))
    pbrcnt(LFEND);
  else if (wordis("CONTINUE"))
    pbrcnt(LFBGN);
  else if (wordis("GOTO"))
    pgoto();
  else if (wordis("INLINE"))
    pinlne();
  else if (wordis("POP"))
    ppop();
  else if (wordis("PUSH"))
    ppush();
  else if (wordis("RETURN"))
    pretrn();
  else
    prssym();
  if (lblcnt && !inblck) {
    DEBUG("Ending implied block\n", 0);
    if (poplbl() == LTDO)
      pdowhl(); //Parse While at End of Do Loop
  }
}

