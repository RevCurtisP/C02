; c02 Program Initialization Code for Unexpanded VIC-20

;ASCII Control Codes Equivalents
CR      EQU $0D   ;Carriage Return
LF      EQU $11   ;Line Feed (Cursor Down)
DEL     EQU $14   ;Delete
HT      EQU $1D   ;Horizontal Tab (Cursor Right)
VT      EQU $91   ;Vertical Tab (Cursor Up)
FF      EQU $93   ;Form Feed (Clear Screen)
BS      EQU $9D   ;Backspace (Cursor Left)

;PETSCII Key Mappings
DELKEY  EQU $14   ;Delete/Backspace Key (Delete)
ESCKEY  EQU $03   ;Escape/Stop Key      (RUN/STOP)
RTNKEY  EQU $0D   ;Return/Enter Key     (RETURN)

;Zero Page Locations
strlo   EQU $FB   ;String Pointer (stdio.asm)
strhi   EQU $FC   ;Free Byte for User Programs
usrzp3  EQU $FD   ;Free Byte for User Programs
usrzp4  EQU $FE   ;Free Byte for User Programs

;Other RAM Locations
user0   EQU $0310 ;Free Byte for User Programs
user1   EQU $0311 ;Free Byte for User Programs
user2   EQU $0312 ;Free Byte for User Programs
user3   EQU $0313 ;Free Byte for User Programs
user4   EQU $0334 ;Free Byte for User Programs
user5   EQU $0335 ;Free Byte for User Programs
user6   EQU $0336 ;Free Byte for User Programs
user7   EQU $0337 ;Free Byte for User Programs
user8   EQU $0338 ;Free Byte for User Programs
user9   EQU $0339 ;Free Byte for User Programs
user10  EQU $033A ;Free Byte for User Programs
user11  EQU $033B ;Free Byte for User Programs
tbffr   EQU $033C ;Cassette I/O Buffer
user12  EQU $03FC ;Free Byte for User Programs
user13  EQU $03FD ;Free Byte for User Programs
user14  EQU $03FE ;Free Byte for User Programs
user15  EQU $03FF ;Free Byte for User Programs

;Video RAM and ROM 
vicscn  EQU $1E00 ;Video Screen Memory Area (Unexpanded)
chrrom  EQU $8000 ;Character Generator ROM
vicclr  EQU $9600 ;Color RAM (Unexpanded)

;Kernal Routines
chrin   EQU $FFCF ;Input Character to Channel
chrout  EQU $FFD2 ;Output Character to Channel
getin   EQU $FFE4 ;Read Character from Keyboard Buffer

;Machine Language Basic Stub
        ORG $1001              ;Start 
basic:  DC  $0C, $10           ; Pointer to Next Line (4108)
        DC  $00, $00           ; Line Number (0)
        DC  $9E                ; SYS 
        DC  $20                ; ' '
        DC  $34, $31, $31 ,$30 ; "4110" 
        DC  $00                ;End of Line Marker
        DC  $00, $00           ;End of Basic Program

start:  TSX         ;Get Stack Pointer
        STX user15  ;and Save for Exit
        JMP main    ;Execute Program

exit:   LDX user15  ;Retrieve Saved Stack Pointer
        TXS         ;and Restore It
        RTS         ;Return to BASIC

;Poll Keyboard for Character
plkey   EQU getin   ;Read Character from Keyboard Buffer

;Get Character from Keyboard
getkey: 

;Wait for Character from Keyboard
rdkey:  JSR plkey   ;Poll Keyboard
        BEQ getkey  ;If No Key, Loop
        RTS

;Delete Previous Character
delchr: RTS

;Advance Character to Next line
newlin: LDA #$0D     ;Load C/R into Accumulator
        
;Print Character to Console
prchr   EQU chrout   ;

;Delete Previous Character
delchr: LDA #$9D     ;Load Cursor Left into Accumulator
        JSR prchr    ;  and Print it
        LDA #$14     ;Load Delete into Accumulater
        JMP prchr    ;  and Print it

;Advance Character to Next line
newlin: LDA #$0D     ;Load C/R into Accumulator
        JMP prchr    ;  and Print it

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
prhex:  AND #$0F    ;Strip High Nybble
        CMP #$0A    ;If Low Nybble >= 10
        BCC prhexc  ;  
        ADC #$06    ;  Convert ':' to 'A'...
prhexc: ADC #$30    ;Convert to ASCII Character
        JMP prchr   ;Print Hex Digit and Return

exit:   RTS             ;Return to Monitor

