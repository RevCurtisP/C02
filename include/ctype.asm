; ctype.h02 assembly language subroutines
;
; All calls replace Accumulator and set Flags
;   Index Registers are not Modified
; isxxxx will load $FF into Accumulator to and Set Carry if True
;         and load $00 into Accumulator to and clear Carry if False
; toxxxx set Carry if character is converted, otherwise clear Carry 

;Character Test Routines
isalnm: PHA
        JSR isdigt  ;If Char is Digit
        PLA
        BCS istru   ;  Return TRUE
                    ;Else
isalph: JSR isupc  ;If Char is Upper Case
        BCS istru   ;  Return TRUE
                    ;Else
islowr: JSR islwc   ;If Char is Lower Case
        JMP isbtf   ;  Return TRUE Else Return FALSE

isuppr: JSR isupc   ;If Char is Uppercase
isbtf:  BCS istru   ;  Return TRUE
        BCC isfls   ;Else Return FALSE

isctrl: CMP #$7F    ;If Char = DEL
        BEQ istru   ;  Return TRUE
        CMP #$20    ;Else If Char < ' '
        JMP isbft   ;  Return TRUE Else Return FALSE

isdigt: CMP #$30    ;If Char < '0'
        BCC isfls   ;  Return FALSE
        CMP #$3A    ;Else If Char >= ':'
        JMP isbft   ;  Return FALSE Else Return TRUE

ispnct: PHA
        JSR isalnm  ;If Char is Alphanumeric
        PLA
        BCS isfls   ;  Return FALSE
                    ;Else
isgrph: CMP #$20    ;If Char is Space
        BEQ isfls   ;  Return FALSE
                    ;Else
isprnt: CMP #$80    ;If Char is High ASCII
        BCS isfls   ;  Return FALSE 
        JSR isctrl  ;If Char is Not Control
        JMP isbft   ;  Return TRUE Else Return FALSE

isspce: CMP #$20    ;If Char is ' '
        BEQ istru   ;  Return TRUE
        CMP #$09    ;If Char < '\t'
        BCC isfls   ;  Return TRUE
        CMP #$0E    ;Else If Char > '\r'
        JMP isbft   ;  Return FALSE Else Return TRUE

isxdgt: JSR touppr  ;Convert to Uppercase
        CMP #$41    ;If Char < 'A'
        BCC isdigt  ;  Check for Digit
        CMP #$47    ;Else If Char >= 'G'
isbft:  BCS isfls   ;  Return FALSE
                    ;Else
istru:  LDA #$FF    ;Return TRUE
issec:  SEC         ;Set Carry
        RTS         ;and Return

isfls:  LDA #$00   ;Return FALSE
isclc:  CLC        ;Clear Carry
isrts:  RTS        ;Return from Subroutine

;Internal Test Routines - Do Not Change Accumulator
islwc:  CMP #$61    ;If Char < 'a'
        BCC isrts   ;  Return with Carry Clear
        CMP #$7B    ;Else If Char >= '{'
        JMP isbcs   ;  Return with Carry Clear Else Return with Carry Set

isupc:  CMP #$41    ;If Char < 'A'
        BCC isrts   ;  Return with Carry Clear
        CMP #$5B    ;Else If Char >= '['
isbcs:  BCS isclc   ;  Return with Carry Clear
        BCC issec   ;Else Return with Carry Set

;Character Conversion Routines
tolowr: JSR isupc   ;If Char is Not Upper Case
        BCC isrts   ;  Return
        ORA #$20    ;Else Set Bit 5 
        RTS         ; and Return

touppr: JSR islwc   ;If Char is Not Lower Case
        BCC isrts   ;  Return
        AND #$DF    ;Else Clear Bit 5 
        RTS         ; and Return

