/* file6502.h - File I/O Command Processor Header File */

#include "lib6502.h"

typedef uint8_t  byte;
typedef uint16_t word;

extern void setdebug(int dbg);


/**********************************************************************************************
 * Process File Command                                                                       *
 * Command passed in A and parameters in X, Y, and Carry                                      *
 *  16-Bit values are passed with the MSB in Y and the LSB in X                               *
 * Results returned in A, X, and Y                                                            *
 *  Unless otherwise specified, A contains the Command Code                                   *
 *  Status of 0 indicates success, any other value indicates failure                          *
 * For FOPEN only bits 5-7 of the File Mode are significant; bits 0-4 are ignored             *
 *  Bits 6-7 are the access mode: %00=Read, %01=Write, %10=Append, %11=Records                *
 *  Bit 5 is the access type: 0=text, 1=binary                                                *
 *                                                                                            *
 * A=Command  Description                     Parameters          Returns                     *
 * A SETADDR  Set File Buffer Address         Y,X=Address         Y,X=Address                 *
 * C FCLOSR   Close File (Carry Clear)        Y=Channel           Y=Error                     *
 *   CLOSEDIR Close Directory (Carry Set)     Y=Channel           Y=Error                     *
 * D READDIR  Read Dir Entry (Carry Clear)    Y=Channel           X=Name Length, Y=Error      *
 *            SETADDR Struct Address                                                          *
 *   READHDR  Read Dir Header (Carry Set)     Y=Channel           X=Name Length, Y=Error      *
 *            SETADDR Struct Address                                                          *
 * E FEOF     Check for End of File           Y=Channel           Y=EOF Indicator             *
 * F FFLUSH   Flush File to Disk              Y=Channel           X=Status, Y=Error           *
 * G FGETC    Read Character from File        Y=Channel           X=Character, Y=Error        *
 * H FGETS    Read String from File           Y=Channel           X=String Legth, Y=Error     *
 *            SETADDR Array Address                                                           *
 * I FSINIT   Initialize File System                                                          *
 * G FGETW    Read Integer (Carry Clear)      Y=Channel           Y.X=Integer, A=Error        *
 *   FPUTW    Write Integer (Carry Set)       Y=Channel           Y=Error                     *
 *            SETADDR Struct Address                                                          *
 * K REMOVE   Remove File from Disk           Y=Drive             Y=Error                     *
 *            SETNAME Filename                                                                *
 * L FLOAD    Load File into Memory                               Y,X=End Address, A=Error    *
 *            SETNAME File Name                                                               *
 *            SETADDR Start Address                                                           *
 * M RENAME   Change Name of File on Disk     Y=Drive             Y=Error                     *
 *            SETNAME Old Name                                                                *
 *            SETBUFF New Name                                                                *
 * N SETNAME  Set Filename (Carry Clear)      YX=Name             YX=Name                     *
 *   SETBUFF  Set File Buffer (Carry Set)     YX=String           YX=String                   *
 * O FOPEN    Open File (Carry Clear)         Y=Drive, X=Mode     X=Channel, Y=Error          *
 *            SETNAME File Name                                                               *
 *            SETINDEX Record Length (MRECRD)                                                 *
 *   OPENDIR  Open Directory (Carry Set)      Y=Drive             X=Channel, Y=Error          *
 *            SETNAME Directory Name                                                          *
 * P FPUTC    Write Character to File         Y=Channel, X=Char   E=Status, Y=Error           *
 * Q FPUTS    Write String (Carry Clear)      Y=Channel           X=String Length, Y=Error    *
 *            SETADDR String Address                                                          *
 *   FPUTLN   Write Line (Carry Set)          Y=Channel           X=String Length, Y=Error    *
 *            SETADDR String Address                                                          *
 * R FREAD    Read Bytes (Carry Clear)        Y=Channel, X=Count  X=Count, Y=Error            *
 *            SETADDR Array Address                                                           *
 *   FGETR    Read Record (Carry Set)         Y=Channel           A=Error, YX=Next Index      *
 *            SETADDR Struct                                                                  *
 *            SETINDEX Record Address                                                         *
 * S FSAVE    Save File to Disk               Y,X=End Address     Y=Error                     *
 *            SETNAME File Name                                                               *
 *            SETADDR Start Address                                                           *
 * U GETCWD   Get Current Dir (Carry Clear)   Y=Drive ID          X=Name Length, Y=Error      *
 *            SETADDR String Buffer                                                           *
 * U CHDIR    Change Directory (Carry Set)    Y=Drive ID          Y=Error                     *
 *            SETNAME Directory Name                                                          *
 * V GETDRIVE Get Current Drive (Carry Clear)                     X=Drive ID, Y=Error         *
 *   CHDRIVE  Set Current Drive (Carry Set)   Y=Drive ID          Y=Error                     *
 * W FWRITE   Write Bytes (Carry Clear)       Y=Channel, X=Count  X=Count, Y=Error            *
 * X MKDIR    Create Directory (Carry Clear)  Y=Drive ID          Y=Error                     *
 *            SETNAME Directory Name                                                          *
 *   RMDIR    Remove Directory (Carry Set)    Y=Drive ID          Y=Error                     *
 *            SETNAME Directory Name                                                          *
 * Y FERROR   Get Last Error on Channel       Y=Channel           X=Channel Error, Y = Error  *
 *            SETADDR Array Address                                                           *
 * Z FSEEK    Move to Position (Carry Clear)  Y=Channel           X=Status, Y=Error           *
 *            SETINDEX Position                                                               *
 *   FTELL    Get File Position (Carry Clear) Y=Channel           A=Error, YX=Position        *
 **********************************************************************************************/
extern int filecmd(M6502 *mpu, word addr, byte data);

/**********************************************************************************************
 * Process System Command                                                                     *
 * Command passed in A and parameters in X, Y, and Carry                                      *
 *  16-Bit values are passed with the MSB in Y and the LSB in X                               *
 * Results returned in A, X, and Y                                                            *
 *  Unless otherwise specified, A contains the Command Code                                   *
 *                                                                                            *
 * A=Command  Description                     Parameters          Returns                     *
 * C CLOCK    Get System Clock                                    A,Y,X=Clock (Seconds/50)    *
 * T GETTM    Get Date/Time (Carry Clear)     Y,X = tm Address                                *
 *   SETTM    Set Date/Time Time (Carry Set   Y,X = tm Address    Y=Error                     *           *
 **********************************************************************************************/
extern int syscmd(M6502 *mpu, word addr, byte data);

/**********************************************************************************************
 * Read Key Directly from Console                                                             *
 * Bypasses getc() from stdin, eliminating input buffering                                  *
 * Returns ASCII key value in A, extended Key Code in Y and X                                 *
 *   Cursor Control Keys produce an ASCII value with the high bit set                         *
 **********************************************************************************************/
extern int getkey(M6502 *mpu, word addr, byte data);

/**********************************************************************************************
 * Write Character Directly to Console                                                        *
 * Bypasses putc() from stdout, eliminating output buffering                                  *
 * Character Code is passed in A                                                              *
 **********************************************************************************************/
extern int putcon(M6502 *mpu, word addr, byte data);
