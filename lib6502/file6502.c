/* file6502.c - File I/O Command Processor for run6502.c */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <direct.h>
#include <dirent.h>

#include "error.h"
#include "lib6502.h"
#include "file6502.h"

typedef uint8_t  byte;
typedef uint16_t word;

int debug;

byte *xmemory[16];  //Extended Memory
byte xmembank;  //Extended Memory Current Bank
word xmemaddr;  //Extended Memory Current Address
word smemaddr;  //System Memory Address

#define STRLEN 128
#define STRSIZ STRLEN+1

/* DOS File Attributes */
#define _A_ARCH	  0x20  //Archive. Set whenever the file is changed, and cleared by the BACKUP command.
#define _A_HIDDEN	0x02  //Hidden file. Not normally seen with the DIR command, unless the /AH option is used. 
#define _A_RDONLY	0x01  //Read-only. File cannot be opened for writing.
#define _A_SUBDIR	0x10  //Subdirectory.
#define _A_SYSTEM	0x04  //System file. Not normally seen with the DIR command, unless the /AS option is used.

/* I/O Blocks for File I/O */
#define MAXIOB 15
#define TDIR  'D'
#define TFILE 'F'

typedef struct dirent dirent;

struct iocb 
{
  int opened;           //Flag: file opened
  char type;            //Channel Type: TFILE, TDIR
  char mode;            //File Mode: 'R', 'W'
  int recsize;          //Record Size (0=None)
  FILE *fp;             //File Pointer
  DIR *dp;              //Directory Pointer
  int errnum;           //Last Error Number
  char name[STRSIZ];    //File/Directory Name
};

static struct iocb iocbs[MAXIOB];
static char filename[STRSIZ]; //File name for open
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
  if (debug) fprintf(stderr, "cleared iocb for channel %d\n", chan);
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

/* Normalize Drive ID */
static char driveid(char drive) {
	char id; 
	if (debug) fprintf(stderr, "normalizing drive $%02x\n", drive);
	if (drive == 0 || strchr(" 0@", drive)) id = 0; //Default Drive
	else id = toupper(drive);
	if (debug) fprintf(stderr, "returning driveid $%02x\n", id);
  return id;	
}

/* Convert Drive ID to Drive Number  *
 * Args: driveid - Drive ID (A-Z)    *           
 * Returns: Drive Number (1-126,     *
 *          0 for Default, or -errno */
static int driveno(char drvid) {
	int id = driveid(drvid);
  if (debug) fprintf(stderr, "converting drive id %02x\n", drvid);
	if (id == 0 ) return 0;
  if (id >='A' && id <='Z') return id - '@';
	return -seterror(0, ENXIO); //No such device or address
}

/* Change Current Drive        *
 * Args: drivno - drive number (1-26) *
 * Returns: Error (0=None)     */
static int chgdrive(char drvno) {
  if (debug) fprintf(stderr, "changing default drive to %d: ", drvno);
  int r = _chdrive(drvno); 
  if (r) r = seterror(0, errno);
  else if (debug) fprintf(stderr, "default drvno changed\n");
  return r;
}

