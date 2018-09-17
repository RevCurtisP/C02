@ECHO OFF
REM Compile and Assemble Program for Apple-1
IF EXIST %1.c02 GOTO COMPILE
    ECHO File %1.c02 not found
    EXIT /B

:COMPILE
ECHO Compiling File %1.c02
..\c02.exe -h apple1 -s apple1 %1 >%1.dbg
IF ERRORLEVEL 1 EXIT /B

ECHO Assembling File %1.asm
dasm %1.asm -f1 -o%1.obj -l%1.lst -s%1.sym
IF ERRORLEVEL 1 EXIT /B

ECHO Converting Object File
python ..\util\bin2asc.py %1
IF ERRORLEVEL 1 EXIT /B
DEL %1.obj

ECHO Copying ASCII File
COPY %1.asc C:\Programs\pom1\asc\%1.asc
