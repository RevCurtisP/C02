/* Apple 1 Header File */

//The keyboard data register returns ASCII codes with the high bit set
//These can be safely printed, since the display hardware ignores the high bit
#define CR   $8D   //Carriage Return key code
#define ESC  $9B   //Escape key code
#define DEL  $DF   //Delete (Backspace) key code

//Memory locations used by monitor program
char* buffer  = $0200 //Monitor Input Buffer

//PIA 6820 Registers
char* kbd     = $D010 //Keyboard Data
char* kbdcr   = $D011 //Keyboard Control Register
char* dsp     = $D012 //Display Data
char* dspcr   = $D013 //Display Control Register

#label exit     $FF00 //Monitor Entry Point

/*
void echo(A)   = $FFEF //Print Character in Accumulator
void prbyte(A) = $FFDC //Print Accumulator as Hexadadecimal number
void prhex(A)  = $FFE5 //Print Low Nybble of Accumulator as Hex Digit
*/

#origin $0300 //$0200 through $02FF used as a string buffer

/*
#asm
#endasm
char getkey();
*/

