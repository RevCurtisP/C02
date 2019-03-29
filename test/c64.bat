@ECHO OFF
IF EXIST %1.c02 GOTO COMPILE
    ECHO File %1.c02 not found
    GOTO EOF

:COMPILE
ECHO Compiling File %1.c02 for Commodore 64
..\c02.exe -h c64 -s c64 -s cbm %1 >%1.dbg
IF %ERRORLEVEL% NEQ 0 GOTO EOF
ECHO Assembling File %1.asm
C:\Programs\dasm %1.asm -f1 -o%1.prg -l%1.lst -s%1.sym

IF %ERRORLEVEL% NEQ 0 GOTO EOF

ECHO Starting Emulator
@start C:\Programs\WinVICE\x64.exe -config x64.ini %1.prg

:EOF
