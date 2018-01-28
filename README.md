# C02

C02 is a simple C-syntax language designed to generate highly optimized
code for the 6502 microprocessor. The C02 specification is a highly
specific subset of the C standard with some modifications and extensions

## Purpose

Why create a whole new language, particularly one with severe restrictions,
when there are already full-featured C compilers available? It can be
argued that standard C is a poor fit for processors like the 6502. The C 
was language designed to translate directly to machine language instructions 
whenever possible. This works well on 32-bit processors, but requires either 
a byte-code interpreter or the generation of complex code on a typical
8-bit processor. C02, on the other hand, has been designed to translate 
directly to 6502 machine language instructions.
 
The C02 language and compiler were designed with two goals in mind.

The first goal is the ability to target machines with low memory: a few 
kilobytes of RAM (assuming the generated object code is to be loaded into 
and ran from RAM), or as little as 128 bytes of RAM and 2 kilobytes of ROM
(assuming the object code is to be run from a ROM or PROM). 

The compiler is agnostic with regard to system calls and library functions. 
Calculations and comparisons are done with 8 bit precision. Intermediate 
results, array indexing, and function calls use the 6502 internal registers.
While this results in compiled code with virtually no overhead, it severely 
restricts the syntax of the language.

The second goal is to port the compiler to C02 code so that it may be
compiled by itself and run on any 6502 based machine with sufficient memory
and appropriate peripherals. This slightly restricts the implementation of
code structures.
