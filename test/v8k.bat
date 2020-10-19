@ECHO OFF
IF EXIST %1.c02 GOTO COMPILE
    ECHO File %1.c02 not found
    GOTO EOF

:COMPILE
ECHO Compiling File %1.c02 for VIC 20 +8k
..\c02.exe -h vic8k -s vic -s cbm %1 >%1.dbg
IF %ERRORLEVEL% NEQ 0 GOTO EOF
ECHO Assembling File %1.asm
..\a02.exe -d -p -i "../include ../include/vic20" %1.asm %1.prg %1.lst >%1.out
REM C:\Programs\dasm %1.asm -f1 -o%1.prg -l%1.lst -s%1.sym
IF %ERRORLEVEL% NEQ 0 GOTO EOF

ECHO Starting Emulator
@start C:\Programs\WinVICE\xvic.exe -config xvic.ini -1 testtape.t64 -10 geosdisk.d64 %1.prg

:EOF