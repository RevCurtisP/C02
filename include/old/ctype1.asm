        PROCESSOR 6502
        ORG $0200
; ctype.h02 assembly language subroutines
;


isalnm: LDX #0    ;ALNUM     
        BPL ctype          
isalph: LDX #1    ;ALPHA
        BPL ctype
isctrl: LDX #2    ;CNTRL
        BPL ctype
isdigt: LDX #3    ;DIGIT
        BPL ctype
isgrph: LDX #4    ;GRAPH
        BPL ctype
islowr: LDX #5    ;LOWER
        BPL ctype
isprnt: LDX #6    ;PRINT
        BPL ctype
ispnct: LDX #7    ;PUNCT
        BPL ctype
isspce: LDX #8    ;SPACE
        BPL ctype
isuppr: LDX #9    ;UPPER
        BPL ctype
isxdgt: LDX #10   ;HXDGT

ctype:  TAY         ;Save Character
        AND #$80    ;If High Bit Set
        BNE cflse   ;  Return False
        LDA cchar,Y ;Get Character Bit Mask
        AND cmask,X ;If it Matches Test Bit Mask
        BNE ctrue   ; Return True
        
cflse:  LDA #0      ;Return False  
        RTS      

ctrue:  LDA #$FF    ;Return True
        RTS

;Informational Only
;CNTRL  EQU %00000001  ;Control Character
;SPACE  EQU %00000010  ;White Space
;BLANK  EQU %00000100  ;Blank 
;PUNCT  EQU %00001000  ;Punctuation
;DIGIT  EQU %00010000  ;Decimal Digit
;HEXDC  EQU %00100000  ;Hexadecimal Digit
;UPPER  EQU %01000000  ;Upper Case
;LOWER  EQU %10000000  ;Lower Case

;Character Test Bit Masks
cmask:  DB  %11010000  ;ALNUM = DIGIT + UPPER + LOWER
        DB  %11000000  ;ALPHA = UPPER + LOWER
        DB  %00000001  ;CNRTL
        DB  %00010000  ;DIGIT
        DB  %11011000  ;GRAPH = PUNCT + DIGIT + UPPER + LOWER
        DB  %10000000  ;LOWER
        DB  %11011100  ;PRINT = BLANK + PUNCT + DIGIT + UPPER + LOWER
        DB  %00001000  ;PUNCT
        DB  %00000010  ;SPACE
        DB  %01000000  ;UPPER
        DB  %00110000  ;HXDGT = DIGIT + HEXDC

;Character Set Bit Masks
cchar:  DB  %00000001  ;00 NUL CNTRL
        DB  %00000001  ;01 SOH CNTRL
        DB  %00000001  ;02 STX CNTRL
        DB  %00000001  ;03 ETX CNTRL
        DB  %00000001  ;04 EOT CNTRL
        DB  %00000001  ;05 ENQ CNTRL
        DB  %00000001  ;06 ACK CNTRL
        DB  %00000001  ;07 BEL CNTRL
        DB  %00000001  ;08 BS  CNTRL
        DB  %00000011  ;09 HT  CNRTL + SPACE
        DB  %00000011  ;0A LF  CNRTL + SPACE
        DB  %00000011  ;0B VT  CNRTL + SPACE
        DB  %00000011  ;0C FF  CNRTL + SPACE
        DB  %00000011  ;0D CR  CNRTL + SPACE
        DB  %00000001  ;0E SS  CNRTL
        DB  %00000001  ;0F SI  CNRTL
        DB  %00000001  ;10 DLE CNRTL
        DB  %00000001  ;11 DC1 CNRTL
        DB  %00000001  ;12 DC2 CNRTL
        DB  %00000001  ;13 DC3 CNRTL
        DB  %00000001  ;14 DC4 CNRTL
        DB  %00000001  ;15 NAK CNRTL
        DB  %00000001  ;16 SYN CNRTL
        DB  %00000001  ;17 ETB CNRTL
        DB  %00000001  ;18 CAN CNRTL
        DB  %00000001  ;19 EM  CNRTL
        DB  %00000001  ;1A SUB CNRTL
        DB  %00000001  ;1B ESC CNRTL
        DB  %00000001  ;1C FS  CNRTL
        DB  %00000001  ;1D GS  CNRTL
        DB  %00000001  ;1E RS  CNRTL
        DB  %00000001  ;1F US  CNRTL
        DB  %00000110  ;20 SPC SPACE + BLANK
        DB  %00001000  ;21 !   PUNCT
        DB  %00001000  ;22 "   PUNCT
        DB  %00001000  ;23 #   PUNCT
        DB  %00001000  ;24 $   PUNCT
        DB  %00001000  ;25 %   PUNCT
        DB  %00001000  ;26 &   PUNCT
        DB  %00001000  ;27 '   PUNCT
        DB  %00001000  ;28 (   PUNCT
        DB  %00001000  ;29 )   PUNCT
        DB  %00001000  ;2A *   PUNCT
        DB  %00001000  ;2B +   PUNCT
        DB  %00001000  ;2C ,   PUNCT
        DB  %00001000  ;2D -   PUNCT
        DB  %00001000  ;2E .   PUNCT
        DB  %00001000  ;2F /   PUNCT
        DB  %00110000  ;30 0   DIGIT + HEX
        DB  %00110000  ;31 1   DIGIT + HEX
        DB  %00110000  ;32 2   DIGIT + HEX
        DB  %00110000  ;33 3   DIGIT + HEX
        DB  %00110000  ;34 4   DIGIT + HEX
        DB  %00110000  ;35 5   DIGIT + HEX
        DB  %00110000  ;36 6   DIGIT + HEX
        DB  %00110000  ;37 7   DIGIT + HEX
        DB  %00110000  ;38 8   DIGIT + HEX
        DB  %00110000  ;39 9   DIGIT + HEX
        DB  %00001000  ;3A :   PUNCT
        DB  %00001000  ;3B ;   PUNCT
        DB  %00001000  ;3C <   PUNCT
        DB  %00001000  ;3D =   PUNCT
        DB  %00001000  ;3E >   PUNCT
        DB  %00001000  ;3F ?   PUNCT


;char tolwr(char c) - Convert to Lower Case
;  sets Carry if if Converted
tolwr:  JSR isupc   ;If Char is Not Upper Case
        BCC isrts   ;  Return
        ORA $20     ;Else Set Bit 6
        RTS         ; and Return

;char toupr(char c) - Convert to Upper Case
;  sets Carry if if Converted
toupr:  JSR islwr   ;If Char is Not Lower Case
        BCC isrts   ;  Return
        AND $DF     ;Else Clear Bit 6
        RTS         ; and Return


