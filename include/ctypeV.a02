        PROCESSOR 6502
        ORG $0200
; ctype.h02 assembly language subroutines
;
; All calls replace Accumulator and set Flags
;   Index Registers are not Modified
; isxxxx will load $FF into Accumulator to and Set Carry if True
;         and load $00 into Accumulator to and clear Carry if False
; toxxxx set Carry if character is converted, otherwise clear Carry 

;Character Test Routines
isalnm: JSR isdigt  ;If Char is Digit
        BCS isrts   ;  Return TRUE
                    ;Else
isalph: JSR isuppr  ;If Char is Upper Case
        BCS isrts   ;  Return TRUE
                    ;Else
islowr: JSR islwc   ;If Char is Lower Case
        BVC isbft   ;  Return FALSE Else Return TRUE
;       BCS isfls   ;  Return FALSE
;       BCC istru   ;Else Return TRUE

isuppr: JSR isupc   ;If Char is Uppercase
        BVC isbft   ;  Return FALSE Else Return TRUE
;       BCS isfls   ;  Return FALSE
;       BCC istru   ;Else Return TRUE

isctrl: CLV         ;Clear Overflow
        CMP #$7F    ;If Char = DEL
        BEQ istru   ;  Return TRUE
        CMP #$20    ;Else If Char < ' '
        BVC isbft   ;  Return TRUE Else Return FALSE
;       BCC istru   ;  Return TRUE
;       BCS isfls   ;Else Return FALSE

isdigt: CLV         ;Clear Overflow
        CMP #$30    ;If Char < '0'
        BCC isfls   ;  Return FALSE
        CMP #$3A    ;Else If Char >= ':'
        BVC isbft   ;  Return FALSE Else Return TRUE
;       BCS isfls   ;  Return FALSE
;       BCC istru   ;Else Return True

ispnct: JSR isalnm  ;If Char is Alphanumeric
        BNE isfls   ;  Return FALSE
                    ;Else
isgrph: CMP #$20    ;If Char is Space
        BEQ isfls   ;  Return FALSE
                    ;Else
isprnt: JSR isctrl  ;If Char is Not Control
        BVC isbft   ;  Return TRUE Else Return FALSE
;       BCC istru   ;  Return TRUE 
;       BCS isfls   ;Else Return FALSE

isspc:  CLV         ;Clear Overflow
        CMP #$20    ;If Char is ' '
        BEQ istru   ;  Return TRUE
        CMP #$09    ;If Char < '\t'
        BCC isfls   ;  Return TRUE
        CMP #$0E    ;Else If Char > '\r'
        BVC isbft   ;  Return FALSE Else Return TRUE
;       BCS isfls   ;  Return FALSE
;       BCC istru   ;Else Return TRUE 

isxdgt: JSR touppr  ;Convert to Uppercase
        CMP #$41    ;If Char < 'A'
        BCC isdigt  ;  Check for Digit
        CMP #$5B    ;Else If Char >= 'G'
isbft:  BCS isfls   ;  Return FALSE
                    ;Else
istru:  LDA #$FF    ;Return TRUE
issec:  SEC         ;Set Carry
        RTS         ;and Return

isfls:  LDA #$00   ;Return FALSE
isclc:  CLC        ;Clear Carry
isrts:  RTS        ;Return from Subroutine

;Internal Test Routines - Do Not Change Accumulator
islwc:  CLV         ;Clear Overflow for Calling Routine
        CMP #$61    ;If Char < 'a'
        BCC isrts   ;  Return with Carry Clear
        CMP #$7B    ;Else If Char >= '{'
        BCS isclc   ;  Return with Carry Clear
        BCC issec  ;Else Return with Carry Set

isupc:  CLV         ;Clear Overflow for Calling Routine
        CMP #$41    ;If Char < 'A'
        BCC isrts   ;  Return with Carry Clear
        CMP #$5B    ;Else If Char >= '['
        BCS isclc   ;  Return with Carry Clear
        BCC issec  ;Else Return with Carry Set

;Character Conversion Routines
tolowr: JSR isupc   ;If Char is Not Upper Case
        BCC isrts   ;  Return
        ORA $20     ;Else Set Bit 6
        RTS         ; and Return

touppr: JSR islwc   ;If Char is Not Lower Case
        BCC isrts   ;  Return
        AND $DF     ;Else Clear Bit 6
        RTS         ; and Return

