            processor 6502
            include "vcs.h"
            include "macro.h"
            SEG
            ORG $F000
Reset
StartOfFrame
   ; Start of vertical blank processing
            lda #0
            sta VBLANK
            lda #2
            sta VSYNC
            
               ; 3 scanlines of VSYNCH signal...
                sta WSYNC
                sta WSYNC
                sta WSYNC
            lda #0
            sta VSYNC           
               ; 37 scanlines of vertical blank...
                sta WSYNC
                sta WSYNC
                sta WSYNC
                sta WSYNC
                sta WSYNC
                sta WSYNC
                sta WSYNC
                sta WSYNC
                sta WSYNC
                sta WSYNC
                sta WSYNC
                sta WSYNC
                sta WSYNC
                sta WSYNC
                sta WSYNC
                sta WSYNC
                sta WSYNC
                sta WSYNC
                sta WSYNC
                sta WSYNC
                sta WSYNC
                sta WSYNC
                sta WSYNC
                sta WSYNC
                sta WSYNC
                sta WSYNC
                sta WSYNC
                sta WSYNC
                sta WSYNC
                sta WSYNC
                sta WSYNC
                sta WSYNC
                sta WSYNC
                sta WSYNC
                sta WSYNC
                sta WSYNC
                sta WSYNC
            
               ; 192 scanlines of picture...
 
                ldx #0
                REPEAT 192; scanlines
                    inx
                    stx COLUBK
                    sta WSYNC
                REPEND
 
            lda #%01000010
            sta VBLANK                     ; end of screen - enter blanking
               ; 30 scanlines of overscan...
                sta WSYNC
                sta WSYNC
                sta WSYNC
                sta WSYNC
                sta WSYNC
                sta WSYNC
                sta WSYNC
                sta WSYNC
                sta WSYNC
                sta WSYNC
                sta WSYNC
                sta WSYNC
                sta WSYNC
                sta WSYNC
                sta WSYNC
                sta WSYNC
                sta WSYNC
                sta WSYNC
                sta WSYNC
                sta WSYNC
                sta WSYNC
                sta WSYNC
                sta WSYNC
                sta WSYNC
                sta WSYNC
                sta WSYNC
                sta WSYNC
                sta WSYNC
                sta WSYNC
                sta WSYNC
            jmp StartOfFrame
            ORG $FFFA
            .word Reset          ; NMI
            .word Reset          ; RESET
            .word Reset          ; IRQ
    	END
