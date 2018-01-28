; c02 Program Initialization Code for Vic-20 with at 8K Expansion

;System Specific ASCII Key Mappings
DELKEY  EQU $7F   ;Delete/Backspace Key (Delete)
ESCKEY  EQU $03   ;Escape/Stop Key      (RUN/STOP)
RTNKEY  EQU $0D   ;Return/Enter Key     (RETURN)

;Zero Page Locations
usradd  EQU $01   ;Address of USR Routine
linnum  EQU $14   ;Integer Line Number Value
txttab  EQU $2B   ;Pointer to Start of BASIC Program Text
vartab  EQU $2D   ;Pointer to Start of BASIC Variable Storage Area
arytab  EQU $2F   ;Pointer to Start of BASIC Array Storage Area
strend  EQU $31   ;Pointer to Start of Free RAM
fretop  EQU $33   ;Pointer to Bottom of String Text Storage Area
memsiz  EQU $37   ;Pointer to Highest Address Used by BASIC
inpptr  EQU $43   ;Pointer to Source of GET, READ, or INPUT Information
facexp  EQU $61   ;Floating Point Accumulator #1: Exponent
facho   EQU $62   ;Floating Point Accumulator #1: Mantissa
facsgn  EQU $66   ;Floating Point Accumulator #1: Sign
fac2    EQU $69   ;Floating Point Accumulator #2
argexp  EQU $69   ;Floating Point Accumulator #2: Exponent
argho   EQU $6E   ;Floating Point Accumulator #2: Mantissa
argsgn  EQU $6F   ;Floating Point Accumulator #2: Sign
chrget  EQU $73   ;Subroutine: Get Next BASIC Text Character
chrgot  EQU $79   ;Entry Point: Get Current BASIC Text Character
txtptr  EQU $7A   ;Pointer to Current BASIC Text Character
pointb  EQU $7C   ;Entry Point: Test Character in Accumulator
exit    EQU $8A   ;RTS at end of CHRGET, CHRGOT, and POINTB Routines
status  EQU $90   ;Kernal I/O Status Word
ldtnd   EQU $98   ;Number of Open I/O Files, Index to End of File Tables
dfltn   EQU $99   ;Default Input Device (Set to 0 for Keyboard)
dflto   EQU $9A   ;Default Output (CMD) Device (Set to 3 for Screen)
time    EQU $A0   ;Software Jiffy Clock
tape1   EQU $B2   ;Pointer: Start of Tape Buffer
fnlen   EQU $B7   ;Length of Current Filename
la      EQU $B8   ;Current Logical File Number
sa      EQU $B9   ;Current Secondary Address
fa      EQU $BA   ;Current Device Number
fnadr   EQU $BB   ;Pointer: Current Filename
lste    EQU $C5   ;Matrix Coordinate of Last Key Pressed
ndx     EQU $C6   ;Number of Characters in Keyboard Buffer
rvs     EQU $C7   ;Flag: Print Reverse Characters
sfdx    EQU $CB   ;Matrix Coordinate of Current Key Pressed
pnt     EQU $D1   ;Pointer to Address of Current Screen Line
pntr    EQU $D3   ;Cursor Column on Current Line
lnmx    EQU $D5   ;Maximum Length of Physical Screen Line
tblx    EQU $D6   ;Current Cursor Physical Line Number
insrt   EQU $D8   ;Insert Mode (Number of Inserts)
ldtb1   EQU $D9   ;Screen Line Link Table
pntc    EQU $F3   ;Pointer to Address of Current Screen Color RAM Location
usrzp1  EQU $FB   ;Free Byte for User Programs
usrzp2  EQU $FC   ;Free Byte for User Programs
usrzp3  EQU $FD   ;Free Byte for User Programs
usrzp4  EQU $FE   ;Free Byte for User Programs

;Basic and Kernal Working Storage
buf     EQU $0200 ;BASIC Line Editor Input Buffer
lat     EQU $0259 ;Kernal Table of Active Logical File Numbers
fat     EQU $0263 ;Kernal Table of Device Numbers for Each Logical File
sat     EQU $026D ;Kernal Table of Secondary Addressed for Each Logical File
keyd    EQU $0277 ;Keyboard Buffer (Queue)
memstr  EQU $0281 ;Pointer: Operating System Start of Memory
memsiz  EQU $0283 ;Pointer: Operating System End of Memory
color   EQU $0286 ;Current Foreground Color Text
hibase  EQU $0288 ;Top Page of Screen Memory
xmax    EQU $0289 ;Maximum Keyboard Buffer Size
rptflg  EQU $028A ;Flag: Which Key Will Repeat?
kount   EQU $028B ;Counter for Timing Delay Between Key Repeats
shflag  EQU $028D ;Flag: SHIFT/CTRL/Logo Keypress
mode    EQU $0291 ;Flag: Change Character Sets with SHIFT/Logo Keypress

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
vicscn  EQU $1000 ;Video Screen Memory Area (Unexpanded)
chrrom  EQU $8000 ;Character Generator ROM
vicclr  EQU $9400 ;Color RAM (Unexpanded)

;Kernal Routines
chrin   EQU $FFCF ;Input Character to Channel
chrout  EQU $FFD2 ;Output Character to Channel
getin   EQU $FFE4 ;Read Character from Keyboard Buffer

;Machine Language Basic Stub
        ORG $1201              ;Start 
basic:  DC  $0C, $12           ; Pointer to Next Line (4108)
        DC  $00, $00           ; Line Number (0)
        DC  $9E                ; SYS 
        DC  $20                ; ' '
        DC  $34, $36, $32 ,$32 ; "4622" 
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

