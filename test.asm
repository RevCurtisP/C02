;Program test.c02
        CPU 6502
        LDA #0
        SEC 
        SBC 1
        CLC 
        ADC 2
        SEC 
        SBC 3
        STA b
        LDA 4
        AND 255
        STA c
        LDA 1
        ORA 256
        STA i
b       DB  0
c       DB  0
