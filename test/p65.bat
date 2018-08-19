@ECHO OFF
IF EXIST %1.c02 GOTO COMPILE
    ECHO File %1.c02 not found
    GOTO EOF

:COMPILE
@ECHO Compiling File %1.c02 for py65mon
..\c02.exe -h py65 %1 >%1.dbg

IF %ERRORLEVEL% NEQ 0 GOTO EOF

 @ECHO Assembling File %1.asm
C:\Programs\dasm %1.asm -f3 -o%1.bin -l%1.lst -s%1.sym

:EOF
