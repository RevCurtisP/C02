#Oric .TAP Converter
#Builds Oric/Atmos .TAP file from DASM -F1 file

#DASM -F1 File two byte load address in LSB, MSB order
#followed by binary object coda

#Oric/Atmos .TAP file has the following structure:
# $16 $16 $16 $16   Tape Synchronization Bytes
# $24 $00 $00       Header Start Bytes
# $00/$80           Program Type (BASIC/Assembly)
# $00/$80/$C7       Autostart (None/BASIC/Assembly)
# $XX $XX           End Address (MSB, LSB)
# $XX $XX           Start Address (MSB, LSB)
# $00               Padding Byte
# 'XXX...'          File Name (up to 16 characters)
# $00               File Name Terminator
# ...               Program Data

import os
import sys

def getArgs():
    argCount = len(sys.argv)
    if argCount != 2:
        print("Usage:", sys.argv[0], "objfile")
        exit(2)
    return sys.argv[1]

def splitFileSpec(filePath):
    fileDir, fileSpec = os.path.split(filePath)
    fileName, fileExt = os.path.splitext(fileSpec)
    if fileExt == '': fileExt = '.obj'
    return fileDir, fileName, fileExt

def readPrgFile(fileSpec):
    with open(fileSpec, "rb") as prgFile:
        prgAddr = prgFile.read(2)
        prgData = prgFile.read()
    address = prgAddr[0] + prgAddr[1] * 256
    return address, prgData

def writeTapFile(dir, name, addr, data):
    tapName = os.path.join(dir, name + ".tap")
    size = len(data)+1              #Program Size includes trailing 0 byte
    type = b'\x00' if addr == 0x0501 else b'\x80'   #Program Type BASIC/Assembly
    auto = b'\x00'                  #Autostart (None)
    #auto = b'\x80' if type == b'\x00' else b'\xC7'  #Autostart Flag
    start = addr.to_bytes(2, byteorder='big')       #Program Start Address
    end = (addr+size).to_bytes(2, byteorder='big')  #Program End Address
    ascName = name.upper().encode('ASCII')          #Program Name as ASCII bytes object
    with open(tapName, "wb") as tapFile:
        tapFile.write(b'\x16\x16\x16\x16')  #Tape Synchronization Bytes
        tapFile.write(b'\x24\x00\x00')      #Header Start Byte
        tapFile.write(type)         #Program Type
        tapFile.write(auto)         #Autostart Flag
        tapFile.write(end)          #Program End Address
        tapFile.write(start)        #Program Start Address MSB
        tapFile.write(b'\x00')      #Padding Byte
        tapFile.write(ascName)      #File Name
        tapFile.write(b'\x00')      #String Terminator
        tapFile.write(data)         #Binary Program Code
        tapFile.write(b'\x00')      #Final Dummy Byte
    return tapName

objFile = getArgs()
fileDir, fileName, fileExt = splitFileSpec(objFile)
inName = os.path.join(fileDir, fileName + fileExt)
print("Reading file", inName)
inAddr, inData = readPrgFile(inName)
outName = writeTapFile(fileDir, fileName, inAddr, inData)
print ("Wrote file", outName)
