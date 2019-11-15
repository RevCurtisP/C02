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
char b, c, d, e, f, g, h;    //8-bit Variables 
int i, j;                    //16-bit Variables
zeropage char p, q;          //8-bit Variables in Page 0
zeropage int u, v;           //16-bit Variables in Page 0
const char nine = 9;         //Const 8-bit variable set to decimal literal
const char maxsiz = $FF;     //Const 8-bit variable set to hexadecimal literal
const char flag = %01010101; //Const 8-bit variable set to binary literal
const char debug = #TRUE;    //Const 8-bit variable set to constant
const int k = $1234;         //Const 16-bit variable set to hexadecimal literal
char r[7];                   //8 byte Array (decimal dimension)
aligned char m[$FF];         //256 byte array aligned to page boundary
const char n = {1,2,3};      //Const array set to literal list
const char s = "string";     //Const array set to string literal
const char t = {"one", 1);   //Const array set to mixed list

/* Functions Declarations */
void myfunc(); //Forward declaration of function
char fnc(c) { /*function body */}     //One 8-bit Parameter
char fnd(c,d) { /*function body */}   //Two 8-bit Parameters
char fne(c,d,e) { /*function body */} //Three 8-bit Parameters
char fni(i) { /*function body */}     //One 16-bit Parameter
char fnj(c,i) { /*function body */}   //8-bit and 16-bit Parameters

/* Returning from a Function */
return c, d, e;    //Return up to three 8-bit values
return c,j;        //Return an 8-bit an 16-bit value
return i;          //Return a 16-bit value
return;            //No explicit return values

/* Assignments */
hmove; s80vid;               //Implicit Assignments
x = 0; y = a; a = 1;         //Register Assignments
b = c + d - e & f | g ^ h;   //Assignment and Expression
r[f] = m[g+1] & t[h-1];      //Array Indexing
r[j] = r[a] + s[x] + t[y];   //Arrays Indexed by Register
d = (e>f) ? d[e] : e[f];     //Shortcut If
b<< ;c[d]>>; x++; y--;       //Post-Operations

/* Function Calls */
b = abs(c); d = min(e,f), plot(b,c,d); //Up to Three Char Arguments
b = div(c+d,e)) - f; c = mult(d+e, f); //Expression in First Arg Only 
j = swap(i); j = ishift(b, i);         //Pass Int or Char with Int
puts("string"); fputs(f, &s);          //Passing Strings and Arrays
setdst(&r); b = strcpy(&s);            //Using Multiple String Arguments
proc(@record, &record);                //Pass Length and Address of Struct
c = getc(); setptr(?,g,h);             //No Args and Skipped Arguments
b,c = scnpos(); d,e,f = get3d();       //Plural Assignments
push b,c; mult(); pop p;               //Pass Arguments via Stack
iprint(); inline "Hello World";        //Pass Inline String Argument
irect(); inline 10,10,100,100;         //Pass Inline Char Arguments
icpstr(); inline &r, &s;               //Pass Inline Address Arguments

/* Control Structures */
if (c = 27) goto end;
if (b) e = div(f,g) else puts("Division by 0!");
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
