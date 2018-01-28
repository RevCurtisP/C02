;ctype.h02 assembly language subroutines
;
;C02 Functions modify Accumulator and Flags
;  isxxxx will load $FF into Accumulator to and Set Carry if True
;          and load $00 into Accumulator to and clear Carry if False
;  toxxxx set Carry if character is converted, otherwise clear Carry 
;Machine Language Subroutines modify Flags but not Accumulator
;  Carry will be Set if True and Cleared if False 
;Index Registers are not modified by any routines

;Character Test Functions - Set Accumulator
isalnm: JSR isaln   ;Is Alphanumeric Character
        BVC isbtf
isalph: JSR isalp;  ;Is Alphabetic Character
        BVC isbtf
isctrl: JSR isctl   ;Is Control Character
        BVC isbtf
isdigt: JSR isdgt   ;Is Digit
        BVC isbtf
isgrph: JSR isgrp   ;Is Graphical Character
        BVC isbtf
ishdgt: JSR ishex   ;Is Hex Digit 
        BVC isbtf
islowr: JSR islwr   ;Is Lowercase Character
        BVC isbtf
ispnct: JSR ispnc   ;Is Punctuation Character
        BVC isbtf
isprnt: JSR isprt   ;Is Printable Character
        BVC isbtf
isspce: JSR isspc   ;Is White Space Character
        BVC isbtf
isuppr: JSR isupr   ;Is Uppercase Character
        BVC isbtf

;Internal Routines - Set Accumulator based on Carry Flag
isbtf:  BCC isfls   ;If Carry Set
istru:  LDA #$FF    ;  Return TRUE
        RTS         ;Else 
isfls:  LDA #$00    ;  Return FALSE
        RTS

;C02/ML Character Conversion Routines
tolowr: JSR isupr   ;If Char is Not Upper Case
        BCC isrts   ;  Return
        ORA #$20    ;Else Set Bit 5 
        RTS         ; and Return

touppr: JSR islwr   ;If Char is Not Lower Case
        BCC isrts   ;  Return
        AND #$DF    ;Else Clear Bit 5 
        RTS         ; and Return

;Machine Language Subroutines - Set/Clear Carry, Preserve Accumulator 
isaln:  JSR isdgt  ;If Char is Digit
        BCS isrts   ;  Return Carry Set
                    ;Else
isalp:  JSR isupr  ;If Char is Upper Case
        BCS isrts   ;  Return Carry Set
                    ;Else
islwr:  CMP #$61    ;If Char < 'a'
        BCC isrts   ;  Return with Carry Clear
        CMP #$7B    ;Else If Char >= '{'
        JMP isbcs   ;  Return with Carry Clear Else Return with Carry Set

isupr:  CMP #$41    ;If Char < 'A'
        BCC isrts   ;  Return with Carry Clear
        CMP #$5B    ;Else If Char >= '['
isbcs:  BCS isclc   ;  Return with Carry Clear
        BCC issec   ;Else Return with Carry Set

issec:  SEC         ;Set Carry
        BCS isrts   ;  and Return

isclc:  CLC         ;Clear Carry
isrts:  CLV         ;Clear Overflow - for C02 calls
        RTS         ;Return from Subroutine

isctl:  CMP #$7F    ;If Char = DEL
        BEQ issec   ;  Return Carry Set
        CMP #$20    ;Else If Char < ' '
        JMP isbcs   ;  Return Carry Set Else Return Carry Clear

isdgt:  CMP #$30    ;If Char < '0'
        BCC isrts   ;  Return Carry Clear
        CMP #$3A    ;Else If Char >= ':'
        JMP isbcs   ;  Return FALSE Else Return TRUE

ispnc:  JSR isaln  ;If Char is Alphanumeric
        BCS isclc   ;  Return Carry Clear
                    ;Else
isgrp:  CMP #$20    ;If Char is Space
        BEQ isclc   ;  Return Carry Clear
                    ;Else
isprt:  CMP #$80    ;If Char is High ASCII
        BCS isclc   ;  Return Carry Clear 
        JSR isctl   ;If Char is Not Control
        JMP isbcs   ;  Return Carry Clear Else Return Carry Set

isspc:  CMP #$20    ;If Char is ' '
        BEQ issec   ;  Return Carry Set
        CMP #$09    ;If Char < '\t'
        BCC isrts   ;  Return Carry Clear
        CMP #$0E    ;Else If Char > '\r'
        JMP isbcs   ;  Return Carry Clear Else Return Carry Set

ishex:  CMP #$41    ;If Char < 'A'
        BCC isdgt   ;  Check for Digit
        CMP #$47    ;Else If Char < 'G'
        BCC issec   ;  Return Carry Set
        CMP #$61    ;Else If Char < 'a'
        BCC isrts  ;   Return Carry Clear
        CMP #$67    ;Else If Char >= 'g'
        JMP isbcs   ;  Return Carry Clear Else Return Carry Set

