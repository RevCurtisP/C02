/* file6502.c - File I/O Command Processor for run6502.c */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <direct.h>
#include <dirent.h>

#include "config.h"
#include "lib6502.h"
#include "file6502.h"


typedef uint8_t  byte;
typedef uint16_t word;

int debug;

/* I/O Blocks for File I/O */
#define MAXIOB 15
#define STRLEN 128
struct iocb 
{
  int opened;           //Flag: file opened
  char type;            //Channel Type: 'F', 'D'
  char mode;            //File Mode: 'R', 'W'
  int recsize;          //Record Size (0=None)
  FILE *fp;             //File Pointer
  DIR *dp;              //Directory Pointer
  int errnum;           //Last Error Number
  char name[STRLEN];    //File/Directory Name
};

static struct iocb iocbs[MAXIOB];
static char filename[STRLEN]; //File name for open
static char filebuff[256];    //File I/O Buffer
static word fileaddr;         //File Read/Write Address
static word fileindx;         //File Record Size/Number

extern void setdebug(int dbg) {
	debug = dbg;
}

static int save(M6502 *mpu, word address, unsigned length, const char *path)
{
  FILE *file= 0;
  int   count= 0;
  if (!(file= fopen(path, "wb")))
    return 0;
  while ((count= fwrite(mpu->memory + address, 1, length, file)))
    {
      address += count;
      length -= count;
    }
  fclose(file);
  return address;
}


static int load(M6502 *mpu, word address, const char *path)
{
  FILE  *file= 0;
  int    count= 0;
  size_t max= 0x10000 - address;
  if (!(file= fopen(path, "rb")))
    return 0;
  if (debug) fprintf(stderr, "loading %s\nstart address %04x\n", path, address);
  while ((count= fread(mpu->memory + address, 1, max, file)) > 0)
  {
    address += count;
    max -= count;
  }
  fclose(file);
  if (debug) fprintf(stderr, "end address %04x\n", address);
  return address;
}

int setiocb(int chan, int opened, char type, char mode, int recsize, FILE *fp, DIR *dp, char *name) {
  iocbs[chan].opened = opened;
  iocbs[chan].type = type;
  iocbs[chan].mode = mode;
  iocbs[chan].recsize = recsize;
  iocbs[chan].fp = fp;
  iocbs[chan].dp = dp;
  iocbs[chan].errnum = 0;
  strcpy(iocbs[chan].name, name);
  return 0;
}

void initiocb(int chan) {
  setiocb(chan, 0, ' ', ' ', 0, NULL, NULL, "");
}

void initiocbs(void) 
{
  filename[0] = 0; //Set filename to ""
  for (int chan=0; chan<=MAXIOB; chan++)
    initiocb(chan);
}

/* Set Error Number and Error Message */
static int seterror(int chan, int errnum) {
  if (debug) fprintf(stderr, "setting channel %d error to %d\n", chan, errnum);
  iocbs[chan].errnum = errnum;
  return errnum;
}

/* Set File Name */
static int setname(M6502 *mpu, word addr, char *name) {
  int i;
  if (debug) fprintf(stderr, "copying name from address $%04x\n", addr);
  for (i=0; i<STRLEN; i++) {
    char c = mpu->memory[addr + i & 0xFFFF];
    if (c) name[i] = c;
    else break;
  }
  name[i] = 0; //Terminate String
  if (debug) fprintf(stderr, "copied %d characters\n", i);
  return i;
}

/* Find Unused IOCB */
static int uniocb(void) {
  int chan;
  for (chan=1; chan<=MAXIOB; chan++) {
    if (iocbs[chan].opened == 0) break;
    if (chan > MAXIOB) {chan = -24; break;}
  }
  return chan;
}

/* Validate Channel */
static int valchan(int chan, char valtype) {
  int errnum = 0; //Error (none)
  if (debug) fprintf(stderr, "validating channel %d\n", chan);
  if (chan > MAXIOB) errnum = 44; //Channel number out of range
  else if (iocbs[chan].opened == 0) errnum = 9; //Bad file descriptor
  else if (iocbs[chan].type != valtype) {
    if (debug) fprintf(stderr, "invalid channel type '%c'\n", iocbs[chan].type);
    switch (iocbs[chan].type) {
      case 'D': errnum = 21; //Is a directory
      case 'F': errnum = 20; //Not a directory
      default: errnum = 77; //File descriptor in bad state 
    }
  }
  return errnum;
}

/* Write Buffer to Memory */
static int writemem(M6502 *mpu, char* buffer, int count) {
  int i;
  for (i = 0; i<count; i++)
    mpu->memory[fileaddr+i] = buffer[i];
  return i;
}  

