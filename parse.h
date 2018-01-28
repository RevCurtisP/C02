/*************************************
 * C02 Input File Parsing Routines   *
 *************************************/

#define TF(x) (x) ? TRUE : FALSE;

enum stypes {CONSTANT, VARIABLE, ARRAY, FUNCTION};  //Symbol Types
enum etypes {ETDEF, ETMAC};                         //Definition Types

char word[LINELEN];   //Word parsed from source file
char uword[LINELEN];  //Word converted too uppercase
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

int match(char c);             //Does Next Character match c
int inbtwn(char mn, char mx); //Is Next Character in Range ()mn - mx)
int isprnt();                  //Is Next Character Printable
int isalpa();                  //Is Next Character Alphabetic
int isanum();                  //Is Next Character AlphaNumeric
int isdec();                   //Is Next Character a Decimal Digit
int ishexd();                  //Is Next Character a Hexadecimal Digit
int isbin();                   //Is Next Character a Binary Digit
int isnl();                    //Is Next Character a NewLine
int isnpre();                  //Is Next Character a Numeric Prfix
int isspc();                   //Is Next Character a Space
int isapos();                  //Is Next Character an Apostrophe
int isvpre();                  //Is Next Character a Value Prefix
int isoper();                  //Is Next Character an Operator
int ispopr();                  //Is Next Character a Post-Operator

int invasc;                    //Invert ASCII Flag
char invchr(char c);           //Invert Character Case

int wordis(char *s);           //Does word match s

char getnxt();                 //Return Next Character and Advance
int look(char c);              //Look for Character 
void expect(char c);           //Look for Character and Exit if not found

void skpchr();                 //Advance to Next Character
void skpeol();                 //Skip to End of Line 
void skpspc();                 //Advance to Next Printable Character
void skpcmt();                 //Skip to End of Comment

void getwrd();                 //Get Next Word

int prsdec();                  //Get Decimal Number 

void prscon();                 //Parse a Constant
void prsopr();                 //Parse Arithmetic Operator
void prsvar(int alwreg);       //Parse Variable

