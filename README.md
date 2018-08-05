![C02 6502 Compiler](https://github.com/RevCurtisP/C02/raw/master/art/logo.png)

C02 is a simple C-syntax language designed to generate highly optimized
code for the 6502 microprocessor. The C02 specification is a highly
specific subset of the C standard with some modifications and extensions

The compiler generates assembly language code, currently targeted to the DASM assembler.

See the top-level [documentation](doc/c02.txt) for more information.

## Building the Compiler
In Linux, use gcc and the [Makefile](./Makefile).

In Windows use the [Pelles C IDE](http://www.smorgasbordet.com/pellesc/) and the file [./C02.ppj](c02.ppj).

## Compiling C02 programs
At a command line, cd into the appropriate directory, e.g. [py65](py65/) or [vic20](vic20/).

Execute the c02 compiler from the parent directory using the c02 source file (without the .c02 extension) as an argument.

For example, to compile the program [conds.c02](py65/conds.c02), in the [py65](py65/) directory, use the command

    ../c02 conds

in Linux, or

    ..\c02 conds

in Windows.
 
Some of the subdirectories contain a c02.bat file, which will compile 
the .c02 program, then run dasm to assemble the code. However, the path 
to dasm is hardcoded, so you will likely need to change it.

The file [c02.sh](./c02sh) provides the same functionality in Linux, 
but it may not be in a working state.

## Syntax Examples
```
/* Directives */
#include <header.h02>   //Include header from standard library
#include "inc/hdr.h02"  //Include header from local directory
#pragma origin 8192     //Set start address of object code
#pragma zeropage $80    //Set start address of zero page variables 


/* Constants */
#define TRUE = $FF ; //Constants
#define FALSE = 0
enum {BLACK, WHITE, RED, CYAN, PURPLE, GREEN, BLUE, YELLOW}; 

/* Structures */
struct record {char name[8]; char index;}; //Struct Definition
struct record rec;                         //Struct Declaration

/* Variables and Array Declarations */
char i, j;                   //Variables 
zeropage p,q;                //Variables in zeropage
const char nine = 9;         //Const variable set to decimal literal
const char maxsiz = $FF;     //Const variable set to hexadecimal literal
const char flag = %01010101; //Const variable set to binary literal
const char debug = #TRUE;    //Const variable set to constant
char r[7];                   //8 byte array 0 (decimal dimension)
aligned char m[$FF];         //256 byte array aligned to page boundary
const char s = "string";     //Const array set to string literal
const char m = {1,2,3};      //Const array set to literal list
const char t = {"one", 1);   //Const array set to mixed list

/* Functions Declarations */
void myfunc(); //Forward declaration of function
char myfunp(tmp1, tmp2, tmp3) {
  //function code
}

/* Assignments */
hmove; s80vid;               //Implicit Assignments
x = 0; y = a; a = 1;         //Register Assignments
b = c + d - e & f | g ^ h;   //Assignment and Expression
r[i] = s[j+1] & t[k-1];      //Array Indexing
d[j] = r[a] + s[x] + t[y];   //Arrays Indexed by Register
r = (i>j) ? d[i] : e[j];     //Shortcut If
a<< ;b[i]>>; x++; y--;       //Post-Operations

/* Function Calls */
i = abs(n); j = min(b,c), plot(h,v,c);  //Up to Three Char Arguments
q = div(m+n,d)) - t; n = mult(e+f, z);  //Expression in First Arg Only 
puts("string"); fputs(fp, &line);       //Passing Strings and Arrays
setdst(&dst); n = strcpy(&src);         //Using Multiple String Agruments
c = getc(); setptr(*,addrhi,addrlo);    //No and Skipped Arguments
row,col = scnpos(); i,j,k = get3d();    //Plural Assignments
push d,r; mult(); pop p;                //Pass Arguments via Stack
iprint(); inline "Hello World";         //Pass Inline String Argument
irect(); inline 10,10,100,100;          //Pass Inline Char Arguments
icpstr(); inline &dst, &src;            //Pass Inline Address Arguments

/* Control Structures */
if (c = 27) goto end;
if (n) q = div(n,d) else puts("Division by 0!");
if (b==0 || b>10 && b<20) fprint(n,"input %d in range"); 
c = 'A' ; while (c <= 'Z') { putc(c); c++; } 
while() { c=rdkey; if (c=0) continue; putchr(c); if (c=13) break; }
do c = rdkey(); while (c=0);
do {c = getchr(); putchr(c);} while (c<>13)
for (c='A'; c<='Z'; c++) putc(c);
for (i=strlen(s)-1;i:+;i--) putc(s[i]);
for (i=0;c>0;i++) { c=getc(); s[i]=c }
select (getc()) {
  case $0D: putln("The Enter key");
  case #ESCKEY: if (#NOESC) break; goto escape; 
  case 'A','a': putln ("The letter A");
  default: putln("some other key");
}
end: //Label
```
