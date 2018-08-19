@ECHO OFF
IF EXIST %1.c02 GOTO COMPILE
    ECHO File %1.c02 not found
    EXIT /B

:COMPILE
ECHO Compiling File %1.c02
..\c02.exe -h apple2 %1 >%1.dbg
IF ERRORLEVEL 1 EXIT /B

ECHO Assembling File %1.asm
dasm %1.asm -f3 -o%1.obj -l%1.lst -s%1.sym
IF ERRORLEVEL 1 EXIT /B

ECHO Building Disk Image
COPY /Y a2boot.dsk %1.dsk
ECHO BRUN %1 |python -c "print(input().upper())" |a2tools in t %1.dsk command
a2tools in b.0C00 %1.dsk %1 %1.obj
IF ERRORLEVEL 1 EXIT /B
DEL %1.obj

ECHO Starting Emulator
START C:\Programs\AppleWin\applewin -d1 %1.dsk 
