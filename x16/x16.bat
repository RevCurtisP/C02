@REM Compile and Run C02 Program for Commander X16 Emulator
@ECHO OFF
IF EXIST %1.c02 GOTO COMPILE
    ECHO File %1.c02 not found
    GOTO EOF

:COMPILE
ECHO Compiling File %1.c02 for Commander X16
..\c02.exe -h x16 -s x16 -s cbm %1 >%1.dbg
IF %ERRORLEVEL% NEQ 0 GOTO EOF
ECHO Assembling File %1.asm
C:\Programs\dasm %1.asm -f1 -o%1.prg -l%1.lst -s%1.sym

IF %ERRORLEVEL% NEQ 0 GOTO EOF

ECHO Starting Emulator in Debug Mode
REM C:\Programs\x16emu\x16emu  -prg %1.prg -run -debug -log v >%1.out
C:\Programs\x16emu\x16emu  -prg %1.prg -run -debug
:EOF