/* Build File Spec */
static int setspec(char *spec, char driveid, char* name, char* dflt) {
  if (debug) fprintf(stderr, "building filespec for drive $%02x, name \"%s\"\n", driveid, name);
	spec[0] = 0; //Set filespec to ""
	int drive = driveno(driveid); if (drive < 0) return seterror(0, -drive);
  for (int i = 0; i<STRLEN; i++) if (name[i] == '/') name[i] = '\\'; //Convert Slashes
  if (strlen(name) == 0) {
		strcpy(name, dflt);
		if (debug) fprintf(stderr, "name changed to \"%s\"\n", name);
	}
	if (drive) {sprintf(spec, "%c:", drive); strncat(spec, name, STRLEN);} 
  else strncpy(spec, name, STRLEN);
	return ENOERROR;
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
static int valchan(int chan, char valtype, FILE *fp) {
  int errnum = 0; //Error (none)
  if (debug) fprintf(stderr, "validating channel %d\n", chan);
  if (chan == 0) {if (fp) iocbs[chan].fp = fp; else errnum = 9; }
  else if (chan > MAXIOB) errnum = 44; //Channel number out of range
  else if (iocbs[chan].opened == 0) errnum = 9; //Bad file descriptor
  else if (iocbs[chan].type != valtype) {
    if (debug) fprintf(stderr, "invalid channel type '%c'\n", iocbs[chan].type);
    switch (iocbs[chan].type) {
      case TDIR: errnum = EISDIR; 	//Is a directory
      case TFILE: errnum = ENOTDIR; //Not a directory
      default: 	errnum = EBADFD; 	//File descriptor in bad state 
    }
  }
  return errnum;
}

/* Write Buffer to Memory */
static int writemem(M6502 *mpu, char* buffer, int count) {
  int i;
  if (debug) fprintf(stderr, "writing to address %04x\n", fileaddr);
  for (i = 0; i<count; i++)
    mpu->memory[fileaddr+i] = buffer[i];
  if (debug) fprintf(stderr, "wrote %d bytes\n", i);
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

/* Convert tm struct to readable time */
void strtm(char *s, tm *time) {
  sprintf(s, "%d-%02d-%02d %02d:%02d:%02d", 
          time->tm_year+1900, time->tm_mon, time->tm_mday, 
          time->tm_hour, time->tm_min, time->tm_sec);
}

int copystring(char *b, char *s, int ofs) {
  if (debug) fprintf(stderr, "copying string \"$s\" to buffer at offset %d\n", s, ofs);
  for (int i = 0; i < STRLEN; i++) {
    if (s[i]) b[ofs++] = s[i]; else break;
  }
  b[ofs++] = 0;
  return ofs;  
}

int copytime(char *b, tm *t, int ofs) {
  char s[STRLEN]; strtm(s, t);
  t->tm_mon = t->tm_mon + 1; //Adjust Zero-based Month
  if (debug) fprintf(stderr, "copying timestamp %s to buffer at offset %d\n", s, ofs);
  b[ofs++] = t->tm_year; b[ofs++] = t->tm_mon; b[ofs++] = t->tm_mday;
  b[ofs++] = t->tm_hour; b[ofs++] = t->tm_min; b[ofs++] = t->tm_sec;
  return ofs;
}

int copyword(char *b, word w, int ofs) {
  if (debug) fprintf(stderr, "copying word %04x to buffer at offset %d\n", w, ofs);
  b[ofs++] = w & 0xFF; b[ofs++] = w >> 8;
  return ofs;  
}

int copyattrs(char *b, int attr, int ofs) {
  b[ofs++] = (attr & _A_ARCH)   ? 0xFF : 00;  
  b[ofs++] = (attr & _A_HIDDEN) ? 0xFF : 00;  
  b[ofs++] = (attr & _A_RDONLY) ? 0xFF : 00;  
  b[ofs++] = (attr & _A_SUBDIR) ? 0xFF : 00;  
  b[ofs++] = (attr & _A_SYSTEM) ? 0xFF : 00;  
  return ofs;
}

/* Copy dirent to character array matching run6502.h02               *
 * struct dirent {                                                   * 
 *   struct {char arch, hidden, system, rdonly, subdir, system} attr *
 *   struct {char year, month, day, hour, minute, second;} time; *
 *   int size, char name[128];}                                            */
int setentry(char *buffer, dirent *de) {
  int offset = 0;
  char ts[STRSIZ]; strtm(ts, de->d_time);
  if (debug) fprintf(stderr, "copying entry \"%s\", attr=$%02x, size=%d, time=%s\n", 
                              de->d_name, de->d_attr, de->d_size, ts);
  offset = copyattrs(buffer, de->d_attr, offset);
  offset = copytime(buffer, de->d_time, offset);
  offset = copyword(buffer, de->d_size, offset);
  offset = copystring(buffer, de->d_name, offset);
  return offset;
}

/* Emulate fileio at addr */
extern int filecmd(M6502 *mpu, word addr, byte data)	{ 
  const char modes[8][4] = {"r", "rb", "w", "wb", "a", "ab", "a+", "ab+"};
  const char ddesc[2][9] = {"creating", "removing"};
  const char fdesc[2][10] = {"file", "directory"};
  const char qdesc[2][7] = {"string", "line"};
  int chan, e, i;
  char c, d, m, n, t, mode[4];
  char *buffer;
  char filespec[STRSIZ], newspec[STRSIZ] ;
  struct tm time;
  dirent *de;
  byte a = mpu->registers->a;
  byte x = mpu->registers->x;
  byte y = mpu->registers->y;
  byte p = mpu->registers->p;
  word yx = y << 8 | x;
  if (debug) fprintf(stderr, "executing file command '%c' with options %02x,%02x,%02x\n", a, y, x, p);
  switch (a) { //File I/O Command
    case 'A': //Set filebuffer address - Y.X = address
      if (p & 1) {
        fileindx = yx;
        if (debug) fprintf(stderr, "file index set to %04x\n", fileindx);
      } else { 
        fileaddr = yx;
        if (debug) fprintf(stderr, "file address set to %04x\n", fileaddr);
      } 
      break;
    //case 'B': //Unused
    case 'C': //Close Channel - Y = channel, CC = File, CS = Directory
      chan = y;
      m = p & 1; t = (m) ? TDIR : TFILE;
      if (debug) fprintf(stderr, "closing %s channel %d\n", fdesc[m], chan);
      y = valchan(chan, t, NULL); if (y) break;
      if (m) e = closedir(iocbs[chan].dp); else e = fclose(iocbs[chan].fp);
      if (e) y = seterror(chan, errno); else initiocb(chan);
      if (debug) fprintf(stderr, "channel closed\n");
      break;
    case 'D': //Read Directory - Y = Channel, CC = Entry, CS = Header
      chan = y;
      x = 0; //Return Value (Read Failed)
      y = valchan(chan, TDIR, NULL); if (y) break;
      if (p & 1) {
        if (debug) fprintf(stderr, "retrieving directory name\n");
        x = writestr(mpu, iocbs[chan].name, STRLEN);
      } else {
        if (debug) fprintf(stderr, "reading directory entry\n");
        de = readdir(iocbs[chan].dp);
        if (de) {x = setentry(filebuff, de); writemem(mpu, filebuff, x);}
        else if (errno != 2) y = seterror(chan, errno);
      }
      break;
    case 'E': //EOF - Y = channel
      chan = y;
      y = valchan(chan, TFILE, NULL); if (y) break;
      y = feof(iocbs[chan].fp);
      break;
    case 'F': //Flush File - Y = Channel
      chan = y;
      y = valchan(chan, TFILE, NULL); if (y) break;
      x = fflush(iocbs[chan].fp);
      if (x) y = seterror(chan, errno);
      break;
    case 'G': //Get Character - Y = channel
      chan = y;
      x = 0; //Character read (none)
      y = valchan(chan, TFILE, stdin); if (y) break;
      c = fgetc(iocbs[chan].fp);
      if (feof(iocbs[chan].fp)) {y = EOF; break;}
      if (c == EOF) {y = seterror(chan, errno); break;}
      x = c & 0xFF;
      break;
    case 'H': //Get String - Y = channel
      chan = y;
      x = 0; //Number of Characters read
      y = valchan(chan, TFILE, stdin); if (y) break;
      char *s = fgets(filebuff, STRLEN, iocbs[chan].fp);
      if (s == NULL) {y = seterror(chan, errno); break;}
      if (debug) fprintf(stderr, "read string '%s'\n", filebuff);
      writestr(mpu, filebuff, STRLEN);
      x = strlen(s); fprintf(stderr, "returning %d bytes read\n", x);
      break;
    case 'I': //Init File System
      initiocbs();  //Initialize I/O Control Blocks
      break;
    case 'J': //Get/Put Word - Y = Channel, Carry Set = Put, Clear = Get
      chan = y;
      a = valchan(chan, TFILE, NULL); if (y) break;
      if (p & 1) {
        y = fileaddr >> 8; x = fileaddr & 0xFF;
        e = fputc(x, iocbs[chan].fp);
        if (e == EOF) {a = seterror(chan, errno); break;}
        e = fputc(y, iocbs[chan].fp);
        if (e == EOF) {a = seterror(chan, errno); break;}
      } else {
        c = fgetc(iocbs[chan].fp);
        if (feof(iocbs[chan].fp)) {a = EOF; break;}
        if (c == EOF) {a = seterror(chan, errno); break;}
        x = c & 0xFF;
        c = fgetc(iocbs[chan].fp);
        if (feof(iocbs[chan].fp)) {a = EOF; break;}
        if (c == EOF) {a = seterror(chan, errno); break;}
        y = c & 0xFF;
      }
      break;
    case 'K': //REMOVE - Delete File - Y,X = Filename
      d = driveid(y); 
			y = setspec(filespec, d, filename, "."); if (y) break;
      if (debug) fprintf(stderr, "removing file '%s'\n", filespec);
      e = remove(filespec); 
      if (e) y=seterror(0, errno);
      break;
    case 'L': //Load file
      //ALLOW SPECIFYING DRIVE ID
      a = ENOERROR; //Error (none)
      if (debug) fprintf(stderr, "loading file at %04h\n", fileaddr);
      e = load(mpu, fileaddr, filename);
      if (!e) {a = seterror(0, errno); break;}
      y = e >> 8; x = e & 0xff;
      break;
    case 'M': //MOVE - Rename File  - Y = Drive ID
      d = driveid(y); //Not Implemented Yet
			y = setspec(filespec, d, filename, ".");  if (y) break;
			y = setspec(newspec, d, filebuff, ".");  if (y) break;
      if (debug) fprintf(stderr, "renaming file \"%s\" to \"%s\"\n", filename, filebuff);
      e = rename(filespec, newspec); 
      if (e) y=seterror(0, errno); else y=0;
      break;
    case 'N': //Set Name - Y,X = string address, CC = filename, CS = filebuff
      m = p & 1;
      if (m) {
        x = setname(mpu, yx, filebuff); //Set filebuff and Return Length
        if (debug) fprintf(stderr, "filebuff set to \"%s\"\n", filename);
      } else { 
        x = setname(mpu, yx, filename); //Set filename and Return Length
        if (debug) fprintf(stderr, "filename set to \"%s\"\n", filename);
      }
      break;
    case 'O': //Open file/directory - Y = Drive#, X = Mode, CC = File, CS = Directory
			if (p & 1) {
				x = 0; //File channel (none)
				d = driveid(y); //Normalize Drive ID
				y = setspec(filespec, d, filename, "."); if (y) {seterror(0, y); break;}
				chan = uniocb(); if (chan <0) {y = -chan; break;}
				if (debug) fprintf(stderr, "opening directory '%s'\n", filespec, mode);
				DIR *dp = opendir(filespec);
				if (dp == NULL) { y = seterror(0, errno); break;}
				if (debug) fprintf(stderr, "directory opened on channel %d\n", chan);
				y = setiocb(chan, -1, TDIR, ' ', 0, NULL, dp, filespec); //Setup IOCB      
				if (y == 0) x = chan;
			} else {
				m = x >> 5 & 7; //Get Mode Index
				x = 0; //File channel (none)
				y = ENOERROR; //Error code (none)
				strcpy(mode, modes[m]); //Set Mode from Mode Index
				chan = uniocb(); if (chan <0) {y = seterror(0, -chan); break;}
				if (debug) fprintf(stderr, "opening file '%s' with mode '%s'\n", filename, mode);
				FILE *fp = fopen(filename, mode);
				if (fp == NULL) { y = seterror(0, errno); break;}
				if (debug) fprintf(stderr, "file opened on channel %d\n", chan);
				i = (m > 5) ? fileindx : 0; //Set Records Size if Mode RECORD
				y = setiocb(chan, -1, TFILE, mode[0], i, fp, NULL, filename); //Setup IOCB      
				if (y == 0) x = chan;
		  }
      break;
    case 'P': //Put character - Y = channel; X = character
      chan = y;
      c = x;
      if (debug) fprintf(stderr, "writing '%c' to channel %d\n", c, chan);
      x = 0xFF; //Character written (Error)
      y = valchan(chan, TFILE, stdout); if (y) break;
      e = fputc(c, iocbs[chan].fp);
      if (e == EOF) {y = seterror(chan, errno); break;}
      x = e & 0xFF;
      break;
    case 'Q': //Put String - Y = channel, Carry Set = putline
      chan = y;
      x = 0; //Number of characters written
      y = valchan(chan, TFILE, stdout); if (y) break;
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
        a = valchan(chan, TFILE, NULL); if (a) break;
        if (debug) fprintf(stderr, "selecting record #%d\n", fileindx);
        n = iocbs[chan].recsize;
        i = fileindx * n;
        if (debug) fprintf(stderr, "setting position to %d\n", i);
        e = fseek(iocbs[chan].fp, i, SEEK_SET);
        if (e) {a = seterror(chan, errno); break;}
      } else {
        y = valchan(chan, TFILE, NULL); if (y) break;
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
      //CHANGE INPUT PARAMETER TO DRIVE ID
      a = ENOERROR; //Error (none)
      if (debug) fprintf(stderr, "saving file from %04x to %04x\n", fileaddr, yx);
      e = save(mpu, fileaddr, yx-fileaddr-1, filename);
      if (!e) {a = seterror(0, errno); break;}
      y = e >> 8; x = e & 0xff;
      break;
    //case 'T': //Unused
    case 'U': //Get/Set Current Directory - Y = Drive/Disk ID, CC=Get. CS=Set
      if (p & 1) { //Change Directory
        d = driveid(y);
        y = setspec(filespec, d, filename, ".");  if (y) break;
        if (debug) fprintf(stderr, "changing directory to \"%s\"\n", filespec);
        if (_chdir(filespec)) {y = seterror(0, errno);}
      } else { //Get Directory
        x = 0; filename[0] = 0; //Clear filename, Set Length to 0
        d = driveno(y); if (d < 0) {y=seterror(0, -d); break;}
        e = _getdrive(); if (e == 0) {y=seterror(0, errno); break;}
        if (d && e != d ) {y = chgdrive(d); if (y) break;}
        if (debug) fprintf(stderr, "getting current working directory for drive %d\n", d);
        if (_getcwd(filename, STRLEN) == NULL) y = seterror(0, errno);
        else { 
          if (debug) fprintf(stderr, "current working directory ie \"%s\"\n", filename);
          x = writestr(mpu, filename, STRLEN);
        }  
        if (d && e != d ) {e = chgdrive(e); if (e) seterror(0, errno);}
      }
      break;
    case 'V': //Get or Set Current Drive, Y=Drive, Carry=Get/Set
      if (p & 1) {
        if (debug) fprintf(stderr, "changing drive to %c\n", y+'@');
        x = _chdrive(y); if (x) y=seterror(0, errno); else y=0;
      } else {
        x = _getdrive(); if (x) y=0; else y=seterror(0, errno);
        if (debug) fprintf(stderr, "current drive is %c\n", x+'@');
      }
      break;
    case 'W': //Write bytes - Y = channel, X = Number of Bytes
      m = p & 1; //Set Mode: 0 = fread, 1 = fgetr
      chan = y;
      if (m) {
        a = valchan(chan, TFILE, NULL); if (a) break;
        if (debug) fprintf(stderr, "selecting record #%d\n", fileindx);
        n = iocbs[chan].recsize;
        i = fileindx * n;
        if (debug) fprintf(stderr, "setting position to %d\n", i);
        e = fseek(iocbs[chan].fp, i, SEEK_SET);
        if (e) {a = seterror(chan, errno); break;}
        i = ftell(iocbs[chan].fp);
        if (debug) fprintf(stderr, "position set to %d\n", i);
      } else {
        y = valchan(chan, TFILE, NULL); if (y) break;
        n = x;
      }
      for (i = 0; i<n; i++) 
        filebuff[i] = mpu->memory[fileaddr+i];
      if (debug) fprintf(stderr, "writing %d bytes\n", n);
      e = fwrite(filebuff, n, 1, iocbs[chan].fp);
      if (debug) fprintf(stderr, "fwrite() returned %d\n", e);
      if (e == 0) {e = seterror(chan, errno); if (m) a = e; else y = e; break; }
      if (m) { 
        fileindx++; y = fileindx >> 8; x = fileindx & 0xff;
        if (debug) fprintf(stderr, "returning record #%d\n", fileindx);
      }
      break;
    case 'X': //Make/Remove Directory - Y = Drive ID, CC = RMDIR, CS = MKDIR
			d = driveid(y);
      m = p & 1; 
      y = setspec(filespec, d, filename, ".");  if (y) break;
      if (debug) fprintf(stderr, "%s directory '%s'\n", ddesc[m], filespec);
      if (m) e = _rmdir(filespec); 
      else e = _mkdir(filespec);
      if (e) y=seterror(0, errno);
      break;
    case 'Y': //Get Last Error: Y=chan
      chan = y;
      y = ENOERROR; //Set Error to None
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
      y = valchan(chan, TFILE, NULL); if (y) break;
      if (p & 1) {
        e = ftell(iocbs[chan].fp);
        if (e < 0) {a = seterror(chan, errno);}
        else {
          if (debug) fprintf(stderr, "returning position %d\n", e);
          a = ENOERROR; 
        }
        y = e >> 8; x = e & 0xff;
      } else {
        if (fileindx == 0xFFFF) {i = 0; m = SEEK_END;}
        else {i = fileindx, m = SEEK_SET;}
        if (debug) fprintf(stderr, "seeking position %d\n", i);
        e = fseek(iocbs[chan].fp, i, m);
				if (e) y = seterror(chan, errno);
      }
      break;
    default: 
      y = 22; //Error - invalid argument
  }
  if (debug) fprintf(stderr, "returning values %02x, %02x, %02x, %02x\n", a, y, x, p);
  mpu->registers->a = a;
  mpu->registers->x = x;
  mpu->registers->y = y;
}

/* Emulate system command dispatch at addr */
extern int syscmd(M6502 *mpu, word addr, byte data)	{ 
  struct tm time;
  dirent *de;
  byte a = mpu->registers->a;
  byte x = mpu->registers->x;
  byte y = mpu->registers->y;
  byte p = mpu->registers->p;
  word yx = y << 8 | x;
  if (debug) fprintf(stderr, "executing system command '%c' with options %02x,%02x,%02x\n", a, y, x, p);
  switch(a) {    
    case 'C': //Get System Clock
      if (debug) fprintf(stderr, "reading system clock\n");
      clock_t clk = clock(); //Get System Clock
      if (debug) fprintf(stderr, "system clock = %d ticks\n", clk);      
      int i = (clk / 50) & 0xFFFFFF; //Convert to 24-bit integer, 50 ticks per second
      a = i >> 16; y = i >> 8; x = i;    //with resolution of 50 ticks per second
      if (debug) fprintf(stderr, "returning $%02x%02x%02x (%d)\n", a, y, x, clk);
      break;
    case 'T': //Get/Set Date and Time - YX = tm address; CC = Get, CS = Set 
      if (p & 1) {
        y = 0xFF; //Return Error - Not Implemented
      } else {
        fileaddr = yx; //Set Struct Address
        _getsystime(&time);
        x = copytime(filebuff, &time, 0); //Convert time struct
        writemem(mpu, filebuff, x); //Write to Memory
        y = 0; //Return Error = None
      }
      break;
    default:
      y = 22; //Error - invalid argument
  }
  if (debug) fprintf(stderr, "returning values %02x, %02x, %02x, %02x\n", a, y, x, p);
  mpu->registers->a = a;
  mpu->registers->x = x;
  mpu->registers->y = y;
}

/* Initialize Extended RAM */
extern void initxmem(void) {
  for (int i=0; i<16; i++) xmemory[i] = (byte *) malloc(0x10000);
  xmembank = 0;
  xmemaddr = 0;
}

/* Increment Ext Memory Address and Page */
static void xmemnext() {
  xmemaddr++; 
  if (xmemaddr > 0xFFFF) {xmembank++; xmemaddr = 0;}
  if (xmembank > 0x0F) xmembank = 0;
}
  
/* Read/Write Extended Memory */
static byte readxmem(int inc) {
  byte b = xmemory[xmembank][xmemaddr];
  //if (debug) fprintf(stderr, "read byte %d from bank %02x address %04x\n", b, xmembank, xmemaddr);
  if (inc) xmemnext();
  return b;
}
static void writexmem(byte b, int inc) {
  //if (debug) fprintf(stderr, "writing byte %d to bank %02x address %04x\n", b, xmembank, xmemaddr);
  xmemory[xmembank][xmemaddr] = b;
  if (inc) xmemnext();
}

/* Emulate extended memory command dispatch at addr */
extern int xmemcmd(M6502 *mpu, word addr, byte data)	{ 
  char mode[2][8] = {"reading", "writing"};
  byte a = mpu->registers->a;
  byte x = mpu->registers->x;
  byte y = mpu->registers->y;
  byte p = mpu->registers->p;
  word yx = y << 8 | x;
  byte cs = (p & 1);
  if (debug) fprintf(stderr, "executing ext memory command '%c' with options %02x,%02x,%02x\n", a, y, x, p);
  switch(a) {
    case 'A': //Get/Set Address - YX = address, CC = get, CS = set
      if (cs) { 
        xmemaddr = yx; 
        if (debug) fprintf(stderr, "set extended memory address to $%04x\n", xmemaddr);
      } else {
        if (debug) fprintf(stderr, "returning extended memory address $%04x\n", xmemaddr);
        y = xmemaddr >> 8; x = xmemaddr & 0xFF;
      }
      break;
    case 'B': //Get/Set Bank - X/A = bank, CC = get, CS = set
      if (cs) {
        xmembank = x & 0x0F; 
        if (debug) fprintf(stderr, "set extended memory bank to $%02x\n", xmembank);
      } else {
        if (debug) fprintf(stderr, "returning extended memory bank $%02x\n", xmembank);
        a = xmembank;
      }
      break;
    case 'C': //Read/Write Byte w/o Increment - X/A = Byte, CC = Read, CS = Write
      if (cs) writexmem(x,0); else a = readxmem(0);
      break;
    case 'M': //Read/Write Bytes to RAM - YX = Byte Count, CC = Read, CS = Write 
      if (debug) fprintf(stderr, "%s %d bytes: %02x-%04x <-> %04x\n{", mode[cs], yx, xmembank, xmemaddr, smemaddr);
      for (int i=0; i<yx; i++) {
        if (cs) writexmem(mpu->memory[smemaddr], -1);
        else mpu->memory[smemaddr] = readxmem(-1);
        if (debug) {if (i) putc(',', stderr); fprintf(stderr, "%d", mpu->memory[smemaddr]);}
        smemaddr++; if (smemaddr > 0xFFFF) smemaddr = 0;
      }
      if (debug) fprintf(stderr, "}\n");
      a = xmembank; y = xmemaddr >> 8; x = xmemaddr & 0xFF;
      break;
    case 'N': //Read/Write Next Byte - X/A = Byte, CC = Read, CS = Write
      if (cs) writexmem(x,-1); else a = readxmem(-1);
      break;
    case 'S': //Set System RAM Address - YX = address
      smemaddr = yx; 
      if (debug) fprintf(stderr, "set system memory address to $%04x\n", smemaddr);
      break;
    case 'W': //Read/Write Next Word  - YX = Word, CC = Read, CS = Write
      if (cs) {writexmem(x,-1); writexmem(y,-1);}
      else {x = readxmem(-1); y = readxmem(-1);}
      break;
    case 'X': //Read/Write Bytes to RAM - YX = Byte Count, CC = Read, CS = Write 
      if (debug) fprintf(stderr, "swapping %d bytes: %02x-%04x <-> %04x\n{", yx, xmembank, xmemaddr, smemaddr);
      for (int i=0; i<yx; i++) {
        byte temp = mpu->memory[smemaddr];
        mpu->memory[smemaddr] = readxmem(-1);
        writexmem(temp, -1);
        smemaddr++; if (smemaddr > 0xFFFF) smemaddr = 0;
      }
      a = xmembank; y = xmemaddr >> 8; x = xmemaddr & 0xFF;
      break;
    default:
      if (debug) fprintf(stderr, "invalid command '%x'\n", a);
  }
  if (debug) fprintf(stderr, "returning values %02x, %02x, %02x, %02x\n", a, y, x, p);
  mpu->registers->a = a;
  mpu->registers->x = x;
  mpu->registers->y = y;
}

/* Read Keys Directly from Console, Eliminating getc() buffering */
extern int getkey(M6502 *mpu, word addr, byte data)	{ 
  int a = 0, y=0, x=0; //Initialize Key Press to None
  int p = mpu->registers->p;
  if (_kbhit()) {
    y = _getch();
    if (y == 0 || y == 0xE0) {      
      x = _getch(); //Get Extended Code
      a = x | 0x80; //Convert to High ASCII
    } else { 
      a = y;
    }
  }
  if (a) p = p & 0xFD; else p = p | 0x02; //Set Z Flag
  if (a > 127) p = p | 0x80; else p = p & 0x7F; //Set N Flag
  mpu->registers->a = a;
  mpu->registers->x = x;
  mpu->registers->y = y;
  mpu->registers->p = p;
}

/* Write Character Directly to Console, Eliminating putc() buffering */
extern int putcon(M6502 *mpu, word addr, byte data)	{ 
  byte a = mpu->registers->a;
  switch (a) {
    case 0xD6: a = _cputs("\e[2J"); break;
    default: a = _putch(a);
  }
  mpu->registers->a = a;
}
