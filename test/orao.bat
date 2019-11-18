@REM Compile and Assemble Program for Orao
@ECHO OFF

SET ORAOBIN=C:\Programs\OraoEmulator\bin
SET ORAOTAP=C:\Programs\OraoEmulator\tap
SET ORAOWAV=C:\Programs\OraoPy\wav
SET ORAOTOOLS=C:\Programs\OraoEmulator\tools

IF EXIST %1.c02 GOTO COMPILE
    ECHO File %1.c02 not found
    EXIT /B

:COMPILE
ECHO Compiling File %1.c02
..\c02.exe -h orao -s orao %1 >%1.dbg
IF ERRORLEVEL 1 EXIT /B

ECHO Assembling File %1.asm
dasm %1.asm -f1 -o%1.obj -l%1.lst -s%1.sym
IF ERRORLEVEL 1 EXIT /B

ECHO Creating Binary File
%ORAOTOOLS%\obj2bin %1.obj %ORAOBIN%\%1.bin
IF ERRORLEVEL 1 EXIT /B

ECHO Building Tape Image
%ORAOTOOLS%\maketap %ORAOTAP%\%1.tap %ORAOBIN%\%1.bin
IF ERRORLEVEL 1 EXIT /B

ECHO Building WAV File
%ORAOTOOLS%\orao2wav %ORAOBIN%\%1.bin
IF ERRORLEVEL 1 EXIT /B
MOVE %ORAOBIN%\%1.wav %ORAOWAV%

DEL %1.obj
