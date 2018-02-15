REM Create Bootable Disk with Binary File
copy /Y boot.dsk %1.dsk
echo BRUN %1 |a2tools in t %1.dsk command
a2tools in b.0C00 %1.dsk %1 %1.bin
