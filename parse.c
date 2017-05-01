/*************************************
 * C02 Input File Parsing Routines   *
 *************************************/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>

#include "common.h"
#include "files.h"
#include "parse.h"

char opstr[2]; //Operator as a String

/* Various tests against nxtchr */
int match(char c) {return TF(nxtchr == c);}
int inbtwn(char mn, char mx) {return TF(nxtchr >= mn && nxtchr <= mx);}
int isprnt() {return isprint(nxtchr);}
int isalph() {return isalpha(nxtchr);}
int isanum() {return isalnum(nxtchr);}
int isdec()  {return inbtwn('0', '9');}
int ishexd() {return TF(isdec() || inbtwn('A', 'Z'));}
int isbin()  {return inbtwn('0', '1');}
int isnl()   {return TF(match('\n') || match('\r'));}
int isspc()  {return isspace(nxtchr);}
int isnpre() {return TF(isdec() || match('$') || match('%'));}
int isapos() {return match('\'');}
int iscpre() {return TF(isnpre() || isapos());}
int isvpre() {return TF(isalph() || iscpre());}
int isoper() {return TF(strchr("+-&|^", nxtchr));}
int ispopr() {return TF(strchr("+-<>", nxtchr));}

/* if Word is s then return TRUE else return FALSE*/
int wordis(char *s)
{
  return strcmp(word, s) == 0;
}

/* Get Next Character from Current Input File     *
 * Uses: inpfil -  Input File Handle            *
 * Sets: nxtchr - Next Character in Source File */
char getnxt() 
{
  int i;
  int wascr = match('\r');
  char c = nxtchr;
  //if (nxtwrd[nxtptr])          //If nxtwrd is set
  //  nxtchr = nxtwrd[nxtptr++]; //  Pop First Character from nxtwrd
  //else                      
    nxtchr = fgetc(inpfil);
  nxtupc = toupper(nxtchr);	
  if (wascr && match('\n')) return c; //Skip NL after CR
  if (isnl()) curcol=1; else curcol++;
  if (curcol == 1) curlin++;
  return c;
}

/* Advance Input File to next printable character */
void skpspc() 
{
  DEBUG("Skipping Spaces\n", 0);
  while (isspc()) 
    getnxt();
}

/* Check if the next printable character is c *
 * and advance past it if it is               *
 * Returns TRUE is character is found,        *
 *         otherwise FALSE                    */
int look(char c) 
{
  int found;
  skpspc();
  found = match(c);
  if (found) {
    skpchr();
    CCMNT(c);
  }
  return found;
} 

/* if next printable character is c then skip, else generate error */ 
void expect(char c)
{
  if (look(c))
    return;
  else {
    printf("Expected Character '%c', but found '%c'\n", c, nxtchr);
    exterr(EXIT_FAILURE);  
  }
}

/* Advance Input File to next printable character */
void skpchr() {char skip = getnxt();}

/* Advance Input File to end of line */
void skpeol() {while (!isnl()) getnxt();}

/* Advance Source File to end of comment    *
 * Recognizes both C and C++ style comments */
void skpcmt()
{
  DEBUG("Skipping Comment\n", 0);     
  skpchr();               //skip initial /
  if (match('/'))         //if C style comment
    skpeol();        //  skip rest of line
  else if (match('*'))    //if C++ style comment
    while (TRUE)          //  skip to */
    {
      skpchr(); 
      if (!match('*')) continue;
      while (match('*')) skpchr(); 
      if (!match('/')) continue;
      skpchr();
      break;	
      //todo: add code to catch unterminated comment
  	}
  else                    //if neither  
    expctd("/ or *");   // error out
}

/* Reads next Word in current Input File, where      *
 *   a Word is a sequence of AlphaNumeric characters *
 * Sets: word - the Word read from the source file   */
void getwrd()
{
  int wrdlen = 0;
  skpspc();
  if (!isalph()) expctd("Alphabetic Character");
  while (isanum())
  {
    word[wrdlen++] = getnxt();
  }
  word[wrdlen] = 0;
}

/* Get Definition Text */
void getdef() 
{
  int wrdlen = 0;
  skpspc();
  while (isprnt()) {
    word[wrdlen++] = getnxt();
  }
  word[wrdlen] = 0;
}

/* Process define directive */
void prsdef() 
{
  getwrd(); //get defined identifier
  DEBUG("Defining '%s'\n", word);
  strncpy(defnam[defcnt], word, VARLEN);
  getdef();  
  DEBUG("Defined as '%s'\n", word);
  strncat(deftxt[defcnt], word, DEFLEN);
  defcnt++;
}

/* Check for and Expand DEFINE  *
 * Sets: nxtwrd, nxtchr, nxtupr *
 *       if match is found      */
void expdef()
{
  int i;
  if (!isalph()) return;
  DEBUG("Checking for definition '%s'\n", word);
  nxtptr = 0;
  for (i=0; i<defcnt; i++) {
    if (strcmp(word, defnam[i]) == 0) {
      strcpy(nxtwrd, deftxt[i]);
      nxtchr = nxtwrd[0];
      nxtupc = toupper(nxtchr);
      DEBUG("Expanded to '%s'\n", word);
      return;
    }
  }
  strcpy(nxtwrd, word);  
}

char escape(char c) 
{
  DEBUG("Escaping character '%c'\n", c);
  switch (c) {
    case 'r': return 0x0d;
    default: return c;
  }
}

/* Get String */

