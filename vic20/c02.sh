#!/bin/bash
#Compile and Assemble C02 Program

#Check for Command Line Argument
if [[ "$1" = "" ]]; then
  echo "Usage: $SNAM file[.c02]"
  exit
fi

#Split File Name Parts
FSPC=$1;          #File Spec
FNAM=${FSPC%.*};  #File Name without Extension
FEXT=${FSPC##*.}; #File Extension

#Compile C02 FILE
../c02 $FSPC

#Assemble ASM File
echo "Assembling file $FNAM.asm"
dasm $FNAM.asm -f1 -o$FNAM.prg -l$FNAM.lst -s$FNAM.sym
ESTS=$?; #Exit Status
if [[ $ESTS -ne 0 ]]; then
  echo "Error Assembling file $FNAM.asm"
  exit $ESTS
fi

#Report Successful Completion
echo "Successfully compiled and assembled file $FSPC"

