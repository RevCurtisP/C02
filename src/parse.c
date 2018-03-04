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
#include "asm.h"
#include "parse.h"

/* Various tests against nxtchr */
int match(char c) {return TF(nxtchr == c);}
int inbtwn(char mn, char mx) {return TF(nxtchr >= mn && nxtchr <= mx);}
int isalph(void) {return isalpha(nxtchr);}
int isanum(void) {return isalnum(nxtchr);}
int isapos(void) {return match('\'');}
int isbin(void)  {return inbtwn('0', '1');}
int isbpre(void) {return TF(isnpre() || isapos());}
int iscpre(void) {return TF(isbpre() || ishash());}
int isdec(void)  {return inbtwn('0', '9');}
int ishash(void) {return match('#');}
int ishexd(void) {return TF(isdec() || inbtwn('A', 'Z'));}
int isnl(void)   {return TF(match('\n') || match('\r'));}
int isnpre(void) {return TF(isdec() || match('$') || match('%'));}
int isoper(void) {return TF(strchr("+-&|^", nxtchr));}
int ispopr(void) {return TF(strchr("+-<>", nxtchr));}
int isprnt(void) {return isprint(nxtchr);}
int isspc(void)  {return isspace(nxtchr);}
int isvpre(void) {return TF(isalph() || iscpre());}
int isxpre(void) {return TF(isvpre() || match('-'));}

/* Process ASCII Character */
char prcchr(char c) {
  if (invasc) c = isalpha(c) ? (islower(c)?toupper(c):tolower(c)) : c;
  if (mskasc) c = c | 0x80;
  if (invasc || mskasc) DEBUG("Character converted to '%c'\n", c)
  return c;
}

/* if Word is s then return TRUE else return FALSE*/
int wordis(char *s) {
  return strcmp(word, s) == 0;
}

/* Get Next Character from Current Input File     *
 * Uses: inpfil -  Input File Handle            *
 * Sets: nxtchr - Next Character in Source File */
