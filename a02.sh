#!/bin/bash
#Assemble C02 Program

#Check for Command Line Argument
if [[ "$1" = "" ]]; then
  echo "Usage: $SNAM file[.asm]"
  exit
fi

#Split File Name Parts
FSPC=$1;          #File Spec
FNAM=${FSPC%.*};  #File Name without Extension
FEXT=${FSPC##*.}; #File Extension

#Assemble ASM File
dasm $FNAM.asm -f3 -o$FNAM.bin -l$FNAM.lst -s$FNAM.sym
ESTS=$?; #Exit Status
if [[ $ESTS -ne 0 ]]; then
  echo "Error compiling file $FNAM.asm"
  exit $ESTS
fi 

#Report Successful Completion
echo "Successfully assembled file $FSPC"
