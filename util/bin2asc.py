#Binary to ASCII
#Converts DASM -F1 file to Apple 1 Monitor format

#DASM -F1 File two byte load address in LSB, MSB order
#followed by binary object coda

#Apple 1 Monitor Format
#ADDR: BYTE BYTE BYTE...
#0300: 00 01 02 03 04 05 06 07
#0308: 08 09 0A 0B 0C 0D 0E 0F

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

def writeAscFile(dir, name, addr, data):
    ascName = os.path.join(dir, name + ".asc")
    line = "";
    with open(ascName, "w") as ascFile:
        for i in range(len(data)):
            if i & 7 == 0:
                if len(line): ascFile.write(line + '\n')
                line = "{:04X}:".format(addr + i)
            line = line + " {:02X}".format(data[i])
        ascFile.write(line + '\n')
    return ascName

objFile = getArgs()
fileDir, fileName, fileExt = splitFileSpec(objFile)
inName = os.path.join(fileDir, fileName + fileExt)
print("Reading file", inName)
inAddr, inData = readPrgFile(inName)
outName = writeAscFile(fileDir, fileName, inAddr, inData)
print ("Wrote file", outName)
