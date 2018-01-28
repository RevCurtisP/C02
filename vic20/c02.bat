@ECHO Compiling File %1.c02
..\c02.exe %1

@ECHO Assembling File %1.asm
C:\Programs\dasm %1.asm -f1 -o%1.prg -l%1.lst -s%1.sym
