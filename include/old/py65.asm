; py65mon program initialization code for c02 programs

;System Specific ASCII Key Mappings
DELKEY  EQU $7F     ;Delete/Backspace Key (Delete)
ESCKEY  EQU $1B     ;Escape/Stop Key      (Escape)
RTNKEY  EQU $0D     ;Return/Enter Key     (Carriage Return)

;Zero Page Locations
strlo   EQU $30     ;String Pointer (stdio.asm)
strhi   EQU $31
dstlo   EQU $32     ;Source String Pointer (string.asm)
dsthi   EQU $33

rdseed  EQU $3E     ;Pseudo-Random Seed
random  EQU $3F     ;Pseudo-Random Number Storage
temp0   EQU $40     ;Temporary Storage
temp1   EQU $41
temp2   EQU $42

;Memory Mapped I/O 
putc    EQU $F001   ;Write Character to Console
getc    EQU $F004   ;Read Character from Console

        ORG $0200   ;Start Directly Above Stack

start:  JMP main    ;Execute Program

;Poll Character from Keyboard
plkey:  INC rdseed  ;Cycle the Random Seed (if not provided by system)
        LDA getc    ;Read Character from Console
        RTS

;Read Character from Console
getkey;             ;Same As rdkey

;Wait for Character from Console
rdkey:  JSR plkey   ;Read Character from Console
        BEQ rdkey   ;  Loop if None Received
        RTS

;Delete Previous Character
delchr: LDA #$08    ;Load Backspace into Accumulator
        JSR prchr   ;  and Print it
        LDA #$20    ;Load Space into Accumulater
        JSR prchr   ;  and Print it
        LDA #$08    ;Load Backspace into Accumulator
        JMP prchr   ;  and Print it

;Advance Character to Next line
newlin: LDA #$0D    ;Load C/R into Accumulator
        JSR prchr   ;  and Print it
        LDA #$0A    ;Load L/F into Accumulater
                    ;  and fall into prchr

;Print Character to Console
prchr:  STA putc   ;Write Character to Console
        RTS

exit:   BRK        ;Return to Monitor

;Print Byte as Two-Digit Hex Number to Console
prbyte: PHA        ;Save Accumulater
        LSR        ;Shift Hi Nybble to Low Nybble
        LSR 
        LSR 
        LSR 
        JSR prhex  ;  and Print it
        PLA        ;Restore Accumulator 
                   ;  and fall into prhex
                    
;Print Low Nybble as Hex Digit to Console
prhex:  AND #$0F   ;Strip High Nybb
        SED        ;Set Decimal Flag for 
        CLC        ;  Addition Wizardry
        ADC #$90   ;Convert to $90-$99,$00-$05
        ADC #$40   ;Convert to $30-$39,$41-$46
        CLD        ;Clear Decimal Flag
        JMP prchr  ;Print Hex Digit and Return


