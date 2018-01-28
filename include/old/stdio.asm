; C02 library stdio.h02 assembly language subroutines
; Requires external routines getkey, prchr, delchr, and newlin
; external zero page locations strlo and strhi
; and external constants DELKEY, ESCKEY, and RTNKEY

;char getchr()
getchr: JMP getkey     ;Call external getkey routine

;void putchr(c)
putchr: JMP prchr      ;Call external print character

;char getstr(&s)
getstr: STX strlo      ;Save Pointer in Store Index
        STY strhi
        LDY #$00       ;Initialize character offset
getstl: JSR getchr     ;Get Keypress
getstd: CMP #DELKEY    ;If Delete
        BNE getste     ;Then  
        TYA            ;  If Offset is Zero
        BEQ getstl     ;    Get Next Character
        DEY            ;  Else Decrement Offset
        JSR delchr     ;    Delete Previous Character
        JMP getstl     ;    and Get Next Character
getste: CMP #ESCKEY    ;Else If Escape
        BNE getstc     ;Then  
        LDY #$FF       ;  Return -1
        BNE getsty     
getstc: CMP #RTNKEY    ;Else If Not Carriage Return 
        BEQ getstx
        JSR putchr     ;  Echo Character
        STA (strlo),Y  ;  Store Character at offset
        INY            ;  increment offset and
        BPL getstl     ;  loop if less than 128
getstx: JSR newlin     ;Else Advance Cursor to Next Line
        LDA #$00       ;  Terminate String        
        STA (strlo),Y  ;  and 
getsty: TYA            ;  Return String Length
        RTS

;char outstr(n, &s)
outstr: LDA #$00       ;Set Start Position to 0
                       ;and fall into outsub
;char outsub(n, &s)
outsub: STX strlo      ;Save pointer in Store Index
        STY strhi
        TAY            ;Initialize character offset
outsul: LDA (strlo),Y  ;Read next character in string
        BEQ outsux     ;If Not 0
        JSR putchr     ;  Print character at offset,
        INY            ;  increment offset, and
        BPL outsul     ;  loop if less than 128
outsux: TAY            ;Return number of
        RTS            ;  characters printed

;char putstr(*s)
putstr: JSR outstr     ;Write string to screen
        JMP newlin     ;Call external newline routine and return

