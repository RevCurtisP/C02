@echo off
echo Compiling File %1.c02
..\c02.exe %1 >%1.dbg
if ERRORLEVEL 1 exit /b

echo Assembling File %1.asm
dasm %1.asm -f3 -o%1.bin -l%1.lst -s%1.sym
if ERRORLEVEL 1 exit /b

echo Building Disk Image
copy /Y boot.dsk %1.dsk
echo BRUN %1 |a2tools in t %1.dsk command
a2tools in b.0C00 %1.dsk %1 %1.bin
if ERRORLEVEL 1 exit /b
del %1.bin

echo Starting Emulator
start C:\Programs\AppleWin\applewin -d1 %1.dsk 
