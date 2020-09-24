/* file6502.h - File I/O Command Processor Header File */

#include "lib6502.h"

typedef uint8_t  byte;
typedef uint16_t word;

extern void setdebug(int dbg);
extern int filecmd(M6502 *mpu, word addr, byte data);
