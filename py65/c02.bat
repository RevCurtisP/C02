@ECHO Compiling File %1.c02
..\c02.exe %1 >%1.dbg
 
@ECHO Assembling File %1.asm
C:\Programs\dasm %1.asm -f3 -o%1.bin -l%1.lst -s%1.sym