void getstr() {
  char strdel, tmpchr;
  int wrdlen = 0, escnxt = FALSE;
  DEBUG("Parsing string\n", 0);
  strdel = getnxt();  //Get String Delimiter
  CCMNT(strdel);
  while(match(strdel) == escnxt) {
    CCMNT(nxtchr);
    if (escnxt) {
      word[wrdlen++] = escape(getnxt());
      escnxt = FALSE;    
    }
    else {
      if (match('\\')) 
        escnxt = TRUE;
      else
        word[wrdlen++] = nxtchr;
      skpchr();
    }  
  }
  skpchr(); //Skip End Delimiter
  CCMNT(strdel);
  word[wrdlen++] = 0;
}

/* Read Binary number from input file                *
 *   a Binary is a series of binary digits (0 or 1)  *
 *   prefixed with '%'                               *
 * Sets: word - binary number including leading '%'  *
 * Returns: integer value of number                  */ 
int prsbin()
{
  int wrdlen = 0;
  int digit;
  int number = 0;
  if (!match('%'))
    expctd("hexadecimal number"); 
  word[wrdlen++] = nxtchr;
  getnxt();
  while (isbin()) {
    word[wrdlen++] = nxtchr;
    digit = nxtchr - '0';
    number = number * 2 + digit;
    skpchr();
  }
  word[wrdlen] = 0;
  return (number);
}

/* Read Decimal number from input file      *
 * a Decimal is a series of digits (0-9)    *
 * Sets: word - number without leading 0's  *
 * Returns: integer value of number         */ 
int prsdec()
{
  int wrdlen = 0;
  int digit;
  int number = 0;
  if (!isdec()) expctd("Digit");
  while (isdec())
  {
    if (!match('0') || number != 0) //skip leading zeros
      word[wrdlen++] = nxtchr;      
    digit = getnxt() - '0';
    number = number * 10 + digit;
  }
  if (number == 0) word[wrdlen++] = '0';
  word[wrdlen] = 0;
  return(number);
}

/* Reads Hexadecimal number from input file      *
 * Sets: word - Hex number including leading '$' *
 * Returns: integer value of number              */ 
int prshex()
{
  int wrdlen = 0;
  int digit;
  int number = 0;
  DEBUG("Parsing hexadecimal constant\n", 0);
  if (!match('$'))
    expctd("hexadecimal number"); 
  word[wrdlen++] = getnxt();
  while (ishexd()) {
    DEBUG("Found hex digit '%c'\n", nxtchr);    
    word[wrdlen++] = nxtchr;
    if (isdec())
      digit = nxtchr - '0';
    else
      digit = nxtchr - 'A' + 10;
    number = number * 16 + digit;
    skpchr();
  }
  word[wrdlen] = 0;
  return (number);
}

/* Reads Character constant from input file      *
 * Sets: word - Character constant including     *
 *              single quotes                    *
 * Returns: ASCII value of constant              */ 
int prschr()
{
  int wrdlen = 0;
  char c;
  DEBUG("Parsing character constant\n", 0);
  expect('\'');
  word[wrdlen++] = '\'';
  if (match('\\')) 
    word[wrdlen++] = getnxt();
  c = getnxt();
  DEBUG("Extracted character %c\n", c);
  word[wrdlen++] = c;
  expect('\'');
  word[wrdlen++] = '\'';
  word[wrdlen] = 0;
  return (int)c;
}

/* Parse numeric value                      *
 * Args: maxval - maximum allowed value     *
 * Sets: value - parsed number (as string)  *
 * Returns: parsed number                   */
int prsnum(int maxval) 
{
  int number;
  skpspc();
  if (!iscpre()) expctd("constant value");
  switch(nxtchr) {
    case '%': 
      number = prsbin();
      break;
    case '$': 
      number = prshex();
      break;
    case '\'':
      number = prschr();
      break;
    default:
      number = prsdec();
  }
  DEBUG("Parsed number '%s'\n", word);
  DEBUG("with value '%d'\n", number);

  if (number > maxval) {
    ERROR("Out of bounds constant '%d';\n", number, EXIT_FAILURE);
  }
  if (maxval > 255)
    sprintf(value, "$%04X", number);
  else
    sprintf(value, "$%02X", number);
  return number;
}

/* Parse numeric constant                   *
 * Args: maxval - maximum allowed value     *
 * Sets: value - the constant (as a string) *
 *       valtyp - value type (CONSTANT)   */
void prscon(int maxval) 
{
  cnstnt = prsnum(maxval);
  valtyp = CONSTANT;
  if (maxval > 255)
    sprintf(value, "#$%04X", cnstnt);
  else
    sprintf(value, "#$%02X", cnstnt);
  DEBUG("Generated constant '%s'\n", value);
}


/* Get Value Type */
int gettyp()
{
  if (match('(')) return FUNCTION;
  else if (match('[')) return ARRAY;
  else return VARIABLE;
}

/* Parse next word as variable or function name *
 * Sets: value - Identifier Name                *  
 *       valtyp - Identifier Type             */
void prsvar() 
{
  getwrd();
  valtyp = gettyp();
  if (valtyp != FUNCTION) chksym(word);
  strcpy(value, word);
  DEBUG("Parsed variable '%s'\n", value);
}

/* Parse arithmetic or bitwise operator */
void prsopr()
{
  if (!isoper)
    expctd("Arithmetic or bitwise operator");
  oper = getnxt();
  DEBUG("Parsed operator '%c'\n", oper);
  opstr[0] = oper;
  opstr[1] = 0;
  ACMNT(opstr);
  skpspc();
}

