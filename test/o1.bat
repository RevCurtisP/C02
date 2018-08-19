@ECHO OFF
REM Compile and Assemble Program for ORIC-1
IF EXIST %1.c02 GOTO COMPILE
    ECHO File %1.c02 not found
    EXIT /B

:COMPILE
ECHO Compiling File %1.c02
..\c02.exe -h oric %1 >%1.dbg
IF ERRORLEVEL 1 EXIT /B

ECHO Assembling File %1.asm
dasm %1.asm -f1 -o%1.obj -l%1.lst -s%1.sym
IF ERRORLEVEL 1 EXIT /B

ECHO Building Disk Image
python ..\util\orictap.py %1
IF ERRORLEVEL 1 EXIT /B
DEL %1.obj

REM ECHO Starting Emulator
REM START C:\Programs\Oricutron\oricutron.exe -m1 -t%1.tap 
