; Program initialization code for C02 programs
; Template for System Specific Code

;System Specific ASCII Key Mappings
DELKEY  EQU $7F     ;Delete/Backspace Key ($08=Backspace, $7F=Delete)
ESCKEY  EQU $1B     ;Escape/Stop Key      ($03=Ctrl-C, $1B=Escape)
RTNKEY  EQU $0D     ;Return/Enter Key     ($0D=Carriage Return)
NULKEY  EQU $00     ;No Key was Pressed   ($00=Null)

;Zero Page Locations
strlo   EQU $00     ;String Pointer (stdio.asm and string.asm)
strhi   EQU $01
strlo   EQU $02     ;Source String Pointer (string.asm)
strhi   EQU $04

rdseed  EQU $0E     ;Pseudo-Random Seed
random  EQU $0F     ;Pseudo-Random Number Storage
temp0   EQU $10     ;Temporary Storage
temp1   EQU $11     ;

        ORG $0200   ;Program Start Address

start:  NOP         ;System specific initialization code
        TXS         ;If an RTS is used to return to the Operating System,
        STX stksav  ;  the Stack Pointer should be preserved
        JMP main    ;Execute Program

;Read Character from Console
getkey;             ;Usually Drops into rdkey, but may need to call rdkey
                    ;  then clean/convert returned value (e.g. Apple-1) 

;Poll Character from Keyboard
plkey:  INC rdseed  ;Cycle the Random Seed (if not provided by system)
        NOP         ;Code Read from Keyboad         
        RTS

;Wait for Character from Console
rdkey:  JSR plkey   ;Usually calls plkey 
        BEQ rdkey   ;  until a non-zero is returned
        RTS         

;Delete Previous Character
delchr: RTS         ;Code to move Cursor to left and clear that position
                    ;May work differently on systems that don't support this

;Advance Cursor to Next line
newlin: RTS         ;Code to move Cursor to beginning of next line
                    ;May emit Carriage Return, Line Feed, or both

;Print Character to Screen
prchr:  RTS         ;Code to write ASCII character to Screen

;Exit Program and Return to Operating System or Monitor
exit:   BRK         ;Usually BRK if returning to Monitor
        LDX stksav  ;If an RTS is used to return to the Operating System,
        TXS         ;  the Stack Pointer should be restored to original state 
        RTS         ;  in case program exits in the middle of a Function

;Note: The following two functions replicate calls available 
;in the Apple-1 monitor and are included for test purposes
;They will likely be removed before the final release

;Print Byte as Two-Digit Hex Number to Console
prbyte: PHA         ;Save Accumulater
        LSR         ;Shift Hi Nybble to Low Nybble
        LSR 
        LSR 
        LSR 
        JSR prhex   ;  and Print it
        PLA         ;Restore Accumulator 
                    ;  and fall into prhex

;Print Low Nybble as Hex Digit to Console
prhex:  AND #$0F    ;Strip High Nybb
        SED         ;Set Decimal Flag for 
        CLC         ;  Addition Wizardry
        ADC #$90    ;Convert to $90-$99,$00-$05
        ADC #$40    ;Convert to $30-$39,$41-$46
        CLD         ;Clear Decimal Flag
        JMP prchr   ;Print Hex Digit and Return   

;Alternate Code for Systems with Interrupts that don't CLD
prhex:  AND #$0F    ;Strip High Nybble
        CMP #$0A    ;If Low Nybble >= 10
        BCC prhexc  ;  
        ADC #$06    ;  Convert ':' to 'A'...
prhexc: ADC #$30    ;Convert to ASCII Character
        JMP prchr   ;Print Hex Digit and Return

