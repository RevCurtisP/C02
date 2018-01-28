; C02 library string.h02 assembly language subroutines
; Requires the following RAM locations be defined
; zero page: strlo,strhi, and srclo,srchi
; anywhere:  temp0 and temp0

;Internal Call - Initialize Source String
;Args: X,Y = Pointer to source string
;Sets: strlo,strhi = Pointer to source string
;Affects: N,Z
;Returns: Y = #$00
strini: STX strlo       ;Save String Pointer
        STY strhi
        LDY #$00        ;Initialize Index Into String
        RTS

;strcmp(&s) - Compare String (to Destination String)
;Requires: dstlo, dsthi - Pointer to destination string
;Args: X,Y = Pointer to source string
;Affects N,Z
;Returns A=$01 and C=1 if Destination > Source  
;        A=$00 and Z=1, C=1 if Destination = Source  
;        A=$FF and C=0 if Destination < Source
;        Y=Position of first character that differs
strcmp: JSR strini      ;Initialize Source String
strcml: LDA (dstlo),Y   ;Load Destination Character
        CMP (strlo),Y   ;Compare Against Source Character
        BNE strcmx;     ;If Equal
        ORA (strlo),Y   ;OR with Source Character
        BEQ strcmr      ;  If Both are 0, Return 0
        INY             ;Increment Offset
        BPL strcml      ;  and Loop if < 128
strcmx: BCC strclx      ;If Source < Destination 
        LDA #$01        ;Else Return 1
strcmr: RTS             ;  

;strchr(c, &s) - Find Character in String
;Args: A = Character to look for
;      X,Y = Pointer to string to search in
;Sets: strlo,strhi = Pointer to string 
;      temp0 = Character being searched for
;Affects: N,Z
;Returns: A = Position in string, C=1 if found
;         A = $FF, C=0 if not found
;         Y = Position of last character scanned
strchr: JSR strini      ;Initialize Source String
        STA temp0;      ;Save Character
chrstl: LDA (strlo),Y   ;Get Next Character
        BEQ strclc      ;If NUL, Return $FF and Carry Clear 
        CMP temp0       ;Compare Character
        BEQ strlex      ;If Found, Return Index
        INY             ;Increment Counter
        BPL chrstl      ;  and Loop if < 128
                        ;Else Return $FF and Carry Clear
strclc: CLC             ;Clear Carry
strclx: LDA #$FF        ;Load -1 into Accumulater
        RTS             ;and Return

;strlen(&s) - Return Length of String
;Args: X,Y - Pointer to string
;Sets: strlo,strhi = Pointer to source string
;Affects: N,Z
;Returns: A,Y = Length of string
strlen: JSR strini      ;Initialize Source String
strlel: LDA (strlo),Y   ;Get Next Character
        BEQ strlex      ;If <> NUL 
        INY             ;  Increment Index
        BPL strlel      ;  and Loop if < 128
strlex: TYA             ;Transfer Index to Accumulator
        RTS             ;and Return

;strset(&s) - Set Destination String 
;             Called before strcat(), strcmp(), strcpy(), strstr()
;Args: X,Y = Pointer to destination string
;Sets: strlo,strhi = Pointer to destination string
;Affects: N,Z
strset: STX dstlo       ;Save String Pointer
        STY dsthi
        RTS

;strcat(&s) Concatenate String (to Destination String)
;Requires: dstlo, dsthi - Pointer to destination string
;Args: X,Y = Pointer to source string
;Sets: strlo,strhi = Pointer to source string
;      temp0 = Length of source prior to concatenation
;Affects: C,N,Z
;Returns: A,Y = Total length of concatenated string
strcat: JSR strini      ;Initialize Source String
strcal: LDA (dstlo),Y   ;Find end of Destination String
        BEQ strcax      ;
        INY             ;
        BPL strcal      ;
strcax: STY temp0       ;Subtract Destination String Length
        LDA strlo       ;  from Source String Pointer
        SEC
        SBC temp0
        STA strlo
        LDA strhi
        SBC #$00
        STA strhi
        JMP strcpl      ;Execute String Copy

;strcpy(&s) - Copy String (to Destination String)
;Requires: dstlo, dsthi - Pointer to destination string
;Args: X,Y = Pointer to source string
;Sets: strlo,strhi = Pointer to source string
;Affects: N,Z
;Returns: A,Y = Number of characters copies
strcpy: JSR strini      ;Initialize Source String
strcpl: LDA (strlo),Y   ;Get Character from Source String
        STA (dstlo),Y   ;Copy to Destination String
        BEQ strcpx      ;If <> NUL 
        INY             ;  Increment Index
        BPL strcpl      ;  and Loop if < 128
strcpx: TYA             ;Transfer Index to Accumulator
        RTS             ;and Return

;strcut(n, &s) - Copy from Position n to End of Source (into Destination)
;Requires: dstlo, dsthi - Pointer to destination string
;Args: A = Starting position in start string
;      X,Y = Pointer to source string
;Sets: strlo,strhi = Pointer to specified position source string
;Affects: N,Z
;Returns: A,Y = Length of copied string
strcut: JSR strini      ;Initialize Source String
        CLC
        ADC strlo       ;Move Source Pointer
        STA strlo       ;  to Specified Position in String
        BCC strcpl
        INC strhi
        JMP strcpl      ;and Jump Into String Copy Loop

;strstr(&s) - Search for String (in Destination String)
;Requires: dstlo, dsthi - Pointer to destination string
;Args: X,Y = Pointer to source string
;Sets: dstlo,dsthi = Pointer to position in source string
;                    End of string if not found 
;      strlo,strhi = Pointer to source string
;      temp0 = Last position checked in destination string
;Affects: N,Z
;Returns: A = Position, C=1 if found
;         A = $FF, C=0 if not found
;         Y = Last position checked in source string      
strstr: JSR strini      ;Initialize Source String
        STY temp0       ;Initialize Position
strstl: LDY #$00;       ;Initialize Compare Offset
        LDA (dstlo),Y   ;Get Start Character in Destination
        BEQ strclc      ;If NUL return $FF and Carry Clear
        JSR strcml      ;Jump into Compare Loop
        BEQ strstx      ;If Not Equal
        BMI strstn      ;  If Source is Greater
        LDA (strlo),Y   ;     If at End of Source String 
        BEQ strstx      ;     Return Current Position
strstn: INC temp0       ;  Else Increment Position
        BMI strclc      ;    If > 127 return $FF and Carry Clear
        INC dstlo       ;    Increment Source Pointer
        BNE strstl
        INC dsthi       ;    If not End of Memory
        BNE strstl      ;      Loop
        BEQ strclc      ;    Else return $FF and Carry Clear
strstx: SEC             ;Else Set Carry
        LDA temp0       ;  Load Position
        RTS             ;  and Return

