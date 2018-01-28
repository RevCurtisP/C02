; C02 library stdlib.h02 assembly language subroutines
; Requires 
; external zero page locations strlo and strhi
; and external locations random, temp0, and temp1. 

;Return Absolute Value of Accumulator
abs:    CMP #$80        ;If Negative (High Bit Set)
        BCC absx        ;  Carry will Already be Set
        EOR #$FF        ;  One's Complement
        ADC #$00        ;  and Increment (Carry set by CMP)
absx:   RTS

;Multiply A times Y
;Uses temp0, temp1
;Affects A,N,Z,C
mult:   STA temp0       ;Store Multiplicand
        STY temp1       ;Store Multiplier
;Multiply temp0 times temp1
multt:  LDA #$00        ;Initialize Accumulator
        BEQ multe       ;Enter Loop
multa:  CLC
        ADC temp0       ;Add Multiplicand
multl:  ASL temp0       ;Shift Multiplicand Left
multe:  LSR temp1       ;Shift Multiplier Right
        BCS multa       ;If Bit Shifted Out, Add Multiplicand
        BNE multl       ;Loop if Any 1 Bits Left
        RTS

;Divide A by Y
;Used temp0,temp1
;Affects A,X,N,Z,C
div:    STA temp0       ;Store Dividend
        STY temp1       ;Store Divisor
;Divide temp0 by temp1
divt:   LDA #$00        ;Clear Accumulator
        LDX #$07        ;Load Loop Counter
        CLC
divl:   ROL temp0       ;Shift Bit Out of Dividend
        ROL             ;  into Accumulator
        CMP temp1       ;If Accumulator 
        BCC divs        ;  >= Divisor
        SBC temp1       ;Subtract Divisor
divs:   DEX             ;Decrement Counter
        BPL divl        ;  and Loop
        ROL temp0       ;Shift Result into Dividend
        LDA temp0       ;Load Result into Accumulator
        RTS

;Generate Pseudo-Random Number        
;Uses random
;Affects A,N,Z,C
rand:   LDA random      ;Load Last Result
        BNE randl       ;If Zero
        ADC #$01        ;  Set to 1
randl:  ASL             ;Shift the Seed
        BCC randx       ;If a one was shifted out
        EOR #$1D        ;  Twiddle the bite
randx:  STA random      ;Save the Seed
        RTS

;Seed Pseudo-Random Number Generator
rands:  STA random      ;Store Seed
        RTS

;Convert ASCII to Byte
;Uses temp0, temp1
;Affects A,X,Y,N,Z,C
atoc:   STX strlo       ;Save String Pointer
        STY strhi
        LDY #$00        ;Initialize Index Into String
        STY temp0       ;Initialize Result
atocl:  LDA (strlo),Y   ;Get Next Character
        CMP #$30        ;If Less Than '0'
        BCC atocx       ;  Exit
        CMP #$3A        ;If Greater Than '9'
        BCS atocx       ;  Exit
        AND #$0F        ;Convert to Binary Nybble
        STA temp1       ;  and Save It
        LDA temp0       ;Load Result
        ASL             ;Multiply by 5 by
        ASL             ;  Multiplying by 4
        ADC temp0       ;  And Adding Itself
        ASL             ;Multiply that by 2  
        ADC temp1       ;Add Saved Nybble
        STA temp0       ;  and Store Result
        INY             ;Increment Index
        BPL atocl       ;  and Loop
atocx:  LDA temp0       ;Load Result
        RTS             ;And Return

;Convert Byte to ASCII
;Uses strlo, strhi, temp0, temp1, temp2
;Affects A,X,Y,N,Z
ctoa:   STX strlo      ;Save Pointer in Store Index
        STY strhi
        JSR cvbcd      ;Convert Accumulator to BCD
        LDY #$00       ;Set String Offset to 0
        LDA temp2      ;Get High Byte
        BEQ ctoa1      ;If Not Zero
        JSR ntoa       ;  Convert Low Nybble
ctoa1:  LDA temp1      ;Get Low Byte
        LSR            ;Shift High Nybble
        LSR            ;  into Low Nybble
        LSR
        LSR
        BNE ctoa2      ;If Not Zero
        CMP temp2      ;  and High Byte 
        BEQ ctoa3      ;  not Zero   
ctoa2:  JSR ntoa       ;  Convert It
ctoa3:  LDA temp1      ;Get Low Byte
        JSR ntoa       ;Convert It
        LDA #$00
        BEQ ctoax      ;Terminate String
ntoa:   AND #$0F       ;Strip High Nybble
        ORA #$30       ;Convert to ASCII digit
ctoax   STA (strlo),Y  ;Store in String
        INY            ;and Increment Offset
        RTS  

;Convert Binary Number in Accumulator to Binary Coded Decimal
;Uses temp0
;Returns BCD in temp1,temp and A,X = 0
cvbcd:  STA temp0  ;Save Binary Value
        LDA #0     ;Clear BCD Bytes
        STA temp1
        STA temp2
        LDX #8     ;Process 8 bits of Binary
        SEI        ;Disable Interupts
        SED        ;Set Decimal Mode
cvbcdl: ASL temp0  ;Shift High Bit Into Carry
        LDA temp1  ;Add BCD Low Byte to Itself 
        ADC temp1  ;  Plus Bit Shifted out of Binary
        STA temp1  ;  Effectively Multiplying It by 2
        LDA temp2  ;Add BCD High Byte to Itself
        ADC temp2  ;  Plus Bit Shifted out of Low Byte
        STA temp2  ;  Effectively Multiplying It by 2
        DEX        ;Decrement Counter and
        BNE cvbcdl ;  Process Next Bit
        CLD        ;Clear Decimal Mode
        CLI        ;Enable Interrupts
        RTS

