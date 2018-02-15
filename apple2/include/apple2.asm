;Apple 1 program initialization code for c02 programs

;Locations used by Operating System
PROMPT  EQU $33     ;ASCII character for INPUT prompt symbol
LOMEM   EQU $4A     ;
HIMEM   EQU $4C
BASIC   EQU $67         ;Start of BASIC program
VARBLS  EQU $69         ;End of BASIC program and start of variable space
ARRAYS  EQU $6B         ;Start of defined array space
ARRAYE  EQU $6D         ;End of defined array space
STRNGS  EQU $6F         ;Start of string storag

;Locations used by Hi-Res Graphics
HPLOTX  $E0         ;X of last HPLOT
HPLOTY  $E2         ;Y of last HPLOT
HCOLOR  $E4         ;
HIRES   $E6         ;
SCALE   $E7         ;
SHAPES  $E8         ;Shape table start address
XDRAWS  $EA
CSPEED  $F1
ROTATE  $F9

TIMEDM  EQU $BF90       ;Day/month
TIMEYR  EQU $BF91       ;Year
TIMEMN  EQU $BF92       ;Minute
TIMEHR  EQU $BF93       ;Hour



EXIT    EQU  $FF00 ;Monitor Entry Point
ECHO    EQU  $FFEF ;Subroutine - Print Character in Accumulator
PRBYTE  EQU  $FFDC ;Subroutine - Print Accumulator as Hexadadecimal number
PRHEX   EQU  $FFE5 ;Subroutine - Print Low Nybble of Accumulator as Hex Digit

        ORG $0300  ;Start one page above Monitor input buffer

START:  LDX #$FF        ;Reset stack - the monitor doesn't do this
        TXS             ;  (probably because of lack of space) 
        JMP MAIN        ;Execute Program

RDKEY:  BIT KBDCR       ;Check the Keyboard Control Register
        BPL RDKEY       ;  and loop if key not pressed
        LDA KBD         ;  Read key into Accumulator
        RTS