char getnxt(void) {
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
void skpspc(void) {
  //DEBUG("Skipping Spaces\n", 0)
  if (isspc()) CCMNT(' '); //Add only the first space to comments
  while (isspc()) 
    getnxt();
}

/* Check if the next printable character is c *
 * and advance past it if it is               *
 * Returns TRUE is character is found,        *
 *         otherwise FALSE                    */
int look(char c) {
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
void expect(char c) {
  if (c == 0) return;
  if (look(c)) return;
  else {
    fprintf(stderr, "Expected Character '%c', but found '%c'\n", c, nxtchr);
    exterr(EXIT_FAILURE);  
  }
}

/* Advance Input File to next printable character */
void skpchr(void) {
	char skip = getnxt();
	DEBUG("Skipped character '%c'\n", skip)
}

/* Advance Input File to end of line */
void skpeol(void) {while (!isnl()) getnxt();}

/* Advance Source File to end of comment    *
 * Recognizes both C and C++ style comments */
void skpcmt(void)
{
  DEBUG("Skipping Comment\n", 0) 
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
void getwrd(void) {
  int wrdlen = 0;
  skpspc();
  if (!isalph()) expctd("Alphabetic Character");
  while (isanum())
  {
    word[wrdlen++] = toupper(getnxt());
  }
  word[wrdlen] = 0;
  ACMNT(word);
}

/* Escape Character */
char escape(char c) {
  DEBUG("Escaping character '%c'\n", c)
  switch (c) {
    case 'r': return 0x0d;
    default: return c;
  }
}

/* Get String */
void getstr(void) {
  char strdel;
  int wrdlen = 0, escnxt = FALSE;
  DEBUG("Parsing string\n", 0)
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
        word[wrdlen++] = prcchr(nxtchr);
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
int prsbin(void) {
  int wrdlen = 0;
  int digit;
  int number = 0;
  if (!match('%'))
    expctd("binary number"); 
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
int prsdec(void) {
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
int prshex(void) {
  int wrdlen = 0;
  int digit;
  int number = 0;
  DEBUG("Parsing hexadecimal constant '", 0)
  if (!match('$'))
    expctd("hexadecimal number"); 
  word[wrdlen++] = getnxt();
  while (ishexd()) {
    DETAIL("%c", nxtchr);    
    word[wrdlen++] = nxtchr;
    if (isdec())
      digit = nxtchr - '0';
    else
      digit = nxtchr - 'A' + 10;
    number = number * 16 + digit;
    skpchr();
  }
  DETAIL("'\n", 0);
  word[wrdlen] = 0;
  return (number);
}

/* Reads Character constant from input file      *
 * Sets: word - Character constant including     *
 *              single quotes                    *
 * Returns: ASCII value of constant              */ 
int prschr(void) {
  int wrdlen = 0;
  char c;
  DEBUG("Parsing character constant\n", 0)
  expect('\'');
  word[wrdlen++] = '\'';
  if (match('\\')) 
    word[wrdlen++] = getnxt();
  c = getnxt();
  DEBUG("Extracted character %c\n", c)
  word[wrdlen++] = prcchr(c);
  expect('\'');
  word[wrdlen++] = '\'';
  word[wrdlen] = 0;
  return (int)c;
}

/* Parse numeric value                      *
 * Args: maxval - maximum allowed value     *
 * Sets: value - parsed number (as string)  *
 *       word - parses text of value        *
 * Returns: parsed number                   */
int prsnum(int maxval) {
  int number;
  skpspc();
  if (!isbpre()) expctd("constant value");
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
  DEBUG("Parsed number '%s' ", word)
  DETAIL("with value '%d'\n", number)

  if (number > maxval) 
    ERROR("Out of bounds constant '%d';\n", number, EXIT_FAILURE)
  
  if (maxval > 255)
    sprintf(value, "$%04X", number);
  else
    sprintf(value, "$%02X", number);
  return number;
}

/* Parse Nuneric Byte Value */
int prsbyt(void) {return prsnum(0xFF);}

/* Find Defined Constant */
void fnddef(char *name) {
  DEBUG("Looking up defined constant '%s'\n", word)
  for (defidx=0; defidx<defcnt; defidx++) {
    if (strcmp(defnam[defidx], name) == 0)
      return;
  }
  defidx = -1;
}

/* Parse Definition */
int prsdef(void) {
  expect('#');
  getwrd(); //Get Constant Name
  fnddef(word);
  if (defidx < 0) 
    ERROR("Undefined constant '%s'\n", word, EXIT_FAILURE)
  
  strcpy(value, word);
  return defval[defidx];
}

/* Parse numeric constant                      *
 * Args: maxval - maximum allowed value        *
 * Sets: cnstnt - the constant (as an integer) *
 *       value - the constant (as asm arg)     *
 *       valtyp - value type (CONSTANT)        *
 *       word - constant (as a string)         *
 * Note: Value is converted to hexadecimal     *
 *       because DASM uses the format 'c for   *
 *       character arguments instead of 'c'    */
void prscon(void) {
  skpspc();
  if (ishash())
    cnstnt = prsdef();
  else
    cnstnt = prsbyt();
  valtyp = CONSTANT;
  strcpy(word, value); //Patch for DASM
  strcpy(value, "#");
  strcat(value, word);
  DEBUG("Generated constant '%s'\n", value)
}

/* Get Value Type */
int gettyp(void) {
  if (match('(')) return FUNCTION;
  else if (match('[')) return ARRAY;
  else return VARIABLE;
}

/* Parse arithmetic or bitwise operator */
void prsopr(void) {
  if (!isoper())
    expctd("Arithmetic or bitwise operator");
  oper = getnxt();
  DEBUG("Parsed operator '%c'\n", oper)
  CCMNT(oper);
  skpspc();
}


/* Generate Post-Operation Error */
void poperr(char* name) {
  fprintf(stderr, "Illegal post-operation %c%c on register %s\n", oper, oper, name);
  exterr(EXIT_FAILURE);
}

/* Process Post Operator */
void prcpst(char* name, char *index) {
  DEBUG("Processing post operation '%c'\n", oper)
  if (strlen(index)) { 
      asmlin("LDX", index);
      strcat(name,",X");
  }
  switch(oper) {    
    case '+': 
      if (strcmp(name, "X")==0)
        asmlin("INX", "");
      else if (strcmp(name, "Y")==0)
        asmlin("INY", "");
      else if (strcmp(name, "A")==0)
        poperr(name); //65C02 supports implicit INC, 6502 does not
      else 
        asmlin("INC", name);
      break;
    case '-':
      if (strcmp(name, "X")==0)
        asmlin("DEX", "");
      else if (strcmp(name, "Y")==0)
        asmlin("DEY", "");
      else if (strcmp(name, "A")==0)
        poperr(name); //65C02 supports implicit DEC, 6502 does not
      else 
        asmlin("DEC", name);
      break;
    case '<':
      if (strcmp(name, "X")==0)
        poperr(name); //Index Register Shift not Supported
      else if (strcmp(name, "Y")==0)
        poperr(name); //Index Register Shift not Supported
      else if (strcmp(name, "A")==0)
        asmlin("ASL", "");
      else
        asmlin("ASL", name);
      break;
    case '>':
      if (strcmp(name, "X")==0)
        poperr(name); //Index Register Shift not Supported
      else if (strcmp(name, "Y")==0)
        poperr(name); //Index Register Shift not Supported
      else if (strcmp(name, "A")==0)
        asmlin("LSR", "");
      else
       asmlin("LSR", name);
      break;
    default:
      fprintf(stderr, "Unrecognized post operator '%c'\n", oper);
      exterr(EXIT_FAILURE);
  }
}

/* Parse Post Operator */
int prspst(char trmntr, char* name, char* index) {
  oper = getnxt();
  CCMNT(oper);
  DEBUG("Checking for post operation '%c'\n", oper)
  if (nxtchr == oper) {
    skpchr();
    CCMNT(oper);
    expect(trmntr);
    prcpst(name, index);  //Process Post-Op
    oper = 0;
  }
  else {
    DEBUG("Not a post operation\n", 0)
  }
  return oper;
}
