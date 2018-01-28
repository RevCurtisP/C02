; c02 Program Initialization Code for Unexpanded VIC-20

;System Specific ASCII Key Mappings
DELKEY  EQU $7F   ;Delete/Backspace Key (Delete)
ESCKEY  EQU $03   ;Escape/Stop Key      (RUN/STOP)
RTNKEY  EQU $0D   ;Return/Enter Key     (RETURN)

;Zero Page Locations
;       EQU $00   ;USR JMP
adray1  EQU $03   ;Vector: Convert Floating Point to Signed Integer
adray2  EQU $05   ;Vector: Convert Integet to Floating Point
charac  EQU $07   ;Search Character for Scanning BASIC Text Input
endchr  EQU $08   ;Search Character for Statement Terminator or Quote
trmpos  EQU $09   ;Column Position of Cursor before Last SPC or TAB
verckb  EQU $0A   ;Flag: Load or Verify (BASIC)
count   EQU $0B   ;Index into Text Input Buffer/Number of Array Subscripts
dimflg  EQU $0C   ;Flags for Routines that Locate or Build an Array
valtyp  EQU $0D   ;Type of Data (String or Numeric)
intflg  EQU $0E   ;Type of Numeric Data (Integer or Floating Point)
garbfl  EQU $0F   ;Flag for LIST, Garbage Collection, and Program Terminators
subflg  EQU $10   ;Subscript Reference to Array or User Defined Function Call
inpflg  EQU $11   ;Is Data Input to GET, READ, or INPUT?
tansgn  EQU $12   ;Sign of Result of TAN or SIN Function
channl  EQU $13   ;Current I/O Channel (CMD Logical File) Number
linnum  EQU $14   ;Integer Line Number Value
temppt  EQU $16   ;Pointer to Next Availabale Space in Temporary String Stack
lastpt  EQU $17   ;Pointer to Address of Last String in Temporary String Stack
tempst  EQU $19   ;Descriptor Stack for Temporary Strings
index   EQU $22   ;Miscellaneous Temporary Pointers and Save Area
;           $24
resho   EQU $26   ;Floating Point Multiplication Work Area
txttab  EQU $2B   ;Pointer to Start of BASIC Program Text
vartab  EQU $2D   ;Pointer to Start of BASIC Variable Storage Area
arytab  EQU $2F   ;Pointer to Start of BASIC Array Storage Area
strend  EQU $31   ;Pointer to Start of Free RAM
fretop  EQU $33   ;Pointer to Bottom of String Text Storage Area
frespc  EQU $35   ;Temporary Pointer for Strings
memsiz  EQU $37   ;Pointer to Highest Address Used by BASIC
curlin  EQU $39   ;Current BASIC Line Number
oldlin  EQU $3B   ;Previous BASIC Line Number
oldtxt  EQU $3D   ;Pointer to Address of Current BASIC Statement
datlin  EQU $3F   ;Current DATA Line Number
datptr  EQU $41   ;Pointer to Address of Current DATA Item
inpptr  EQU $43   ;Pointer to Source of GET, READ, or INPUT Information
varnam  EQU $45   ;Current Variable Name
varpnt  EQU $47   ;Pointer to Current BASIC Variable Value
forpnt  EQU $49   ;Temporary Pointer to Index Variable Used by FOR
opptr   EQU $4B   ;Math Operator Table Displacement
opmask  EQU $4D   ;Mask for Comparison Operator
defpnt  EQU $4E   ;Pointer to Current FN Descriptor
dscpnt  EQU $50   ;Temporary Pointer to Current String Descriptor
;           $52   ;current string length
four6   EQU $53   ;Constant for Garbage Collection
jmper   EQU $54   ;Jump to Function Instruction
;           $57   ;BASIC Numeric Work Area
fac1    EQU $61   ;Floating Point Accumulator #1
facexp  EQU $61   ;Floating Point Accumulator #1: Exponent
facho   EQU $62   ;Floating Point Accumulator #1: Mantissa
facsgn  EQU $66   ;Floating Point Accumulator #1: Sign
sgnflg  EQU $67   ;Number of Terms in Series Evaluation
bits    EQU $68   ;Floating Point Accumulator #1: Overflow Digit
fac2    EQU $69   ;Floating Point Accumulator #2
argexp  EQU $69   ;Floating Point Accumulator #2: Exponent
argho   EQU $6E   ;Floating Point Accumulator #2: Mantissa
argsgn  EQU $6F   ;Floating Point Accumulator #2: Sign
facov   EQU $70   ;Low Order Mantissa Byte of Floating Point Accumulator #1
fbufpt  EQU $71   ;Series Evaluation Pointer
chrget  EQU $73   ;Subroutine: Get Next BASIC Text Character
chrgot  EQU $79   ;Entry Point: Get Current BASIC Text Character
txtptr  EQU $7A   ;Pointer to Current BASIC Text Character
pointb  EQU $7C   ;Entry Point: Test Character in Accumulator
exit    EQU $8A   ;RTS at end of CHRGET, CHRGOT, and POINTB Routines
rndx    EQU $8B   ;RND Function Seed Value
status  EQU $90   ;Kernal I/O Status Word
stkey   EQU $91   ;Flag: Was STOP Key Pressed
svxt    EQU $92   ;Timing Constant for Tape Reads
verck   EQU $93   ;Flag: Load or Verify (Kernal)
c3po    EQU $94   ;Flag: Serial Bus - Output Character was Buffered
bsour   EQU $95   ;Buffered Character for Serial Bus
syno    EQU $96   ;Cassette Block Synchronization Number
xsav    EQU $97   ;Temporary .X Register Save Area
ldtnd   EQU $98   ;Number of Open I/O Files, Index to End of File Tables
dfltn   EQU $99   ;Default Input Device (Set to 0 for Keyboard)
dflto   EQU $9A   ;Default Output (CMD) Device (Set to 3 for Screen)
prty    EQU $9B   ;Tape Character Parity
dpsw    EQU $9C   ;Flag: Tape Byte Received
msflg   EQU $9D   ;Flag: Kernal Message Control
ptr1    EQU $9E   ;Tape Pass 1 Error Log Index
ptr2    EQU $9F   ;Tape Pass 2 Error Log Correction Index
time    EQU $A0   ;Software Jiffy Clock
;       EQU $A3   ;Temporary Data Storage Area
cntdn   EQU $A5   ;Cassette Synchronization Character Countdown
bufpnt  EQU $A6   ;Count of Characters in Tape I/O Buffer
inbit   EQU $A7   ;RS-232 Input Bits/Tape I/O Miscellaneous Flag
bitci   EQU $A8   ;RS-232 Input Bit Count/Tape Error Flag
rinone  EQU $A9   ;RS-232 Flag: Check for Start Bit/Tape Character Type Flag
ridata  EQU $AA   ;RS-232 Input Byte Buffer
riprty  EQU $AB   ;RS-232 Input Parity/Cassette Leader Counter
sal     EQU $AC   ;Pointer to Starting Address of Load/Screen Scrolling
eal     EQU $AE   ;Pointer to Ending Address of Load (End of Program)
cmp0    EQU $B0   ;Used to Determine Value of SVXT
;       EQU $B1   ;Used in Timing of Tape Reads
tape1   EQU $B2   ;Pointer: Start of Tape Buffer
bitts   EQU $B4   ;RS-232 Output Bit Count/Tape Load Receive Flag
nxtbit  EQU $B5   ;RS-232 Next Bit to Send/Tape EOT Flag
rodata  EQU $B6   ;RS-232 Output Byte Buffer
fnlen   EQU $B7   ;Length of Current Filename
la      EQU $B8   ;Current Logical File Number
sa      EQU $B9   ;Current Secondary Address
fa      EQU $BA   ;Current Device Number
fnadr   EQU $BB   ;Pointer: Current Filename
roprt   EQU $BD   ;RS-232 Output Parity/Tape Character Being Read or Sent
fsblk   EQU $BE   ;Cassette Read/Write Block Count
mych    EQU $BF   ;Tape Input Byte Buffer
cas1    EQU $C0   ;Tape Motor Interlock
stal    EQU $C1   ;I/O Start Address
memuss  EQU $C3   ;Tape Load Temporary Address
lste    EQU $C5   ;Matrix Coordinate of Last Key Pressed
ndx     EQU $C6   ;Number of Characters in Keyboard Buffer
rvs     EQU $C7   ;Flag: Print Reverse Characters
indx    EQU $C8   ;Pointer: End of Logical Line for Input
lxsp    EQU $C9   ;Cursor X.Y Position at Start of Input
sfdx    EQU $CB   ;Matrix Coordinate of Current Key Pressed
blnsw   EQU $CC   ;Cursor Blink Enable
blnct   EQU $CD   ;Timer Countdown to Blink Cursor   
gdbln   EQU $CE   ;Character Under Cursor
blnon   EQU $CF   ;Flag: Was Last Cursor Blink On or Off
crsw    EQU $D0   ;Flag: Input from Keyboard or Screen
pnt     EQU $D1   ;Pointer to Address of Current Screen Line
pntr    EQU $D3   ;Cursor Column on Current Line
qtsw    EQU $D4   ;Flag: Editor in Quote Mode?
lnmx    EQU $D5   ;Maximum Length of Physical Screen Line
tblx    EQU $D6   ;Current Cursor Physical Line Number
;       EQU $D7   ;Temporary Storage for ASCII Value of Last Character Printed
insrt   EQU $D8   ;Insert Mode (Number of Inserts)
ldtb1   EQU $D9   ;Screen Line Link Table
user    EQU $F3   ;Pointer to Address of Current Screen Color RAM Location
keytab  EQU $F5   ;Vector: Keyboard Decode Table
ribuf   EQU $F7   ;Pointer: RS-232 Input Buffer
robuf   EQU $F8   ;Pointer: RS-232 Output Buffer
frekzp  EQU $FB   ;Four Free Bytes of Zero Page for User Programs
baszpt  EQU $FF   ;BASIC Temporary Data for Floating Point to ASCII Conversion

