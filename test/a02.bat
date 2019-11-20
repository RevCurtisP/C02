@ECHO OFF
IF "%1" == "" (
  ECHO Usage: %0 a02file
  GOTO :EXIT
) 
SET ASMEXT=""
IF EXIST %1.a02 SET ASMEXT=a02
IF EXIST %1.asm SET ASMEXT=asm
IF "%ASMEXT%" == "" (
  ECHO Assembly File %1 Not Found
  GOTO :EXIT
) 
ECHO Assembling File %1.%ASMEXT%
..\a02.exe %1.%ASMEXT% %1.bin %1.lst >%1.out
