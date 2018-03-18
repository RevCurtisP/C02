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
 
Some of the subdirectories contain a c02.bat file, which will compile the .c02 program, then run dasm to assemble the code. However, the path to dasm is hardcoded, so you will likely need to change it.

The file [c02.sh](./c02sh) provides the same functionality in Linux, but it may not be in a working state.

## Syntax Examples
```
/* Constants */
const #TRUE = $FF, #FALSE = 0; //Constant
enum {BLACK, WHITE, RED, CYAN, PURPLE, GREEN, BLUE, YELLOW}; 

/* Structures */
struct record {char name[8]; char index;}; //Struct Definition
struct record rec;                         //Struct Declaration

/* Declarations */
char i, j;             //Variables 
char debug = #TRUE;    //Variable initialized to constant
char flag = %01010101; //Variable initialized to literal
char r[7];             //8 byte Array 
char s = "string";     //Array initialized to string
char m = {1,2,3};      //Array initialized to list

/* Functions Declarations */
void myfunc(); //Forward declaration of function
char min(tmp1, tmp2) {
  //function definition
}

/* Assignments */
hmove; s80vid;             //Implicit Assignments
x = 0; y = a; a = 1;       //Register Assignments
b = c + d - e & f | g ^ h; //Assignment and Expression
d[j] = r[i], s[x], t[y];   //Array Indexing
a<< ;b[i]>>; x++; y--;     //Post-Operations

/* Function Calls */
i = abs(n); j = min(b,c), k = max(d,e); plot(h,v,c);
n = mult(e+f, div(m+n,d)) - t;
puts("string"); putc(#CR); fputs(fp, &line);
c = getc(); i = strchr(c, &s); row,col = scnpos(); 
push d,r; mult(); pop p;        //Pass via Stack
iprint(); inline "Hello World"; //Pass Inline String
irect(); inline 10,10,100,100;  //Pass Inline Parameters

/* Control Structures */
if (c = 27) goto end;
if (n) q = div(n,d) else puts("Division by 0!");
c = 'A' ; while (c <= 'Z') { putc(c); c++; } 
while() { c=rdkey; if (c=0) continue; putchr(c); if (c=13) break; }
do c = rdkey(); while (c=0);
do (c = getchr(); putchr(c); while (c<>13)
for (c='A'; c<='Z'; c++) putc(c);
for (i=strlen(s)-1;i:+;i--) putc(s[i]);
for (i=0;c>0;i++) { c=getc(); s[i]=c }
select (getc()) {
  case $0D: putln("The Enter key");
  case 'A','a': putln ("The letter A");
  default: putln("some other key");
}
end: //Label
```
