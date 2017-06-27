/*  Assembler for the MOS Technology 650X series of microprocessors
 *  Written by J. H. Van Ornum (201) 949-1781
 *		AT&T Bell Laboratories
 *		 Holmdel, NJ
 *
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#define LAST_CH_POS	132
#define SFIELD	23
#define STABSZ	6000
#define SBOLSZ	20

/*
 * symbol flags
 */
#define DEFZRO	2	/* defined - page zero address	*/
#define MDEF	3	/* multiply defined		*/
#define UNDEF	1	/* undefined - may be zero page */
#define DEFABS	4	/* defined - two byte address	*/
#define UNDEFAB 5	/* undefined - two byte address */

/*
 * operation code flags
 */
#define PSEUDO	0x6000
#define CLASS1	0x2000
#define CLASS2	0x4000
#define IMM1	0x1000		/* opval + 0x00	2 byte	*/
#define IMM2	0x0800		/* opval + 0x08	2 byte	*/
#define ABS	0x0400		/* opval + 0x0C	3 byte	*/
#define ZER	0x0200		/* opval + 0x04	2 byte	*/
#define INDX	0x0100		/* opval + 0x00	2 byte	*/
#define ABSY2	0x0080		/* opval + 0x1C	3 byte	*/
#define INDY	0x0040		/* opval + 0x10	2 byte	*/
#define ZERX	0x0020		/* opval + 0x14	2 byte	*/
#define ABSX	0x0010		/* opval + 0x1C	3 byte	*/
#define ABSY	0x0008		/* opval + 0x18	3 byte	*/
#define ACC	0x0004		/* opval + 0x08	1 byte	*/
#define IND	0x0002		/* opval + 0x2C	3 byte	*/
#define ZERY	0x0001		/* opval + 0x14	2 byte	*/

/*
 * pass flags
 */
#define FIRST_PASS	0
#define LAST_PASS	1
#define DONE		2

FILE	*optr;
FILE	*iptr;
int	dflag;			/* debug flag */
int	errcnt;			/* error counter */
int	hash_tbl[128];		/* pointers to starting links in symtab */
char	hex[5];			/* hexadecimal character buffer */
int	iflag;			/* ignore .nlst flag */
int	lablptr;		/* label pointer into symbol table */
int	lflag;			/* disable listing flag */
int	loccnt;			/* location counter	*/
int	nflag;			/* normal/split address mode */
int	nxt_free;		/* next free location in symtab */
int	objcnt;			/* object byte counter */
int	oflag;			/* object output flag */
int	opflg;			/* operation code flags */
int	opval;			/* operation code value */
int	pass;			/* pass counter		*/
char	prlnbuf[LAST_CH_POS+1]; /* print line buffer	*/
int	sflag;			/* symbol table output flag */
int	slnum;			/* source line number counter */
char	symtab[STABSZ];		/* symbol table		*/
				/* struct sym_tab		*/
				/* {	char	size;		*/
				/*	char	chars[size];	*/
				/*	char	flag;		*/
				/*	int	value;		*/
				/*	int	next_pointer	*/
				/* }				*/
char	symbol0[SBOLSZ];		/* temporary symbol storage	*/
int	udtype;			/* undefined symbol type	*/
int	undef;			/* undefined symbol in expression flg  */
int	value;			/* operand field value */
char	zpref;			/* zero page reference flag	*/


#define A	0x20)+('A'&0x1f))
#define B	0x20)+('B'&0x1f))
#define C	0x20)+('C'&0x1f))
#define D	0x20)+('D'&0x1f))
#define E	0x20)+('E'&0x1f))
#define F	0x20)+('F'&0x1f))
#define G	0x20)+('G'&0x1f))
#define H	0x20)+('H'&0x1f))
#define I	0x20)+('I'&0x1f))
#define J	0x20)+('J'&0x1f))
#define K	0x20)+('K'&0x1f))
#define L	0x20)+('L'&0x1f))
#define M	0x20)+('M'&0x1f))
#define N	0x20)+('N'&0x1f))
#define O	0x20)+('O'&0x1f))
#define P	0x20)+('P'&0x1f))
#define Q	0x20)+('Q'&0x1f))
#define R	0x20)+('R'&0x1f))
#define S	0x20)+('S'&0x1f))
#define T	0x20)+('T'&0x1f))
#define U	0x20)+('U'&0x1f))
#define V	0x20)+('V'&0x1f))
#define W	0x20)+('W'&0x1f))
#define X	0x20)+('X'&0x1f))
#define Y	0x20)+('Y'&0x1f))
#define Z	0x20)+('Z'&0x1f))

#define OPSIZE	63