;Page 1 Locations
bad     EQU $0100 ;Tape Input Error Log
;       EQU $013F ;Microprocessor Stack

;Basic and Kernal Working Storage
buf     EQU $0200 ;BASIC Line Editor Input Buffer
lat     EQU $0259 ;Kernal Table of Active Logical File Numbers
fat     EQU $0263 ;Kernal Table of Device Numbers for Each Logical File
sat     EQU $026D ;Kernal Table of Secondary Addressed for Each Logical File
keyd    EQU $0277 ;Keyboard Buffer (Queue)
memstr  EQU $0281 ;Pointer: Operating System Start of Memory
memsiz  EQU $0283 ;Pointer: Operating System End of Memory
timout  EQU $0285 ;Flag: Kernal Variable for IEEE Timeout
color   EQU $0286 ;Current Foreground Color Text
gdcol   EQU $0287 ;Color of Character Under Cursor
hibase  EQU $0288 ;Top Page of Screen Memory
xmax    EQU $0289 ;Maximum Keyboard Buffer Size
rptflg  EQU $028A ;Flag: Which Key Will Repeat?
kount   EQU $028B ;Counter for Timing Delay Between Key Repeats
delay   EQU $028C ;Counter for Timing Delay Until First Key Repeat Begins
shflag  EQU $028D ;Flag: SHIFT/CTRL/Logo Keypress
lstshf  EQU $028E ;Last Pattern SHIFT/CTRL/Logo Keypress
keylog  EQU $028F ;Vector to Keyboard Table Setup Routine
mode    EQU $0291 ;Flag: Change Character Sets with SHIFT/Logo Keypress
autodn  EQU $0292 ;Flag: Screen Scroll Enabled
m51ctr  EQU $0293 ;RS-232: Mock 6551 Control Register
m51cdr  EQU $0294 ;RS-232: Mock 6551 Command Register
m51adj  EQU $0295 ;RS-232: Non-Standard Bit Timing (Not Implemented)
rsstat  EQU $0297 ;RS-232: Mock 6551 Status Register
bitnum  EQU $0298 ;RS-232: Number of Bits Left to be Sent/Received
baudof  EQU $0299 ;RS-232: Time Required to Send a Bit
ridbe   EQU $029B ;RS-232: Index to End of Receive Buffer
ridbs   EQU $029C ;RS-232: Index to Start of Receive Buffer
rodbs   EQU $029D ;RS-232: Index to Start of Transmit Buffer
rodbe   EQU $029E ;RS-232: Index to End of Transmit Buffer
irqtmp  EQU $029F ;Save Area for IRQ Vector During Cassette I/O
enabl   EQU $02A1 ;RS-232 Interrupts Enabled
;       EQU $02A2 ;CIA #1 Control Register B Activity During Cassette I/O
;       EQU $02A3 ;Save Area for CIA #1 Interrupt Control Register
;       EQU $02A4 ;Save Area for CIA #1 Control Register A
;       EQU $02A5 ;Temporary Index to Next Line for Screen Scrolling
;       EQU $02A6 ;PAL/NTSC Flag 
;       EQU $02A7 ;Unused
;       EQU $02C0 ;Sprite 11  

