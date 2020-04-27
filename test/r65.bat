@ECHO OFF
IF EXIST %1.c02 GOTO COMPILE
    ECHO File %1.c02 not found
    GOTO EOF

:COMPILE
@ECHO Compiling File %1.c02 for run6502
..\c02.exe -h run6502 -s run6502 %1 >%1.dbg

IF %ERRORLEVEL% NEQ 0 GOTO EOF

@ECHO Assembling File %1.asm
..\a02 %1.asm %1.bin %1.lst

@ECHO Executing file %1.bin 
..\emu\run6502 -l 0200 %1.bin -G FFE0 -P FFE3 -N 0000 -R 0200 -X 0

:EOF