/* Write String to Memory */
static int writestr(M6502 *mpu, char* buffer, int count) {
  int i;
  if (debug) fprintf(stderr, "writing '%s' to address %04x\n", buffer, fileaddr);
  for (i = 0; i<count; i++) {
    char c = buffer[i];
    if (c == 0) break;    
    mpu->memory[fileaddr+i] = c;
  }
  mpu->memory[fileaddr+i] = 0; //Terminate String
  if (debug) fprintf(stderr, "wrote %d characters\n", i);
  return i;
}

/* Emulate fileio at addr */
extern int filecmd(M6502 *mpu, word addr, byte data)	{ 
  const char modes[8][4] = {"r", "rb", "w", "wb", "a", "ab", "r+", "rb+"};
  const char qdesc[2][7] = {"string", "line"};
  int chan, e, i;
  char c, m, n, mode[4];
  char *name;
  struct dirent *de;
  byte a = mpu->registers->a;
  byte x = mpu->registers->x;
  byte y = mpu->registers->y;
  byte p = mpu->registers->p;
  word yx = y << 8 | x;
  char drive = y & 0x1f;
  if (debug) fprintf(stderr, "executing '%c' with options %02x,%02x,%02x\n", a, y, x, p);
  switch (a) { //File I/O Command
    case 'A': //Set filebuffer address - Y.X = address
      if (p & 1) {
        fileindx = yx;
        if (debug) fprintf(stderr, "file index set to %04x\n", fileaddr);
      } else { 
        fileaddr = yx;
        if (debug) fprintf(stderr, "file address set to %04x\n", fileaddr);
      } 
      break;
    case 'B': //Close Directory - Y = channel
      chan = y;
      if (debug) fprintf(stderr, "closing directory channel %d\n", chan);
      y = valchan(chan, 'D'); if (y) break;
      if (closedir(iocbs[chan].dp)) y = seterror(chan, errno);
      else initiocb(chan);
      break;
    case 'C': //Close file - Y = channel
      chan = y;
      if (debug) fprintf(stderr, "closing file channel %d\n", chan);
      y = valchan(chan, 'F'); if (y) break;
      if (fclose(iocbs[chan].fp)) y = seterror(chan, errno);
      else initiocb(chan);
      break;
    case 'D': //Open Directory - Y,X = Directory Name
      x = 0; //File channel (none)
      y = 0; //Error code (none)
      chan = uniocb(); if (chan <0) {y = -chan; break;}
      setname(mpu, yx, filename); 
      if (strlen(filename) == 0) strcpy(filename, ".");
      if (debug) fprintf(stderr, "opening directory '%s'\n", filename, mode);
      DIR *dp = opendir(filename);
      if (dp == NULL) { y = seterror(0, errno); break;}
      if (debug) fprintf(stderr, "directory opened on channel %d\n", chan);
      y = setiocb(chan, -1, 'D', ' ', 0, NULL, dp, filename); //Setup IOCB      
      if (y == 0) x = chan;
      break;
    case 'E': //EOF - Y = channel
      chan = y;
      y = valchan(chan, 'F'); if (y) break;
      y = feof(iocbs[chan].fp);
      break;
    case 'F': //Flush File - Y = Channel
      chan = y;
      y = valchan(chan, 'F'); if (y) break;
      if (fflush(iocbs[chan].fp)) y = seterror(chan, errno);
      break;
    case 'G': //Get character - Y = channel
      chan = y;
      x = 0; //Character read (none)
      y = valchan(chan, 'F'); if (y) break;
      c = fgetc(iocbs[chan].fp);
      if (feof(iocbs[chan].fp)) {y = 255; break;}
      if (c == EOF) {y = seterror(chan, errno); break;}
      x = c & 0xFF;
      break;
    case 'H': //Get String - Y = channel
      chan = y;
      x = 0; //Number of Characters read
      y = valchan(chan, 'F'); if (y) break;
      char *s = fgets(filebuff, STRLEN, iocbs[chan].fp);
      if (s == NULL) {y = seterror(chan, errno); break;}
      if (debug) fprintf(stderr, "read string '%s'\n", filebuff);
      writestr(mpu, filebuff, STRLEN);
      x = strlen(s); fprintf(stderr, "returning %d bytes read\n", x);
      break;
    case 'I': //Init File System
      initiocbs();  //Initialize I/O Control Blocks
      break;
    case 'J': //Read Directory Entry
      chan = y;
      x = 0; //Return Value (Read Failed)
      y = valchan(chan, 'D'); if (y) break;
      if (p & 1) {
        if (debug) fprintf(stdout, "retrieving directory name\n");
        x = writestr(mpu, iocbs[chan].name, STRLEN);
      } else {
        if (debug) fprintf(stdout, "reading directory entry\n");
        de = readdir(iocbs[chan].dp);
        if (de) {
          if (debug) fprintf(stdout, "read entry '%s'\n", de->d_name);
          x = writestr(mpu, de->d_name, STRLEN);
        }
        else if (errno != 2) y = seterror(chan, errno);
      }
      break;
    case 'K': //REMOVE - Delete File - Y,X = Filename
      setname(mpu, yx, filename); 
      if (debug) fprintf(stderr, "removing file '%s'\n", filename);
      x = remove(filename);
      if (x) y=seterror(0, errno); else y=0;
      break;
    case 'L': //Load file
      a = 0; //Error (none)
      if (debug) fprintf(stderr, "loading file at %04h\n", fileaddr);
      e = load(mpu, fileaddr, filename);
      if (!e) {a = seterror(0, errno); break;}
      y = e >> 8; x = e & 0xff;
      break;
    case 'M': //MOVE - Rename File  - Y,X = Filename
      setname(mpu, yx, filebuff); 
      if (debug) fprintf(stderr, "renaming file '%s' to '%s'\n", filename, filebuff);
      x = rename(filename, filebuff);
      if (x) y=seterror(0, errno); else y=0;
      break;
    case 'N': //Set filename - Y,X = string address
      x = setname(mpu, yx, filename); //Set filename and Return Length
      if (debug) fprintf(stderr, "filename set to '%s'\n", filename);
      break;
    case 'O': //Open file - Y = Drive#, X = Mode (Bits 7,6 = RWA, Bit 5 = Binary)
      m = x >> 5 & 7; //Get Mode Index
      x = 0; //File channel (none)
      y = 0; //Error code (none)
      strcpy(mode, modes[m]); //Set Mode from Mode Index
      chan = uniocb(); if (chan <0) {y = seterror(0, -chan); break;}
      if (debug) fprintf(stderr, "opening file '%s' with mode '%s'\n", filename, mode);
      FILE *fp = fopen(filename, mode);
      if (fp == NULL) { y = seterror(0, errno); break;}
      if (debug) fprintf(stderr, "file opened on channel %d\n", chan);
      i = (m == 3) ? fileindx : 0; //Set Records Size if Mode RECORD
      y = setiocb(chan, -1, 'F', mode[0], i, fp, NULL, filename); //Setup IOCB      
      if (y == 0) x = chan;
      break;
    case 'P': //Put character - Y = channel; X = character
      chan = y;
      c = x;
      if (debug) fprintf(stderr, "writing '%c' to channel %d\n", c, chan);
      a = 0; //Character written (none)
      y = valchan(chan, 'F'); if (y) break;
      e = fputc(c, iocbs[chan].fp);
      if (e == EOF) {y = seterror(chan, errno); break;}
      a = e & 0xFF;
      break;
    case 'Q': //Put String - Y = channel
      chan = y;
      x = 0; //Number of characters written
      y = valchan(chan, 'F'); if (y) break;
      for (i = 0; i<128; i++) {
        c = mpu->memory[fileaddr+i];
        if (c) filebuff[i] = c;
        else break;
      }
      filebuff[i] = 0;
      if (debug) fprintf(stderr, "writing %s '%s'\n", qdesc[p&1], filebuff);
      if (p & 1) strcat(filebuff, "\n");
      e = fputs(filebuff, iocbs[chan].fp);
      if (e == EOF) {y = seterror(chan, errno); break;}
      x = strlen(filebuff);
      break;
    case 'R': //Read bytes - Y = channel, X=Number of Bytes
      m = p & 1; //Set Mode: 0 = fread, 1 = fgetr
      chan = y;
      if (m) {
        a = valchan(chan, 'F'); if (a) break;
        if (debug) fprintf(stderr, "selecting record #%d\n", fileindx);
        n = iocbs[chan].recsize;
        i = fileindx * n;
        if (debug) fprintf(stderr, "setting position to %d\n", i);
        e = fseek(iocbs[chan].fp, i, SEEK_SET);
        if (e) {a = seterror(chan, errno); break;}
      } else {
        y = valchan(chan, 'F'); if (y) break;
        n = x;
      }
      if (debug) fprintf(stderr, "reading %d bytes\n", n);
      x = fread(filebuff, 1, n, iocbs[chan].fp);
      if (debug) fprintf(stderr, "read %d bytes\n", x);
      e = ferror(iocbs[chan].fp); 
      if (e) {seterror(chan, errno); if (m) a = e; else y = e; break; }
      writemem(mpu, filebuff, x);
      if (m) { 
        fileindx++; y = fileindx >> 8; x = fileindx & 0xff;
        if (debug) fprintf(stderr, "returning record #%d\n", fileindx);
      }
      break;
    case 'S': //Save file Y,X = end address
      a = 0; //Error (none)
      if (debug) fprintf(stderr, "saving file from %04x to %04x\n", fileaddr, yx);
      e = save(mpu, fileaddr, yx-fileaddr-1, filename);
      if (!e) {a = seterror(0, errno); break;}
      y = e >> 8; x = e & 0xff;
      break;
    case 'T': //Get Current Directory - YX = String Address
      x = 0; //Directory Name Length
      y = 0; //Error (None)
      if (debug) fprintf(stderr, "getting current working directory\n");
      if (_getcwd(filename, STRLEN)) {
        if (debug) fprintf(stderr, "cwd: %s\n", filename);
        for (i = 0; i<STRLEN; i++) {
          c = filename[i];
          if (c == 0) break;
          mpu->memory[yx+i] = c;
        }
        mpu->memory[yx+i] = 0;
        x = i;
      }  
      else y = seterror(0, errno);
      break;
    case 'U': //Change Directory - YX = Directory Name
      x = 0; //Result (Success)
      y = 0; //Error (None)
      setname(mpu, yx, filename); //Set filename to Directory Name
      if (debug) fprintf(stderr, "changing directory to '%s'\n", filename);
      if (_chdir(filename)) {x = 0xFF; y = seterror(0, errno);}
      break;
    case 'V': //Get or Set Current Drive, Y=Drive, Carry=Get/Set
      if (p & 1) {
        if (debug) fprintf(stderr, "changing drive to %c\n", y+'@');
        x = _chdrive(y);
        if (x) y=seterror(0, errno); else y=0;
      } else {
        x = _getdrive();
        if (x) y=0; else y=seterror(0, errno);
        if (debug) fprintf(stderr, "current drive is %c\n", x+'@');
      }
      break;
    case 'W': //Write bytes - Y = channel, X=Number of Bytes
      chan = y;
      y = valchan(chan, 'F'); if (y) break;
      for (i = 0; i<x; i++) 
        filebuff[i] = mpu->memory[fileaddr+i];
      if (debug) fprintf(stderr, "writing %d bytes\n", x);
      e = fwrite(filebuff, x, 1, iocbs[chan].fp);
      if (e != 1) {y = seterror(chan, errno); break;}
      break;
    case 'X': //Make/Remove Directory - YX = Directory Name
      setname(mpu, yx, filename); 
      if (p & 1) {
        if (debug) fprintf(stderr, "removing directory '%s'\n", filename);
        x = _rmdir(filename);
      } else {
        if (debug) fprintf(stderr, "creating directory '%s'\n", filename);
        x = _mkdir(filename);
      }
      if (x) y=seterror(0, errno); else y=0;
      break;
    case 'Y': //Get Last Error: Y=chan
      chan = y;
      y = 0; //Set Error to None
      x = 0xFF; //Set Result to Invalid
      if (chan > MAXIOB) {y = 44; break;}
      if (debug) fprintf(stderr, "getting last error for channel %d\n", chan);
      x = iocbs[chan].errnum;
      char *msg = strerror(x);
      if (debug) fprintf(stderr, "retrieved error %d, '%s'\n", x, msg);
      writestr(mpu, msg, STRLEN);
      break;
    case 'Z': //File Position: Y=chan, fileaddr = position, Carry = seek/tell
      chan = y;
      y = valchan(chan, 'F'); if (y) break;
      if (p & 1) {
        e = ftell(iocbs[chan].fp);
        if (e < 0) {a = seterror(chan, errno);}
        else {
          if (debug) fprintf(stderr, "returning position %d\n", e);
          a = 0; y = e >> 8; x = e & 0xff;
        }
      } else {
        if (fileindx == 0xFFFF) {i = 0; m = SEEK_END;}
        else {i = fileindx, m = SEEK_SET;}
        if (debug) fprintf(stderr, "seeking position %d\n", i);
        x = fseek(iocbs[chan].fp, i, m);
				if (x) y = seterror(chan, errno);
      }
      break;
    default: 
      y = 22; //Error - invalid argument
  }
  if (debug) fprintf(stderr, "returning values %02x, %02x, %02x\n", a, y, x);
  mpu->registers->a = a;
  mpu->registers->x = x;
  mpu->registers->y = y;
}