int	optab[]	=		/* nmemonic  operation code table	*/
{				/* '.' = 31, '*' = 30, '=' = 29		*/
	((0*0x20)+(29)),PSEUDO,1,
	((((0*0x20)+(30))*0x20)+(29)),PSEUDO,3,
	((((((0*A*D*C,IMM2|ABS|ZER|INDX|INDY|ZERX|ABSX|ABSY,0x61,
	((((((0*A*N*D,IMM2|ABS|ZER|INDX|INDY|ZERX|ABSX|ABSY,0x21,
	((((((0*A*S*L,ABS|ZER|ZERX|ABSX|ACC,0x02,
	((((((0*B*C*C,CLASS2,0x90,
	((((((0*B*C*S,CLASS2,0xb0,
	((((((0*B*E*Q,CLASS2,0xf0,
	((((((0*B*I*T,ABS|ZER,0x20,
	((((((0*B*M*I,CLASS2,0x30,
	((((((0*B*N*E,CLASS2,0xd0,
	((((((0*B*P*L,CLASS2,0x10,
	((((((0*B*R*K,CLASS1,0x00,
	((((((0*B*V*C,CLASS2,0x50,
	((((((0*B*V*S,CLASS2,0x70,
	((((((0*C*L*C,CLASS1,0x18,
	((((((0*C*L*D,CLASS1,0xd8,
	((((((0*C*L*I,CLASS1,0x58,
	((((((0*C*L*V,CLASS1,0xb8,
	((((((0*C*M*P,IMM2|ABS|ZER|INDX|INDY|ZERX|ABSX|ABSY,0xc1,
	((((((0*C*P*X,IMM1|ABS|ZER,0xe0,
	((((((0*C*P*Y,IMM1|ABS|ZER,0xc0,
	((((((0*D*E*C,ABS|ZER|ZERX|ABSX,0xc2,
	((((((0*D*E*X,CLASS1,0xca,
	((((((0*D*E*Y,CLASS1,0x88,
	((((((0*E*O*R,IMM2|ABS|ZER|INDX|INDY|ZERX|ABSX|ABSY,0x41,
	((((((0*I*N*C,ABS|ZER|ZERX|ABSX,0xe2,
	((((((0*I*N*X,CLASS1,0xe8,
	((((((0*I*N*Y,CLASS1,0xc8,
	((((((0*J*M*P,ABS|IND,0x40,
	((((((0*J*S*R,ABS,0x14,
	((((((0*L*D*A,IMM2|ABS|ZER|INDX|INDY|ZERX|ABSX|ABSY,0xa1,
	((((((0*L*D*X,IMM1|ABS|ZER|ABSY2|ZERY,0xa2,
	((((((0*L*D*Y,IMM1|ABS|ZER|ABSX|ZERX,0xa0,
	((((((0*L*S*R,ABS|ZER|ZERX|ABSX|ACC,0x42,
	((((((0*N*O*P,CLASS1,0xea,
	((((((0*O*R*A,IMM2|ABS|ZER|INDX|INDY|ZERX|ABSX|ABSY,0x01,
	((((((0*P*H*A,CLASS1,0x48,
	((((((0*P*H*P,CLASS1,0x08,
	((((((0*P*L*A,CLASS1,0x68,
	((((((0*P*L*P,CLASS1,0x28,
	((((((0*R*O*L,ABS|ZER|ZERX|ABSX|ACC,0x22,
	((((((0*R*O*R,ABS|ZER|ZERX|ABSX|ACC,0x62,
	((((((0*R*T*I,CLASS1,0x40,
	((((((0*R*T*S,CLASS1,0x60,
	((((((0*S*B*C,IMM2|ABS|ZER|INDX|INDY|ZERX|ABSX|ABSY,0xe1,
	((((((0*S*E*C,CLASS1,0x38,
	((((((0*S*E*D,CLASS1,0xf8,
	((((((0*S*E*I,CLASS1,0x78,
	((((((0*S*T*A,ABS|ZER|INDX|INDY|ZERX|ABSX|ABSY,0x81,
	((((((0*S*T*X,ABS|ZER|ZERY,0x82,
	((((((0*S*T*Y,ABS|ZER|ZERX,0x80,
	((((((0*T*A*X,CLASS1,0xaa,
	((((((0*T*A*Y,CLASS1,0xa8,
	((((((0*T*S*X,CLASS1,0xba,
	((((((0*T*X*A,CLASS1,0x8a,
	((((((0*T*X*S,CLASS1,0x9a,
	((((((0*T*Y*A,CLASS1,0x98,
	((((((0*0x20)+(31))*W*O^((((0*R*D,PSEUDO,2,	/* 0x7cab */
	((((((0*0x20)+(31))*B*Y^((((0*T*E,PSEUDO,0,	/* 0x7edc */
	((((((0*0x20)+(31))*D*B^((((0*Y*T,PSEUDO,6,	/* 0x7fb6 */
	((((((0*0x20)+(31))*N*L^((((0*S*T,PSEUDO,5,	/* 0x7fb8 */
	((((((0*0x20)+(31))*L*I^((((0*S*T,PSEUDO,4,	/* 0x7ffd */
	0x7fff,0,0,
	0x7fff,0,0
};

int	step[] =
{
	3*((OPSIZE+1)/2),
	3*((((OPSIZE+1)/2)+1)/2),
	3*((((((OPSIZE+1)/2)+1)/2)+1)/2),
	3*((((((((OPSIZE+1)/2)+1)/2)+1)/2)+1)/2),
	3*(2),
	3*(1),
	0
};


#define CPMEOF EOF

/*
 *  Two changes to version 1.4 have been made to "port" as6502 to CP/M(tm).
 *  A "tolower()" function call was add to the command line processing
 *  code to (re)map the command line arguments to lower case (CP/M
 *  converts all command line arguments to upper case).  The readline()
 *  function has code added to "ignore" the '\r' character (CP/M includes
 *  the \r character along with \n).
 *
 *  Also, the ability to process multiple files on the command line has been
 *  added.  Now one can do, for example:
 *
 *	as6502 -nisvo header.file source.file data.file ...
 *
 *    George V. Wilder
 *	IX 1A-360 x1937
 *	ihuxp!gvw1
 */

/*
 *  This Macintosh port (1.41) by RTK, Feb. 27, 1995.  Added Mac console
 *  control and removed CP/M checks.
 *
 */

main(argc, argv)
   int	argc;
   char *argv[];
{
	char	c;
	int	cnt;
	int	i;
	int	ac;
	char	**av;

	while (--argc > 0 && (*++argv)[0] == '-') {
		for (i = 1; (c = tolower((*argv)[i])) != '\0'; i++) {
			if (c == 'd')		/* debug flag */
				dflag++;
			if (c == 'i')		/* ignore .nlst flag */
				iflag++;
			if (c == 'l')		/* disable listing flag */
				lflag--;
			if (c == 'n')		/* normal/split address mode */
				nflag++;
			if (c == 'o')		/* object output flag */
				oflag++;
			if (c == 's')		/* list symbol table flag */
				sflag++;
			if (c == 'v')		/* print assembler version */
				fprintf(stdout, "as6502 - version 4.1b - 11/22/84 - JHV [gvw]\n");
		}
	}
	ac = argc;
	av = argv;
	pass = FIRST_PASS;
	for (i = 0; i < 128; i++)
		hash_tbl[i] = -1;
	errcnt = loccnt = slnum = 0;
	while (pass != DONE) {
		initialize(ac, av, argc);
		if(pass == LAST_PASS && ac == argc)
			errcnt = loccnt = slnum = 0;
		while (readline() != -1)
			assemble();
		if (errcnt != 0) {
			pass = DONE;
			fprintf(stdout, "Terminated with error counter = %d\n", errcnt);
		}
		switch (pass) {
		case FIRST_PASS:
			--ac;
			++av;
			if(ac == 0) {
				pass = LAST_PASS;
				if (lflag == 0)
					lflag++;
				ac = argc;
				av = argv;
			}
			break;
		case LAST_PASS:
			--ac;
			++av;
			if(ac == 0) {
				pass = DONE;
				if (sflag != 0)
					stprnt();
			}
		}
		wrapup();
		if ((dflag != 0) && (pass == LAST_PASS)) {
			fprintf(stdout, "nxt_free = %d\n", nxt_free);
			cnt = 0;
			for (i = 0; i < 128; i++)
				if (hash_tbl[i] == -1)
					cnt++;
			fprintf(stdout, "%d unused hash table pointers out of 128\n", cnt);
		}
	}
	return(0);
}

/*****************************************************************************/

/* initialize opens files */

initialize(ac, av, argc)
   int	ac;
   char *av[];
   int  argc;
{

	if (ac == 0) {
		fprintf(stdout, "Invalid argument count (%d).\n", argc);
		exit(1);
	}
	if ((iptr = fopen(*av, "r")) == NULL) {
		fprintf(stdout, "Open error for file '%s'.\n", *av);
		exit(1);
	}
	if ((pass == LAST_PASS) && (oflag != 0) && ac == argc) {
		if ((optr = fopen("6502.out", "w")) == NULL) {
			fprintf(stdout, "Create error for object file 6502.out.\n");
			exit(1);
		}
	}
}

/* readline reads and formats an input line	*/

int	field[] =
{
	SFIELD,
	SFIELD + 8,
	SFIELD + 14,
	SFIELD + 23,
	SFIELD + 43,
	SFIELD + 75
};

readline()
{
	int	i;		/* pointer into prlnbuf */
	int	j;		/* pointer to current field start	*/
	int	ch;		/* current character		*/
	int	cmnt;		/* comment line flag	*/
	int	spcnt;		/* consecutive space counter	*/
	int	string;		/* ASCII string flag	*/
	int	temp1;		/* temp used for line number conversion */

	temp1 = ++slnum;
	for (i = 0; i < LAST_CH_POS; i++)
		prlnbuf[i] = ' ';
	i = 3;
	while (temp1 != 0) {	/* put source line number into prlnbuf */
		prlnbuf[i--] = temp1 % 10 + '0';
		temp1 /= 10;
	}
	i = SFIELD;
	cmnt = spcnt = string = 0;
	j = 1;
	while ((ch = getc(iptr)) != '\n') {
		prlnbuf[i++] = ch;
		if ((ch == ' ') && (string == 0)) {
			if (spcnt != 0)
				--i;
			else if (cmnt == 0) {
				++spcnt;
				if (i < field[j])
					i = field[j];
				if (++j > 3) {
					spcnt = 0;
					++cmnt;
				}
			}
		}
		else if (ch == '\t') {
			prlnbuf[i - 1] = ' ';
			spcnt = 0;
			if (cmnt == 0) {
				if (i < field[j])
					i = field[j];
				if (++j > 3)
					++cmnt;
			}
			else i = (i + 8) & 0x78;
		}
		else if ((ch == ';') && (string == 0)) {
			spcnt = 0;
			if (i == SFIELD + 1)
				++cmnt;
			else if (prlnbuf[i - 2] != '\'') {
				++cmnt;
				prlnbuf[i-1] = ' ';
				if (i < field[3])
					i = field[3];
				prlnbuf[i++] = ';';
			}
		}
		else if (ch == EOF || ch == CPMEOF)
			return(-1);
		else {
			if ((ch == '"') && (cmnt == 0))
				string = string ^ 1;
			spcnt = 0;
			if (i >= LAST_CH_POS - 1)
				--i;
		}
	}
	prlnbuf[i] = 0;
	return(0);
}

/*
 * wrapup() closes the source file
 */

wrapup()
{

	fclose(iptr);
	if ((pass == DONE) && (oflag != 0)) {
		fputc('\n', optr);
		fclose(optr);
	}
	return;
}

/* symbol table print
 */

stprnt()
{
	int	i;
	int	j;
	int	k;

	fputc('\014', stdout);
	fputc('\n', stdout);
	i = 0;
	while	((j = symtab[i++]) != 0) {
		for (k = j; k > 0; k--)
			fputc(symtab[i++], stdout);
		for (k = 20 - j; k > 0; k--)
			fputc(' ', stdout);
		++i;
		j = (symtab[i++] & 0xff);
		j += (symtab[i++] << 8);
		hexcon(4, j);
		fprintf(stdout, "\t%c%c:%c%c\t%c%c%c%c\n",
			hex[3], hex[4], hex[1], hex[2],
			hex[1], hex[2], hex[3], hex[4]);
		i += 2;
	}
}


int	optab[];
int	step[];

/* translate source line to machine language */

assemble()
{
	int	flg;
	int	i;		/* prlnbuf pointer */

	if ((prlnbuf[SFIELD] == ';') | (prlnbuf[SFIELD] == 0)) {
		if (pass == LAST_PASS)
			println();
		return;
	}
	lablptr = -1;
	i = SFIELD;
	udtype = UNDEF;
	if (colsym(&i) != 0 && (lablptr = stlook()) == -1)
		return;
	while (prlnbuf[++i] == ' ');	/* find first non-space */
	if ((flg = oplook(&i)) < 0) {	/* collect operation code */
		labldef(loccnt);
		if (flg == -1)
			error("Invalid operation code");
		if ((flg == -2) && (pass == LAST_PASS)) {
			if (lablptr != -1)
				loadlc(loccnt, 1, 0);
			println();
		}
		return;
	}
	if (opflg == PSEUDO)
		pseudo(&i);
	else if (labldef(loccnt) == -1)
		return;
	else {
		if (opflg == CLASS1)
			class1();
		else if (opflg == CLASS2)
			class2(&i);
		else class3(&i);
	}
}

/****************************************************************************/

/* printline prints the contents of prlnbuf */

println()
{
	if (lflag > 0)
		fprintf(stdout, "%s\n", prlnbuf);
}

/* colsym() collects a symbol from prlnbuf into symbol0[],
 *    leaves prlnbuf pointer at first invalid symbol character,
 *    returns 0 if no symbol collected
 */

colsym(ip)
    int *ip;
{
	int	valid;
	int	i;
	char	ch;

	valid = 1;
	i = 0;
	while (valid == 1) {
		ch = prlnbuf[*ip];
		if (ch == '_' || ch == '.');
		else if (ch >= 'a' && ch <= 'z');
		else if (ch >= 'A' && ch <= 'Z');
		else if (i >= 1 && ch >= '0' && ch <= '9');
		else if (i == 1 && ch == '=');
		else valid = 0;
		if (valid == 1) {
			if (i < SBOLSZ - 1)
				symbol0[++i] = ch;
			(*ip)++;
		}
	}
	if (i == 1) {
		switch (symbol0[1]) {
		case 'A': case 'a':
		case 'X': case 'x':
		case 'Y': case 'y':
			error("Symbol is reserved (A, X or Y)");
			i = 0;
		}
	}
	symbol0[0] = i;
	return(i);
}

/* symbol table lookup
 *	if found, return pointer to symbol
 *	else, install symbol as undefined, and return pointer
 */

stlook()
{
	int	found;
	int	hptr;
	int	j;
	int	nptr;
	int	pptr;
	int	ptr;

	hptr = 0;
	for (j = 0; j < symbol0[0]; j++)
		hptr += symbol0[j];
	hptr %= 128;
	ptr = hash_tbl[hptr];
	if (ptr == -1) {		/* no entry for this link */
		hash_tbl[hptr] = nxt_free;
		return(stinstal());
	}
	while (symtab[ptr] != 0) {	/* 0 count = end of table */
		found = 1;
		for (j = 0; j <= symbol0[0]; j++) {
			if (symbol0[j] != symtab[ptr + j]) {
				found = 0;
				pptr = ptr + symtab[ptr] + 4;
				nptr = (symtab[pptr + 1] << 8) + (symtab[pptr] & 0xff);
				nptr &= 0xffff;
				if (nptr == 0) {
					symtab[ptr + symtab[ptr] + 4] = nxt_free & 0xff;
					symtab[ptr + symtab[ptr] + 5] = (nxt_free >> 8) & 0xff;
					return(stinstal());
				}
				ptr = nptr;
				break;
			}
		}
		if (found == 1)
			return(ptr);
	}
	error("Symbol not found");
	return(-1);
}

/*  instal symbol into symtab
 */
stinstal()
{
register	int	j;
register	int	ptr1;
register	int	ptr2;

	ptr1 = ptr2 = nxt_free;
	if ((ptr1 + symbol0[0] + 6) >= STABSZ) {
		error("Symbol table full");
		return(-1);
	}
	for (j = 0; j <= symbol0[0]; j++)
		symtab[ptr1++] = symbol0[j];
	symtab[ptr1] = udtype;
	nxt_free = ptr1 + 5;
	return(ptr2);
}

/* operation code table lookup
 *	if found, return pointer to symbol,
 *	else, return -1
 */

oplook(ip)
   int	*ip;
{
register	char	ch;
register	int	i;
register	int	j;
	int	k;
	int	temp[2];

	i = j = 0;
	temp[0] = temp[1] = 0;
	while((ch=prlnbuf[*ip])!= ' ' && ch!= 0 && ch!= '\t' && ch!= ';') {
		if (ch >= 'A' && ch <= 'Z')
			ch &= 0x1f;
		else if (ch >= 'a' && ch <= 'z')
			ch &= 0x1f;
		else if (ch == '.')
			ch = 31;
		else if (ch == '*')
			ch = 30;
		else if (ch == '=')
			ch = 29;
		else return(-1);
		temp[j] = (temp[j] * 0x20) + (ch & 0xff);
		if (ch == 29)
			break;
		++(*ip);
		if (++i >= 3) {
			i = 0;
			if (++j >= 2) {
				return(-1);
			}
		}
	}
	if ((j = temp[0]^temp[1]) == 0)
		return(-2);
	k = 0;
	i = step[k] - 3;
	do {
		if (j == optab[i]) {
			opflg = optab[++i];
			opval = optab[++i];
			return(i);
		}
		else if (j < optab[i])
			i -= step[++k];
		else i += step[++k];
	} while (step[k] != 0);
	return(-1);
}

/* error printing routine */

error(stptr)
   char *stptr;
{
	loadlc(loccnt, 0, 1);
	loccnt += 3;
	loadv(0,0,0);
	loadv(0,1,0);
	loadv(0,2,0);
	fprintf(stdout, "%s\n", prlnbuf);
	fprintf(stdout, "%s\n", stptr);
	errcnt++;
}

/* load 16 bit value in printable form into prlnbuf */

loadlc(val, f, outflg)
    int val;
    int f;
    int outflg;
{
	int	i;

	i = 6 + 7*f;
	hexcon(4, val);
	if (nflag == 0) {
		prlnbuf[i++]  = hex[3];
		prlnbuf[i++]  = hex[4];
		prlnbuf[i++]  = ':';
		prlnbuf[i++]  = hex[1];
		prlnbuf[i] = hex[2];
	}
	else {
		prlnbuf[i++] = hex[1];
		prlnbuf[i++] = hex[2];
		prlnbuf[i++] = hex[3];
		prlnbuf[i] = hex[4];
	}
	if ((pass == LAST_PASS)&&(oflag != 0)&&(objcnt <= 0)&&(outflg != 0)) {
		fprintf(optr, "\n;%c%c%c%c", hex[3], hex[4], hex[1], hex[2]);
		objcnt = 16;
	}
}



/* load value in hex into prlnbuf[contents[i]] */
/* and output hex characters to obuf if LAST_PASS & oflag == 1 */

loadv(val,f,outflg)
   int	val;
   int	f;		/* contents field subscript */
   int	outflg;		/* flag to output object bytes */
{

	hexcon(2, val);
	prlnbuf[13 + 3*f] = hex[1];
	prlnbuf[14 + 3*f] = hex[2];
	if ((pass == LAST_PASS) && (oflag != 0) && (outflg != 0)) {
		fputc(hex[1], optr);
		fputc(hex[2], optr);
		--objcnt;
	}
}

/* convert number supplied as argument to hexadecimal in hex[digit] (lsd)
		through hex[1] (msd)		*/

hexcon(digit, num)
    int digit;
   int	num;
{

	for (; digit > 0; digit--) {
		hex[digit] = (num & 0x0f) + '0';
		if (hex[digit] > '9')
			hex[digit] += 'A' -'9' - 1;
		num >>= 4;
	}
}

/* assign <value> to label pointed to by lablptr,
 *	checking for valid definition, etc.
 */

labldef(lval)
    int lval;
{
	int	i;

	if (lablptr != -1) {
		lablptr += symtab[lablptr] + 1;
		if (pass == FIRST_PASS) {
			if (symtab[lablptr] == UNDEF) {
				symtab[lablptr + 1] = lval & 0xff;
				i = symtab[lablptr + 2] = (lval >> 8) & 0xff;
				if (i == 0)
					symtab[lablptr] = DEFZRO;
				else	symtab[lablptr] = DEFABS;
			}
			else if (symtab[lablptr] == UNDEFAB) {
				symtab[lablptr] = DEFABS;
				symtab[lablptr + 1] = lval & 0xff;
				symtab[lablptr + 2] = (lval >> 8) & 0xff;
			}
			else {
				symtab[lablptr] = MDEF;
				symtab[lablptr + 1] = 0;
				symtab[lablptr + 2] = 0;
				error("Label multiply defined");
				return(-1);
			}
		}
		else {
			i = (symtab[lablptr + 2] << 8) +
				(symtab[lablptr+1] & 0xff);
			i &= 0xffff;
			if (i != lval && pass == LAST_PASS) {
				error("Sync error");
				return(-1);
			}
		}
	}
	return(0);
}

/* determine the value of the symbol,
 * given pointer to first character of symbol in symtab
 */

symval(ip)
    int *ip;
{
	int	ptr;
	int	svalue;

	svalue = 0;
	colsym(ip);
	if ((ptr = stlook()) == -1)
		undef = 1;		/* no room error */
	else if (symtab[ptr + symtab[ptr] + 1] == UNDEF)
		undef = 1;
	else if (symtab[ptr + symtab[ptr] + 1] == UNDEFAB)
		undef = 1;
	else svalue = ((symtab[ptr + symtab[ptr] + 3] << 8) +
		(symtab[ptr + symtab[ptr] + 2] & 0xff)) & 0xffff;
	if (symtab[ptr + symtab[ptr] + 1] == DEFABS)
		zpref = 1;
	if (undef != 0)
		zpref = 1;
	return(svalue);
}


/* class 1 machine operations processor - 1 byte, no operand field */

class1()
{
	if (pass == LAST_PASS) {
		loadlc(loccnt, 0, 1);
		loadv(opval, 0, 1);
		println();
	}
	loccnt++;
}


/* class 2 machine operations processor - 2 byte, relative addressing */

class2(ip)
    int *ip;
{

	if (pass == LAST_PASS) {
		loadlc(loccnt, 0, 1);
		loadv(opval, 0, 1);
		while (prlnbuf[++(*ip)] == ' ');
		if (evaluate(ip) != 0) {
			loccnt += 2;
			return;
		}
		loccnt += 2;
		if ((value -= loccnt) >= -128 && value < 128) {
			loadv(value, 1, 1);
			println();
		}
		else error("Invalid branch address");
	}
	else loccnt += 2;
}


/* class 3 machine operations processor - various addressing modes */

class3(ip)
    int *ip;
{
	char	ch;
	int	code;
	int	flag;
	int	i;
	int	ztmask;

	while ((ch = prlnbuf[++(*ip)]) == ' ');
	switch(ch) {
	case 0:
	case ';':
		error("Operand field missing");
		return;
	case 'A':
	case 'a':
		if ((ch = prlnbuf[*ip + 1]) == ' ' || ch == 0) {
			flag = ACC;
			break;
		}
	default:
		switch(ch = prlnbuf[*ip]) {
		case '#': case '=':
			flag = IMM1 | IMM2;
			++(*ip);
			break;
		case '(':
			flag = IND | INDX | INDY;
			++(*ip);
			break;
		default:
			flag = ABS | ZER | ZERX | ABSX | ABSY | ABSY2 | ZERY;
		}
		if ((flag & (INDX | INDY | ZER | ZERX | ZERY) & opflg) != 0)
			udtype = UNDEFAB;
		if (evaluate(ip) != 0)
			return;
		if (zpref != 0) {
			flag &= (ABS | ABSX | ABSY | ABSY2 | IND | IMM1 | IMM2);
			ztmask = 0;
		}
		else ztmask = ZER | ZERX | ZERY;
		code = 0;
		i = 0;
		while (( ch = prlnbuf[(*ip)++]) != ' ' && ch != 0 && i++ < 4) {
			code *= 8;
			switch(ch) {
			case ')':		/* ) = 4 */
				++code;
			case ',':		/* , = 3 */
				++code;
			case 'X':		/* X = 2 */
			case 'x':
				++code;
			case 'Y':		/* Y = 1 */
			case 'y':
				++code;
				break;
			default:
				flag = 0;
			}
		}
		switch(code) {
		case 0:		/* no termination characters */
			flag &= (ABS | ZER | IMM1 | IMM2);
			break;
		case 4:		/* termination = ) */
			flag &= IND;
			break;
		case 25:		/* termination = ,Y */
			flag &= (ABSY | ABSY2 | ZERY);
			break;
		case 26:		/* termination = ,X */
			flag &= (ABSX | ZERX);
			break;
		case 212:		/* termination = ,X) */
			flag &= INDX;
			break;
		case 281:		/* termination = ),Y */
			flag &= INDY;
			break;
		default:
			flag = 0;
		}
	}
	if ((opflg &= flag) == 0) {
		error("Invalid addressing mode");
		return;
	}
	if ((opflg & ztmask) != 0)
		opflg &= ztmask;
	switch(opflg) {
	case ACC:		/* single byte - class 3 */
		if (pass == LAST_PASS) {
			loadlc(loccnt, 0, 1);
			loadv(opval + 8, 0, 1);
			println();
		}
		loccnt++;
		return;
	case ZERX: case ZERY:	/* double byte - class 3 */
		opval += 4;
	case INDY:
		opval += 8;
	case IMM2:
		opval += 4;
	case ZER:
		opval += 4;
	case INDX: case IMM1:
		if (pass == LAST_PASS) {
			loadlc(loccnt, 0, 1);
			loadv(opval, 0, 1);
			loadv(value, 1, 1);
			println();
		}
		loccnt += 2;
		return;
	case IND:		/* triple byte - class 3 */
		opval += 16;
	case ABSX:
	case ABSY2:
		opval += 4;
	case ABSY:
		opval += 12;
	case ABS:
		if (pass == LAST_PASS) {
			opval += 12;
			loadlc(loccnt, 0, 1);
			loadv(opval, 0, 1);
			loadv(value, 1, 1);
			loadv(value >> 8, 2, 1);
			println();
		}
		loccnt += 3;
		return;
	default:
		error("Invalid addressing mode");
		return;
	}
}

/* pseudo operations processor */

pseudo(ip)
    int *ip;
{
	int	count;
	int	i;
	int	tvalue;

	switch(opval) {
	case 0:				/* .byte pseudo */
		labldef(loccnt);
		loadlc(loccnt, 0, 1);
		while (prlnbuf[++(*ip)] == ' ');	/*    field	*/
		count = 0;
		do {
			if (prlnbuf[*ip] == '"') {
				while ((tvalue = prlnbuf[++(*ip)]) != '"') {
					if (tvalue == 0) {
						error("Unterminated ASCII string");
						return;
					}
					if (tvalue == '\\')
						switch(tvalue = prlnbuf[++(*ip)]) {
						case 'n':
							tvalue = '\n';
							break;
						case 't':
							tvalue = '\t';
							break;
						}
					loccnt++;
					if (pass == LAST_PASS) {
						loadv(tvalue, count, 1);
						if (++count >= 3) {
							println();
							for (i = 0; i < SFIELD; i++)
								prlnbuf[i] = ' ';
							prlnbuf[i] = 0;
							count = 0;
							loadlc(loccnt, 0, 1);
						}
					}
				}
				++(*ip);
			}
			else {
				if (evaluate(ip) != 0) {
					loccnt++;
					return;
				}
				loccnt++;
				if (value > 0xff) {
					error("Operand field size error");
					return;
				}
				else if (pass == LAST_PASS) {
					loadv(value, count, 1);
					if (++count >= 3) {
						println();
						for (i = 0; i < SFIELD; i++)
							prlnbuf[i] = ' ';
						prlnbuf[i] = 0;
						count = 0;
						loadlc(loccnt, 0, 1);
					}
				}
			}
		} while (prlnbuf[(*ip)++] == ',');
		if ((pass == LAST_PASS) && (count != 0))
			println();
		return;
	case 1:				/* = pseudo */
		while (prlnbuf[++(*ip)] == ' ');
		if (evaluate(ip) != 0)
			return;
		labldef(value);
		if (pass == LAST_PASS) {
			loadlc(value, 1, 0);
			println();
		}
		return;
	case 2:				/* .word pseudo */
		labldef(loccnt);
		loadlc(loccnt, 0, 1);
		while (prlnbuf[++(*ip)] == ' ');
		do {
			if (evaluate(ip) != 0) {
				loccnt += 2;
				return;
			}
			loccnt += 2;
			if (pass == LAST_PASS) {
				loadv(value, 0, 1);
				loadv(value>>8, 1, 1);
				println();
				for (i = 0; i < SFIELD; i++)
					prlnbuf[i] = ' ';
				prlnbuf[i] = 0;
				loadlc(loccnt, 0, 1);
			}
		} while (prlnbuf[(*ip)++] == ',');
		return;
	case 3:				/* *= pseudo */
		while (prlnbuf[++(*ip)] == ' ');
		if (prlnbuf[*ip] == '*') {
			if (evaluate(ip) != 0)
				return;
			if (undef != 0) {
				error("Undefined symbol in operand field.");
				return;
			}
			tvalue = loccnt;
		}
		else {
			if (evaluate(ip) != 0)
				return;
			if (undef != 0) {
				error("Undefined symbol in operand field.");
				return;
			}
			tvalue = value;
		}
		loccnt = value;
		labldef(tvalue);
		if (pass == LAST_PASS) {
			objcnt = 0;
			loadlc(tvalue, 1, 0);
			println();
		}
		return;
	case 4:				/* .list pseudo */
		if (lflag >= 0)
			lflag = 1;
		return;
	case 5:				/* .nlst pseudo */
		if (lflag >= 0)
			lflag = iflag;
		return;
	case 6:				/* .dbyt pseudo */
		labldef(loccnt);
		loadlc(loccnt, 0, 1);
		while (prlnbuf[++(*ip)] == ' ');
		do {
			if (evaluate(ip) != 0) {
				loccnt += 2;
				return;
			}
			loccnt += 2;
			if (pass == LAST_PASS) {
				loadv(value>>8, 0, 1);
				loadv(value, 1, 1);
				println();
				for (i = 0; i < SFIELD; i++)
					prlnbuf[i] = ' ';
				prlnbuf[i] = 0;
				loadlc(loccnt, 0, 1);
			}
		} while (prlnbuf[(*ip)++] == ',');
		return;
	}
}

/* evaluate expression */

evaluate(ip)
   int	*ip;
{
	int	tvalue;
	int	invalid;
	int	parflg, value2;
	char	ch;
	char	op;
	char	op2;

	op = '+';
	parflg = zpref = undef = value = invalid = 0;
/* hcj: zpref should reflect the value of the expression, not the value of
   the intermediate symbols
*/
	while ((ch=prlnbuf[*ip]) != ' ' && ch != ')' && ch != ',' && ch != ';') {
		tvalue = 0;
		if (ch == '$' || ch == '@' || ch == '%')
			tvalue = colnum(ip);
		else if (ch >= '0' && ch <= '9')
			tvalue = colnum(ip);
		else if (ch >= 'a' && ch <= 'z')
			tvalue = symval(ip);
		else if (ch >= 'A' && ch <= 'Z')
			tvalue = symval(ip);
		else if ((ch == '_') || (ch == '.'))
			tvalue = symval(ip);
		else if (ch == '*') {
			tvalue = loccnt;
			++(*ip);
		}
		else if (ch == '\'') {
			++(*ip);
			tvalue = prlnbuf[*ip] & 0xff;
			++(*ip);
		}
		else if (ch == '[') {
			if (parflg == 1) {
				error("Too many ['s in expression");
				invalid++;
			}
			else {
				value2 = value;
				op2 = op;
				value = tvalue = 0;
				op = '+';
				parflg = 1;
			}
			goto next;
		}
		else if (ch == ']') {
			if (parflg == 0) {
				error("No matching [ for ] in expression");
				invalid++;
			}
			else {
				parflg = 0;
				tvalue = value;
				value = value2;
				op = op2;
			}
			++(*ip);
		}
		switch(op) {
		case '+':
			value += tvalue;
			break;
		case '-':
			value -= tvalue;
			break;
		case '/':
			value = (unsigned) value/tvalue;
			break;
		case '*':
			value *= tvalue;
			break;
		case '%':
			value = (unsigned) value%tvalue;
			break;
		case '^':
			value ^= tvalue;
			break;
		case '~':
			value = ~tvalue;
			break;
		case '&':
			value &= tvalue;
			break;
		case '|':
			value |= tvalue;
			break;
		case '>':
			tvalue >>= 8;		/* fall through to '<' */
		case '<':
			if (value != 0) {
				error("High or low byte operator not first in operand field");
			}
			value = tvalue & 0xff;
			zpref = 0;
			break;
		default:
			invalid++;
		}
		if ((op=prlnbuf[*ip]) == ' '
				|| op == ')'
				|| op == ','
				|| op == ';')
			break;
		else if (op != ']')
next:			++(*ip);
	}
	if (parflg == 1) {
		error("Missing ] in expression");
		return(1);
	}
	if (value < 0 || value >= 256) {
		zpref = 1;
	}
	if (undef != 0) {
		if (pass != FIRST_PASS) {
			error("Undefined symbol in operand field");
			invalid++;
		}
		value = 0;
	}
	else if (invalid != 0)
	{
		error("Invalid operand field");
	}
	else {
/*
 This is the only way out that may not signal error
*/
		if (value < 0 || value >= 256) {
			zpref = 1;
		}
		else {
			zpref = 0;
		}
	}
	return(invalid);
}

/* collect number operand		*/

colnum(ip)
	int	*ip;
{
	int	mul;
	int	nval;
	char	ch;

	nval = 0;
	if ((ch = prlnbuf[*ip]) == '$')
		mul = 16;
	else if (ch >= '1' && ch <= '9') {
		mul = 10;
		nval = ch - '0';
	}
	else if (ch == '@' || ch == '0')
		mul = 8;
	else if (ch == '%')
		mul = 2;
	while ((ch = prlnbuf[++(*ip)] - '0') >= 0) {
		if (ch > 9) {
			ch -= ('A' - '9' - 1);
			if (ch > 15)
				ch -= ('a' - 'A');
			if (ch > 15)
				break;
			if (ch < 10)
				break;
		}
		if (ch >= mul)
			break;
		nval = (nval * mul) + ch;
	}
	return(nval);
}