;BASIC Indirect Vector Table
ierror  EQU $0300 ;Vector to Print BASIC Error Message Routine
imain   EQU $0302 ;Vector to Main BASIC Program Loop
icrnch  EQU $0304 ;Vector to Crunch ASCII Keyword into Tokens
iqplop  EQU $0306 ;Vector to List BASIC Token as ASCII Text
igone   EQU $0308 ;Vector to Execute Next BASIC Token
ieval   EQU $030A ;Vector to Evaluate Single-Term Arithmetic Expression

;Register Storage Area
sareg   EQU $030C ;Storage Area for .A Register (Accumulator)
sxreg   EQU $030D ;Storage Area for .X Index Register
syreg   EQU $030E ;Storage Area for .Y Index Register
spreg   EQU $030F ;Storage Area for .P (Status) Register

;Miscellaneous Vectors
usrpok  EQU $0310 ;JMP Instruction for User Function
usradd  EQU $0311 ;Address of USR Routine
;       EQU $0313 ;Unused
cinv    EQU $0314 ;Vector to IRQ Interrupt Routine
cbinv   EQU $0316 ;Vector: BRK Instruction Interrupt
nminv   EQU $0318 ;Vector: Non-Maskabke Interrupt

;Kernal Indirect Vectors
iopen   EQU $031A ;Vector to Kernal OPEN Routine
iclose  EQU $031C ;Vector to Kernal CLOSE Routine
ichkin  EQU $031E ;Vector to Kernal CHKIN Routine
ickout  EQU $0320 ;Vector to Kernal CKOUT Routine
iclrch  EQU $0322 ;Vector to Kernal CLRCHN Routine
ibasin  EQU $0324 ;Vector to Kernal CHRIN Routine
ibsout  EQU $0326 ;Vector to Kernal CHROUT Routine
istop   EQU $0328 ;Vector to Kernal STOP Routine
igetin  EQU $032A ;Vector to Kernal GETIN Routine
iclall  EQU $032C ;Vector to Kernal CLALL Routine
usrcmd  EQU $032E ;Vector to User Defined Command (Unused)
iload   EQU $0330 ;Vector to Kernal LOAD Routine
isave   EQU $0332 ;Vector to Kernal SAVE Routine

