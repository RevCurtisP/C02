@REM Compile and Run C02 Program for C64
@ECHO OFF

IF EXIST %1.c02 GOTO COMPILE
ECHO File %1.c02 not found
GOTO EOF

:COMPILE
ECHO Compiling File %1.c02 for Commodore 64
..\c02.exe -d -h c64 -s c64 -s cbm %1 >%1.dbg
IF %ERRORLEVEL% NEQ 0 GOTO EOF

ECHO Assembling File %1.asm
..\a02.exe -d -p -i "../include ../include/c64" %1.asm %1.prg %1.lst >%1.out
IF %ERRORLEVEL% NEQ 0 GOTO EOF

ECHO Starting Emulator
start C:\Programs\WinVICE\x64.exe -config x64.ini %1.prg

:EOF
