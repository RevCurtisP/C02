/*************************************
 * C02 Input File Parsing Routines   *
 *************************************/

#define TF(x) (x) ? TRUE : FALSE;

enum stypes {CONSTANT, VARIABLE, ARRAY, FUNCTION};  //Symbol Types
enum etypes {ETDEF, ETMAC};                         //Definition Types

char nxtwrd[LINELEN]; //Next Word (from DEFINE lookup)
int  nxtptr;          //Pointer to next character in nxtwrd
char value[LINELEN];  //Term parsed from equation
int  valtyp;          //Value Type
char oper;            //Arithmetic or Bitwise Operator
int  cnstnt;          //Value of Parsed Constant  

char defnam[MAXDEF+1][VARLEN+1]; //Definition Name Table
int  defval[MAXDEF+1];           //Definition Value Table
int  defcnt;                     //Number of Definitions Defined
int  defidx;                     //Index into Definition Tables

int invasc;                    //Invert ASCII Flag

int match(char c);             //Does Next Character match c
int inbtwn(char mn, char mx);  //Is Next Character in Range ()mn - mx)
int isalph();                  //Is Next Character Alphabetic
int isanum();                  //Is Next Character AlphaNumeric
int isapos();                  //Is Next Character an Apostrophe
int isbin();                   //Is Next Character a Binary Digit
int isbpre();                  //
int iscpre();                  //Is Next Character a Constant
int isdec();                   //Is Next Character a Decimal Digit
int ishash();                  //Is Next Character a Byte Value
int ishexd();                  //Is Next Character a Hexadecimal Digit
int isnl();                    //Is Next Character a NewLine
int isnpre();                  //Is Next Character a Numeric Prfix
int isoper();                  //Is Next Character an Operator
int ispopr();                  //Is Next Character a Post-Operator
int isprnt();                  //Is Next Character Printable
int isspc();                   //Is Next Character a Space
int isvpre();                  //Is Next Character a Value Prefix
int isxpre();                  //Is Next Character an Expression Prefix



char escape(char c);        //Escape Character
void expect(char c);        //Look for Character and Exit if not found
void fnddef(char *name);    //Find Definition
char getnxt();              //Return Next Character and Advance
void getstr();              //Get String
int  gettyp();              //Get Value Type
void getwrd();              //Get Next Word
char invchr(char c);        //Invert Character Case
int  look(char c);          //Look for Character 
void prcidx(char *name, char *index);   //Process Array Index
void prcpst(char* name, char *index);   //Process Post Operator
int  prsbin();              //Parse Binary Number
int  prsbyt();              //Parse Numeric Byte
int  prschr();              //Parse Character Constant
void prscon();              //Parse a Constant
int  prsdec();              //Parse Decimal Number 
int  prsdef();              //Parse Definition
int  prshex();              //Parse Hexadecimal Number
int  prsnum(int maxval);    //Parse Numeric
void prsopr();              //Parse Arithmetic Operator
int  prspst(char trmntr, char* name, char* index);  //Parse Post Operator
void skpchr();              //Skip Next Character
void skpcmt();              //Skip to End of Comment
void skpeol();              //Skip to End of Line 
void skpspc();              //Advance to Next Printable Character
int wordis(char *s);        //Does word match s