;           $0334 ;Unused
tbuffer EQU $033C ;Cassette I/O Buffer
;           $03FC ;Unused

vicscn  EQU $0400 ;Video Screen Memory Area
;           $07F8 ;Sprite Shape Data Pointers
;           $0800 ;BASIC Program Text  
;           $8000 ;Autostart ROM Cartridge

vicclr  EQU $D800 ;Color RAM

;Kernal Routines
chrin   EQU $FFCF ;Input Character to Channel
chrout  EQU $FFD2 ;Output Character to Channel
getin   EQU $FFE4 ;Read Character from Keyboard Buffer

;Machine Language Basic Stub
        ORG $1001             ;Start Directly Above Stack
basic:  DC  $0C, $10           ; Pointer to Next Line (4109)
        DC  $00, $00           ; Line Number (0)
        DC  $9E                ; SYS 
        DC  $20                ; ' '
        DC  $34, $31, $31 ,$30 ; "4110" 
        DC  $00                ;End of Line Marker
        DC  $00, $00           ;End of Basic Program

        JMP main    ;Execute Program

;Poll Keyboard for Character
plkey   EQU getin   ;Read Character from Keyboard Buffer

;Read Character from Console


;Wait for Character from Console
getkey: JSR plkey   ;Poll Keyboard
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

