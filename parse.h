/*************************************
 * C02 Input File Parsing Routines   *
 *************************************/

#define TF(x) (x) ? TRUE : FALSE;

enum trmtxts {CONSTANT, VARIABLE, ARRAY, FUNCTION};
enum etypes {ETDEF, ETMAC};      //Definition Types

char word[LINELEN];   //Word parsed from source file
char nxtwrd[LINELEN]; //Next Word (from DEFINE lookup)
int  nxtptr;          //Pointer to next character in nxtwrd
char value[LINELEN];  //Term parsed from equation
int  valtyp;          //Value Type
char oper;            //Arithmetic or Bitwise Operator
int cnstnt;           //Value of Parsed Constant

char defnam[MAXDEF+1][VARLEN+1]; //Definition Name Table
char deftxt[MAXDEF+1][DEFLEN+1]; //Definition Text Table
int  defcnt;                     //Number of Definitions Defined

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

int wordis(char *s);           //Does word match s

char getnxt();                 //Return Next Character and Advance
int look(char c);              //Look for Character 
void expect(char c);           //Look for Character and Exit if not found

void skpchr();                 //Advance to Next Character
void skpeol();                 //Skip to End of Line 
void skpspc();                 //Advance to Next Printable Character
void skpcmt();                 //Skip to End of Comment

void getwrd();                 //Get Next Word

void expdef();  //Check for and Expand Definition

int prsdec();                  //Get Decimal Number 

void prscon(int maxval);       //Parse a Constant
void prsopr();                 //Parse Arithmetic Operator
void prsvar();                 //Parse Variable
