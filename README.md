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
