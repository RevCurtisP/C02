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
#include "label.h"

/* Various tests against nxtchr */
int match(char c) {return TF(nxtchr == c);}
int inbtwn(char mn, char mx) {return TF(nxtupc >= mn && nxtupc <= mx);}
int isalph(void) {return isalpha(nxtchr);}
int isalst(void) {return TF(isalph() || match('*'));}
int isanum(void) {return isalnum(nxtchr);}
int isapos(void) {return match('\'');}
int isbin(void)  {return inbtwn('0', '1');}
int isbpre(void) {return TF(isnpre() || isapos());}
int isdec(void)  {return inbtwn('0', '9');}
int iscpre(void) {return match('#');}
int ishexd(void) {return TF(isdec() || inbtwn('A', 'Z'));}
int islpre(void) {return TF(isbpre() || iscpre() || isszop() || isxfop());}
int isnl(void)   {return TF(match('\n') || match('\r') || match(EOF));}
int isnpre(void) {return TF(isdec() || match('$') || match('%'));}
int isoper(void) {return TF(strchr("+-&|^", nxtchr));}
int ispopr(void) {return TF(strchr("+-<>", nxtchr));}
int isprnt(void) {return isprint(nxtchr);}
int isspc(void)  {return isspace(nxtchr);}
int isszop(void) {return match('@');}
int isvpre(void) {return TF(isalph() || islpre());}
int isxfop(void) {return match('?');}
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
  while (isspc()) getnxt();
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
  else ERROR("Expected Character '%c'\n", c, EXIT_FAILURE)
}

/* Advance Input File to next printable character */
void skpchr(void) {
	getnxt();
}

/* Advance Input File to end of line */
void skpeol(void) {while (!isnl()) getnxt();}

/* Advance Source File to end of comment    *
 * Recognizes both C and C++ style comments */
void skpcmt(void)
{
  DEBUG("Skipping Comment\n", 0) 
  skpchr();               //skip initial /
  if (match('/')) skpeol(); //if C style comment skip rest of line
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
  else expctd("/ or *");   //if neither error out
}

/* Reads next Word in current Input File, where      *
 *   a Word is a sequence of AlphaNumeric characters *
 * Sets: word - the Word read from the source file   */
void getwrd(void) {
  wrdlen = 0;
  skpspc();
  if (!isalph()) expctd("Alphabetic Character");
  while (isanum()) word[wrdlen++] = toupper(getnxt());
  word[wrdlen] = 0;
  ACMNT(word);
}

/* Escape Character */
char escape(char c) {
  DEBUG("Escaping character '%c'\n", c)
  switch (c) {
    case 'a': return 0x07; //Alert (Beep/Bell)
    case 'b': return 0x08; //Backspace
    case 'e': return 0x1B; //Escape
    case 'f': return 0x0C; //Form Feed
    case 'n': return 0x0A; //Newline (Line Feed)
    case 'r': return 0x0D; //Return (Carriage Return)
    case 't': return 0x09; //Tab (Horizontal)
    case 'v': return 0x0B; //Vertical Tab
    default: return c;
  }
}

/* Get String                 *
 * Sets: word = parsed string 
 *       wrdlen = length of string (including terminator) */
void getstr(void) {
  char strdel;
  int escnxt = FALSE;
  wrdlen = 0;
  DEBUG("Parsing string\n", 0)
  strdel = getnxt();  //Get String Delimiter
  CCMNT(strdel);
  while(!match(strdel) || escnxt) {
    CCMNT(nxtchr);
    if (escnxt) {
      word[wrdlen++] = escape(getnxt());
      escnxt = FALSE;    
    }
    else {
      if (match('\\')) escnxt = TRUE;
      else word[wrdlen++] = prcchr(nxtchr);
      skpchr();
    }  
  }
  skpchr(); //Skip End Delimiter
  CCMNT(strdel);
  word[wrdlen] = 0;
}

/* Read Binary number from input file                *
 *   a Binary is a series of binary digits (0 or 1)  *
 *   prefixed with '%'                               *
 * Sets: word - binary number including leading '%'  *
 * Returns: integer value of number                  */ 
int prsbin(void) {
  wrdlen = 0;
  int digit;
  int number = 0;
  if (!match('%')) expctd("binary number"); 
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
  wrdlen = 0;
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
  wrdlen = 0;
  int digit;
  int number = 0;
  DEBUG("Parsing hexadecimal literal '", 0)
  if (!match('$')) expctd("hexadecimal number"); 
  word[wrdlen++] = getnxt();
  while (ishexd()) {
    DETAIL("%c", nxtchr);    
    word[wrdlen++] = nxtchr;
    if (isdec()) digit = nxtchr - '0';
    else digit = nxtupc - 'A' + 10;
    number = number * 16 + digit;
    skpchr();
  }
  DETAIL("'\n", 0);
  word[wrdlen] = 0;
  return (number);
}

/* Read Character Literal from Input File   *
 * Sets: word - Character literal including *
 *              single quotes               *
 * Returns: ASCII value of literal          */ 
int prschr(void) {
  wrdlen = 0;
  char c;
  DEBUG("Parsing character literal\n", 0)
  word[wrdlen++] = getnxt(); //Initial Single Quote
  if (match('\\')) word[wrdlen++] = getnxt();
  c = getnxt();
  DEBUG("Extracted character %c\n", c)
  word[wrdlen++] = prcchr(c);
  if (!match('\'')) expctd("character delimiter");
  word[wrdlen++] = getnxt();
  word[wrdlen] = 0;
  return (int)c;
}

