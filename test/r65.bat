@ECHO OFF
IF EXIST %1.c02 GOTO COMPILE
    ECHO File %1.c02 not found
    GOTO EOF

:COMPILE
@ECHO Compiling File %1.c02 for run6502
..\c02.exe -d -h run6502 -s run6502 %1 >%1.dbg

IF %ERRORLEVEL% NEQ 0 GOTO EOF

@ECHO Assembling File %1.asm
..\a02 -d -i "..\include ..\include\run6502" %1.asm %1.bin %1.lst >%1.out

@ECHO Executing file %1.bin 
..\lib6502\run6502 -D -l 0400 %1.bin -K FFE0 -C FFE3 -F FFE6 -S FFE9 -M FFEA -N 0000 -R 0400 -X 0 2>%1.err

:EOF
