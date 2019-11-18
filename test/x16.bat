@REM Compile and Run C02 Program for Commander X16 Emulator
@ECHO OFF
IF EXIST %1.c02 GOTO COMPILE
    ECHO File %1.c02 not found
    GOTO EOF

:COMPILE
ECHO Compiling File %1.c02 for Commander X16
..\c02.exe -c 65C02 -h x16 -s x16 -s cbm %1 >%1.dbg
IF %ERRORLEVEL% NEQ 0 GOTO EOF
ECHO Assembling File %1.asm
..\a02.exe -p %1.asm %1.prg %1.lst >%1.out
REM C:\Programs\dasm %1.asm -f1 -o%1.prg -l%1.lst -s%1.sym

IF %ERRORLEVEL% NEQ 0 GOTO EOF

ECHO Starting Emulator in Debug Mode
C:\Programs\x16emu\x16emu  -prg %1.prg -run -debug -echo raw >%1.out
:EOF