/* Parse numeric value                      *
 * Args: maxval - maximum allowed value     *
 * Sets: value - parsed number (as string)  *
 *       word - parsed text of value        *
 * Returns: parsed number                   */
int prsnum(int maxval) {
  int number;
  skpspc();
  if (!isbpre()) expctd("literal value");
  switch(nxtchr) {
    case '%':  number = prsbin(); break;
    case '$':  number = prshex(); break;
    case '\'': number = prschr(); break;
    default:   number = prsdec();
  }
  DEBUG("Parsed number %s ", word)
  DETAIL("with value %d\n", number)
  if (number > maxval)  ERROR("Out of bounds literal '%d';\n", number, EXIT_FAILURE)
  if (maxval > 255) sprintf(value, "$%04X", number);
  else sprintf(value, "$%02X", number);
  ACMNT(word)
  return number;
}

/* Parse Nuneric Byte Value */
int prsbyt(void) {return prsnum(0xFF);}

/* Find Defined Constant */
void fnddef(char *name) {
  DEBUG("Looking up defined constant '%s'\n", word)
  for (conidx=0; conidx<concnt; conidx++) 
    if (strcmp(connam[conidx], name) == 0) return;
  conidx = -1;
}

/* Parse Constant */
int prscon(void) {
  expect('#');
  getwrd(); //Get Constant Name
  fnddef(word);
  if (conidx < 0) ERROR("Undefined constant '%s'\n", word, EXIT_FAILURE)
  strcpy(value, word);
  return conval[conidx];
}

/* Parse numeric literal                       *
 * Args: maxval - maximum allowed value        *
 * Sets: litval - literal (as an integer)      *
 *       value - literal (as asm arg)          *
 *       valtyp - value type (LITERAL)         *
 *       word - literal (as a string)          *
 * Note: Value is converted to hexadecimal     *
 *       because DASM uses the format 'c for   *
 *       character arguments instead of 'c'    */
void prslit(void) {
  skpspc();
  if      (iscpre()) litval = prscon(); //Parse Constant
  else if (isszop()) litval = psizof(); //Parse SizeOf Operator
  else if (isxfop()) litval = pidxof(); //Parse IndexOf Operator
  else               litval = prsbyt(); //Parse Byte Value
  valtyp = LITERAL;
  strcpy(word, value); //Patch for DASM
  strcpy(value, "#");
  strcat(value, word);
  DEBUG("Generated literal '%s'\n", value)
}

/* Get Value Type */
int gettyp(void) {
  if (match('(')) return FUNCTION;
  else if (match('[')) return ARRAY;
  else return VARIABLE;
}

/* Parse arithmetic or bitwise operator */
void prsopr(void) {
  if (!isoper()) expctd("Arithmetic or bitwise operator");
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
void prcpst(int isint, char* name, char *index) {
  DEBUG("Processing post operation '%c'\n", oper)
  char name1[VARLEN+3];
  strcpy(name1, name); strcat(name1, "+1");
  if (strlen(index)) { 
      asmlin("LDX", index);
      strcat(name,",X");
  }
  switch(oper) {    
    case '+': 
      if      (strcmp(name, "X")==0) asmlin("INX", "");
      else if (strcmp(name, "Y")==0) asmlin("INY", "");
      else if (strcmp(name, "A")==0) poperr(name); //65C02 supports implicit INC, 6502 does not
      else {
	    asmlin("INC", name);
		if (isint) {
		  newlbl(skplbl);
		  asmlin("BNE", skplbl);
		  asmlin("INC", name1);
		  setlbl(skplbl);
        }
      }   
	  break;
    case '-':
      if      (strcmp(name, "X")==0) asmlin("DEX", "");
      else if (strcmp(name, "Y")==0) asmlin("DEY", "");
      else if (strcmp(name, "A")==0) poperr(name); //65C02 supports implicit DEC, 6502 does not
      else {
        if (isint) {
		  newlbl(skplbl);
		  asmlin("LDA", name);
		  asmlin("BNE", skplbl);
		  asmlin("DEC", name1);
		  setlbl(skplbl);
	    }
        asmlin("DEC", name);
      }
      break;
    case '<':
      if      (strcmp(name, "X")==0) poperr(name); //Index Register Shift not Supported
      else if (strcmp(name, "Y")==0) poperr(name); //Index Register Shift not Supported
      else if (strcmp(name, "A")==0) asmlin("ASL", ""); 
      else {
	    asmlin("ASL", name);
	    if (isint) asmlin("ROL", name1);
	  }
      break;
    case '>':
      if      (strcmp(name, "X")==0) poperr(name); //Index Register Shift not Supported
      else if (strcmp(name, "Y")==0) poperr(name); //Index Register Shift not Supported
      else if (strcmp(name, "A")==0) asmlin("LSR", "");
      else {
		asmlin("LSR", name);
	    if (isint) asmlin("ROR", name1);
      }
      break;
    default:
      ERROR("Unrecognized post operator '%c'\n", oper, EXIT_FAILURE)
  }
}

/* Parse Post Operator */
int prspst(char trmntr, int isint, char* name, char* index) {
  oper = getnxt();
  CCMNT(oper);
  DEBUG("Checking for post operation '%c'\n", oper)
  if (nxtchr == oper) {
    skpchr();
    CCMNT(oper);
    expect(trmntr);
    prcpst(isint, name, index);  //Process Post-Op
    return 0;
  }
  DEBUG("Not a post operation\n", 0)
  return oper;
}
