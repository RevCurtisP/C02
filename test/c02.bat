@ECHO OFF
IF EXIST %1.c02 GOTO COMPILE
    ECHO File %1.c02 not found
    GOTO EOF

:COMPILE
ECHO Compiling File %1.c02
..\c02.exe -h header %1 >%1.dbg

:EOF