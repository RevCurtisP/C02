/*************************************
 * C02 Input File Parsing Routines   *
 *************************************/

#define TF(x) (x) ? TRUE : FALSE;

enum stypes {LITERAL, VARIABLE, REGISTER, ARRAY, FUNCTION};  //Symbol Types
enum etypes {ETDEF, ETMAC};                         //Definition Types

char nxtwrd[LINELEN]; //Next Word (from DEFINE lookup)
int  nxtptr;          //Pointer to next character in nxtwrd
char value[LINELEN];  //Term parsed from equation
int  valtyp;          //Value Type
char oper;            //Arithmetic or Bitwise Operator
int  litval;          //Value of Parsed Literal  

char connam[MAXCON+1][CONLEN+1]; //Constant Name Table
int  conval[MAXCON+1];           //Constant Value Table
int  concnt;                     //Number of Constants Defined
int  conidx;                     //Index into Constant Tables

int invasc;                    //Invert ASCII Flag
int mskasc;                    //Set High Bit Flag

int match(char c);             //Does Next Character match c
int isalph();                  //Is Next Character Alphabetic
int isanum();                  //Is Next Character AlphaNumeric
int isapos();                  //Is Next Character an Apostrophe
int isbin();                   //Is Next Character a Binary Digit
int isbpre();                  //
int islpre();                  //Is Next Character a Constant
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

void expect(char c);        //Look for Character and Exit if not found
char getnxt();              //Return Next Character and Advance
void getstr();              //Get String
int  gettyp();              //Get Value Type
void getwrd();              //Get Next Word
int  look(char c);          //Look for Character 
int  prsbyt();              //Parse Numeric Byte
void prslit();              //Parse Literal
int  prsnum(int maxval);    //Parse Numeric
void prsopr();              //Parse Arithmetic Operator
int  prspst(char trmntr, char* name, char* index);  //Parse Post Operator
void skpchr();              //Skip Next Character
void skpcmt();              //Skip to End of Comment
void skpspc();              //Advance to Next Printable Character
int  wordis(char *s);       //Does word match s

