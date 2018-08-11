@ECHO Assembling File %1.asm
dasm %1.asm -f3 -o%1.bin -l%1.lst -s%1.sym

