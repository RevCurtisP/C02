/**************************************************************
 * C02 Compiler - (C) 2013 Curtis F Kaylor                    *
 *                                                            *
 * C02 is a modified C-like language designed for the 6502    *
 *                                                            * 
 * This Compiler generates crasm compatible assembly language *
 *                                                            *
 **************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>

#define TRUE -1
#define FALSE 0

#define VARLEN 7          //Maximum Variable Length
#define MAXVAR 255        //Maximum Number of Variables
#define LABLEN 6          //Maximum Label Length
#define LABFMT "L_%04d"   //Label Format
#define LABSFX ":"        //Label Suffix
#define MAXLAB 15         //Maximum Number of Labels (Nesting Depth)
#define COMPOPS " =< >"   //Comparison Operators

#define CPU_OP  "PROCESSOR" //Target CPU Pseudo-Operator 
#define CPU_ARG "6502"      //Target CPU Operand
#define BYTE_OP "DC"        //Define Byte Pseudo-Op
#define SPC_OP  "DS"        //Define String Pseudo-Op

#define DEBUG(s, val) if (debug) {print_pos(); printf(s, val);}

enum data_types {BYTE, CHAR};
enum term_types {CONSTANT, VARIABLE, ARRAY, FUNCTION};

FILE *src_file;  /*Source File (Input)*/
FILE *out_file;  /*Assembler File (Output)*/
FILE *log_file;  /*Log File (Output) */
FILE *inc_file;  /*Include File Handle*/

char src_name[255]; /*Source File Name*/
char out_name[255]; /*Assembler File Name*/
char log_name[255]; /*Log File Name */
char inc_name[255]; /*Include File Name */

int debug; /*Print Debug Info*/

int next_char; /*Next Character of Source File to Process*/
int curCol, curLine; /*Position in Source Code*/

char comment[255]; /*Text to be printed on comment lines*/

char line[255];   /*Entire line parsed from include file*/
char word[255];   /*Word parsed from source file*/
char value[255];  /*Term parsed from equation*/
char term[255];   /*Term parsed from equation*/
char operator;    /*Arithmetic or Bitwise Operator*/
int  val_type;    /*Value Type*/
int  term_type;   /*Term Type*/

char var_name[MAXVAR+1][VARLEN+1]; /*Variable Name Table*/
char var_type[MAXVAR+1];           /*Variable Type Table*/
char var_size[MAXVAR+1][4];        /*Variable Array Size*/
int  var_count;                    /*Number of Variables Assigned*/

char var_assign[VARLEN+1];         /*Assigned Variable Name*/
char var_index[VARLEN+1] ;         /*Assigned Variable Index*/
int  var_indexed;                  /*Assigned Variable is Indexed*/

char label[LABLEN+1];              /*Most recently generated label*/
char lab_name[MAXLAB+1][LABLEN+1]; /*Label Name Table*/
int  lab_count;                    /*Number of Labels in stack*/
int  lab_next;                     /*Sequence of next label to be generated*/
char lab_asm[LABLEN+2];            /*Label to emit on next asm line*/

/* Print current position in file */
void print_pos() 
{
  printf("(%d,%d) ", curLine, curCol);
}

/* Error - print Source File position and exit */
void exit_error(int err_no)
{
  printf("Line %d Column %d\n", curLine, curCol);
  exit(err_no);
}

/* Error - Print textual description of system error *
 *         and exit with system error code           */
void exit_sys_error(char *s)
{
  perror(s);
  exit_error(errno);
}

/* Error - print "Expected" error message            *
           and exit with general failure code        *
   Args: expected - Description of what was expected */ 
void exit_expected(char *expected)
{
  printf("Expected %s, but found '%c'\n", expected, next_char);
  exit_error(EXIT_FAILURE);
}

/* Get Next Character from Source File            *
 * Uses: src_file - Source File Handle            *
 * Sets: next_char - Next Character in Source File */
void get_char() 
{
  next_char = fgetc(src_file);	
  if (next_char == '\n') curCol=1; else curCol++;
  if (curCol == 1) curLine++;
}

/* Various tests against next_char */
int isAlpha()    {return isalpha(next_char);}
int isAlphaNum() {return isalnum(next_char);}
int isDigit()    {return isdigit(next_char);}
int isNewLine()  {return (next_char == '\n' || next_char == '\r');}
int isSpace()    {return isspace(next_char);}
int match(int c) {return (next_char == c);}
        
/* Advance Source File to next printable character */
void skip_spaces() {while (isSpace()) get_char();}

/* Advance Source File to end of line */
void skip_to_eol() {while (!isNewLine()) get_char();}

/* Initilize Compiler Variables */
void init()
{
  DEBUG(">Initializing Compiler Variables\n",0);
  var_count = 0;
  lab_count = 0;
  curCol = 0;
  curLine = 0;
}

/* Reads next Word in Source File, where            *
 *  a Word is a sequence of AlphaNumeric characters *
 * Sets: word - the Word read from the source file  */
void get_word()
{
  int wordLen = 0;
  skip_spaces();
  if (!isAlpha()) exit_expected("Alphabetic Character");
  while (isAlphaNum())
  {
    word[wordLen++] = next_char;
    get_char();
  }
  word[wordLen] = 0;
}

/* Reads Decimal number from source file    *
 * a Decimal is a series of digits (0-9)    *
 * Sets: word - number without leading 0's  *
 * Returns: integer value of number         */ 
int get_decimal()
{
  int digit;
  int number = 0;
  int wordLen = 0;
  skip_spaces();
  if (!isDigit()) exit_expected("Digit");
  while (isDigit())
  {
    if (next_char != '0' || number != 0) //skip leading zeros
      word[wordLen++] = next_char;      
    digit = next_char - '0';
    number = number * 
    10 + digit;
    get_char();
  }
  if (number == 0) word[wordLen++] = '0';
  word[wordLen] = 0;
  return(number);
}

/* Parse decimal constant                   *
 * Sets: value - the constant (as a string) */
void parse_constant() {
  int constant;
  val_type = CONSTANT;
  constant = get_decimal();
  if (constant > 255) {
    printf("Out of bounds constant %d;\n", constant);
    exit_error(EXIT_FAILURE);
  }
  strcpy(value, word);
  DEBUG(">Parsed constant '%d'\n", constant);
}

/* if Word is s then return TRUE else return FALSE*/
int word_is(char *s)
{
  return strcmp(word, s) == 0;
}

/* if next printable character is c then skip, else generate error */ 
void expect_char(char c)
{
  skip_spaces();
  if (next_char == c) { get_char(); return; }
  printf("Expected Character '%c', but found '%c'\n", c, next_char);
  exit_error(EXIT_FAILURE);
}

/* Check if the next printable character is c *
 * and advance past it if it is               *
 * Returns TRUE is character is found,        *
 *         otherwise FALSE                    */
int lookfor_char(char c) {
  int found;
  skip_spaces();
  found = match(c);
  if (found) get_char();
  return found;
} 
 
/* Lookup variable name in variable table   *
 * Returns index into var_name array        *
 *         FALSE if variable was not found  */
int lookup_var(char *name) {
  int i;
  DEBUG(">Looking up variable %s\n", word);
  for (i=0; i<var_count; i++) {
    if (strcmp(var_name[i], name) == 0)
      return i;
  }
  return -1;
}

/* Check if variable has been defined */
int var_defined(char *name) {
  if (lookup_var(name) == -1)
    return FALSE;
  else
    return TRUE;
}  

/* Check for variable */
/* Generates error if variable is undefined */
void check_var(char *name) {
  if (!var_defined(name)) {
    printf("Undeclared variable '%s' encountered\n", word);
    exit_error(EXIT_FAILURE);
  }
}

void parse_varsize() {
  DEBUG(">Checking for array definition\n", 0);
  if (match('[')) {
    get_char();
    DEBUG(">Parsing array size\n", 0);
    parse_constant();
    expect_char(']');
  }
  else
    value[0] = 0;
}

/* Add variables(s) of type t*/
void add_vars(int t)
{
  DEBUG(">Processing variable declarations(s) of type %d\n", t);
  while(TRUE) {
    skip_spaces();
    if (isAlpha())
    {    
      get_word();
      if (var_defined(word)) {
        printf("Duplicate declaration of variable '%s\n", word);
        exit_error(EXIT_FAILURE);
      }  
      DEBUG(">Adding variable %s\n", word);
      strncpy(var_name[var_count], word, VARLEN);
      var_type[var_count] = t;
      parse_varsize();
      strncpy(var_size[var_count], value, 3);
      var_count++;
    }
    else {
      printf("Unexpected character '%c' in declaration\n", next_char);
      exit_error(EXIT_FAILURE);
    }
    if (!lookfor_char(','))
      break;
  }    
  expect_char(';');
}

/* output a single line of assembly code */
void asm_line(char *opcode, char *operand)
{
  strcat(lab_asm, LABSFX);
  fprintf(out_file, "%-7s %-3s %s\n", lab_asm, opcode, operand);
  if (debug) printf(">%-7s %-3s %s\n", lab_asm, opcode, operand);
  lab_asm[0] = 0;
}

/* output a single comment line */
void comment_line()
{
  fprintf(out_file, "; %s\n", comment);
}

/* Parse variable                      *
 * Sets: term - the term (as a string) */
void parse_variable() {
  val_type = VARIABLE;
  get_word();
  if (match('(')) {
    val_type = FUNCTION;
  }
  else
    check_var(word);
  strcpy(value, word);
  DEBUG(">Parsed variable '%s'\n", term);
}

/* Parse value (constant or simple variable) *
 * Sets: value - the value (as a string)     */
void parse_value()
{
  DEBUG("Parsing value\n", 0);
  if (!isAlphaNum())
    exit_expected("constant or variable");
  if (isDigit())
    parse_constant();
  else
    parse_variable();
  skip_spaces();
}

/* Check for and Parse array index definition   *
 * Returns: TRUE or FALSE                       *   
 * Sets: value       - array index or           *
 *                     "" if no index defined   */
int parse_index() 
{
  if (match('[')) {
    get_char();
    parse_value();
    DEBUG("Parsed array index '%s'\n", term);
    expect_char(']');
    return TRUE;
  }
  else
    return FALSE;
}

/* Parse next term in expression       *
 * Sets: term - the term (as a string) */
void parse_term()
{
  DEBUG(">Parsing term\n", 0);
  parse_value();
  if (val_type == FUNCTION) {
    printf("Function call only allowed in first term of expression\n");
    exit_error(EXIT_FAILURE);
  }
  strcpy(term, value);
  term_type = val_type;
  if (parse_index()) {
    asm_line("LDX", value);
    strcat(term, ",X");
  }
  skip_spaces();
}

/* Parse function call in first expression  */
void parse_fnterm() 
{
  char fnname[255]; /*Function name*/ 
  DEBUG(">Processing expression function call '%s'...\n", term);
  strcpy(fnname, term);
  get_char(); //skip open paren
  skip_spaces();
  if (isAlphaNum()) {
    parse_term();
    asm_line("LDA", term);
  }
  expect_char(')');
  term_type = FUNCTION;
  asm_line("JSR", fnname);
  skip_spaces();
}

/* Parse first term of expession            *
 * First term can include function calls    *
 * Emits code to load term into accumulator */
void first_term()
{
  parse_value();
  DEBUG(">Processing first term '%s'...\n", value);
  strcpy(term, value);
  term_type = val_type;
  if (term_type == FUNCTION) {
     parse_fnterm();
     return;
  }
  if (parse_index()) {
    term_type = ARRAY;
    asm_line("LDX", value);
    strcat(term, ",X");
  }
  asm_line("LDA", term);
}

/* */
void parse_operator()
{
  if (strchr("+-&|^", next_char) == NULL)
    exit_expected("operator");
  operator = next_char;
  DEBUG(">parsing operator '%c'\n", operator);
  get_char();
  skip_spaces();
}

/* */
void process_operator()
{
  DEBUG(">Processing operator '%c'\n", operator);
  switch(operator)
  {
    case '+': 
      asm_line("CLC", "");
      asm_line("ADC", term);
      break;
    case '-':
      asm_line("SEC", "");
      asm_line("SBC", term);
      break;
    case '&':
      asm_line("AND", term);
      break;
    case '|':
      asm_line("ORA", term);
      break;
    case '^':
      asm_line("EOR", term);
      break;
    default:
      printf("Unrecognized operator '%c'\n", operator);
      exit_error(EXIT_FAILURE);
  }
}

/* Parse and compile expression */
void parse_expr(char terminator)
{
  DEBUG(">Parsing expression\n", 0);
  skip_spaces();
  if (match('-')) 
    asm_line("LDA", "#0"); //handle unary minus            
  else 
    first_term();
  while (!match(terminator))
  {
    parse_operator();
    parse_term();
    process_operator();
  } 
  get_char(); //skip terminator
}

/* Parse and compile assignment */
void parse_assignment()
{
  DEBUG(">Parsing assignment of variable '%s''\n", var_assign);
  get_char(); //skip equals sign
  parse_expr(';');
  if (var_indexed) {
    asm_line("LDX", var_index);
    strcat(var_assign,",X");    
  }
  asm_line("STA", var_assign);
  DEBUG(">Completed assignment of variable '%s''\n", var_assign);
}

void process_postop() {
  DEBUG(">Processing post operation '%c'\n", operator);
  switch(operator)
  {
    case '+': 
      asm_line("INC", var_assign);
      break;
    case '-':
      asm_line("DEC", var_assign);
      break;
    case '<':
      asm_line("ASL", var_assign);
      break;
    case '>':
      asm_line("LSR", var_assign);
      break;
    default:
      printf("Unrecognized post operator '%c'\n", operator);
      exit_error(EXIT_FAILURE);
  }
}

void parse_postop() {
  operator = next_char;
  DEBUG(">Checking for post operation '%c'\n", operator);
  get_char();
  if (next_char == operator) {
    get_char();
    process_postop();
    expect_char(';');
  }
  else
    exit_expected("post operator"); 
}

/* Parse function call in first expression  */
void parse_fncall() 
{
  char fnname[255]; /*Function name*/ 
  DEBUG(">Processing function call '%s'...\n", term);
  strcpy(fnname, word);
  get_char(); //skip open paren
  skip_spaces();
  if (match(')'))    //if no arguments 
    get_char();       //  skip close paren
  else               //otherwise
    parse_expr(')');  //parse expression
  asm_line("JSR", fnname);
  expect_char(';');
}

int encode_comparator(char c) 
{
  switch(c)
  {
    case '=': return 1;
    case '<': return 2;
    case '>': return 4;
    default:  return 0; 
  }
}

/* parse comparison operator */
int parse_comparator() 
{
  int comparator, encoded; 
  encoded = encode_comparator(next_char);
  if (encoded == 0) 
    exit_expected("comparison operator");
  comparator = encoded; 
  get_char();
  encoded = encode_comparator(next_char);
  if (encoded != 0) {
    comparator = comparator | encoded; 
    get_char();
  }
  skip_spaces();
  DEBUG("Parsed comparison operator %d\n", comparator);
  return comparator; 
}

void process_comparator(int comparator) 
{
  DEBUG("Processing comparison operator %d\n", comparator);
  switch(comparator) {
    case 1: // = or ==
      asm_line("BNE", label);
      break;
    case 2: // <
      break;
    case 3: // <= or =<
      break;
    case 4: // >
      break;
    case 5: // >= or =>
      break;
    case 6: // <> or ><
      asm_line("BEQ", label);
      break;
    default:
      printf("Unsupported comparison operator index %d\n", comparator);
      exit_error(EXIT_FAILURE);
  }
}

/* parse and compile condition */
void parse_condition(char terminator) 
{
  DEBUG("Parsing condition\n", 0);
  int comparator; /*comparison operator*/
  first_term();
  skip_spaces();
  comparator = parse_comparator();
  parse_term();
  asm_line("CMP", term);
  process_comparator(comparator);
  expect_char(terminator);
}

/* generate new label */
void new_label() 
{
  sprintf(label, LABFMT, lab_next++);
  DEBUG("Generated new label '%s'\n", label);
  strcpy(lab_name[lab_count++], label);
}

/* set label to emit on next line and remove from stack */
void label_line() 
{
  strcpy(lab_asm, lab_name[--lab_count]);
  DEBUG("Set label '%s' to emit on next line\n", lab_asm);
}

/* parse and compile if statement */
void parse_if() {
  DEBUG(">Parsing if statement '%c'\n", next_char);
  expect_char('(');
  new_label();
  parse_condition(')');
}

/* parse and compile if statement */
void parse_goto() {
  DEBUG(">Parsing goto statement\n", 0);
  get_word();  
  asm_line("JMP", word);
  expect_char(';');  
}

/* parse and compile identifier (variable or function call) */
void parse_identifier()
{
  if (match('(')) {
    parse_fncall();
    return;
  }
  check_var(word);
  strcpy(var_assign, word);  //save variable to assign to
  var_indexed = parse_index();
  if (var_indexed) {
    strncpy(var_index, value, VARLEN);
  }
  if (strchr("+-<>", next_char))
    parse_postop();
  else
  {
    skip_spaces();
    if (match('='))
      parse_assignment();
    else
      exit_expected("=");
  }
}

/* parse label in code */
void parse_label()
{
  if (strlen(lab_asm) > 0) {
    printf("Label not allowed at end of block");
    exit_error(EXIT_FAILURE);
  }
  get_char(); //skip ':'
  if (strlen(word) > LABLEN) {
    printf("Label '%s' exceeds maximum size", word);
    exit_error(EXIT_FAILURE);
  }
  strncpy(lab_asm, word, LABLEN);
}

/* parse and compile program statement */
void parse_statement()
{
  DEBUG(">Parsing statement '%s'\n", word);
  if(match(':')) {
    parse_label();
    return;
  }
  if (word_is("if")) {
    parse_if();
    return;
  }
  if (word_is("goto"))
    parse_goto();
  else
    parse_identifier();
  if (lab_count) {
    label_line();
  }
}

/* Reads and parses the next Word in Source File */
parse_word()
{
  get_word();
  DEBUG(">Parsing Word '%s'\n", word);
  if (word_is("byte")) add_vars(BYTE);
  else if (word_is("char")) add_vars(CHAR);
  else parse_statement();
}

/* Reads next include file name from Source File  *
 * Sets: inc_name - the include file name         */
void parse_inc_name()
{
  int inc_len = 0;
  expect_char('<');
  while (!match('>'))
  {
    inc_name[inc_len++] = next_char;
    get_char();
  }
  get_char(); //skip '>'
  inc_name[inc_len] = 0;
}

/* Open include file                      *
 * Uses: inc_name - Source File Name      *
 * Sets: inc_file - Source File Handle    */
void open_inc_file()
{
  DEBUG(">Opening include file '%s'\n", inc_name);
  inc_file = fopen(inc_name, "r");
  if (inc_file == NULL) exit_sys_error(inc_name);
}

/* Close include file */
void close_inc_file() { fclose(inc_file); }

/* Process include file                    */
void include_file() 
{
  parse_inc_name();
  DEBUG(">Processing include file '%s'\n", inc_name);
  open_inc_file();
  strcpy(comment, "======== Include File ");
  strcat(comment, inc_name);
  strcat(comment, " =======");
  comment_line();
  while (fgets(line, sizeof line, inc_file) != NULL) {
    DEBUG(">Writing line: %s", line);
    fputs(line, out_file);
  }
  strcpy(comment, "==========================================");
  comment_line();
  close_inc_file();
}

/* Process a directive                      */
void process_directive()
{
  get_char();            //skip '#'
  get_word();           //read directive into word
  DEBUG("Processing directive '%s'\n", word);
  if (word_is("include")) {
    include_file();
  }
}

/* Advance Source File to end of comment    *
 * Recognizes both C and C++ style comments */
void skip_comment()
{
  DEBUG(">Skipping Comment\n", 0);     
  get_char();               //skip initial /
  if (match('/'))         //if C style comment
    skip_to_eol();        //  skip rest of line
  else if (match('*'))    //if C++ style comment
    while (TRUE)          //  skip to */
    {
      get_char(); 
      if (!match('*')) continue;
      while (match('*')) get_char(); 
      if (!match('/')) continue;
      get_char();
      break;	
      //todo: add code to catch unterminated comment
  	}
  else                    //if neither  
    exit_expected("/ or *");   // error out
}

void asm_prolog()
{
  asm_line(CPU_OP,CPU_ARG);
  strcpy(comment, "Program ");
  strcat(comment, src_name);
  comment_line();
}

/* Write Variable Table */
void asm_vartbl()
{
  int i;
  for (i=0; i<var_count; i++) {
    strncpy(lab_asm, var_name[i], LABLEN);
    if (strlen(var_size[i]) > 0) {
      DEBUG("Allocating array %s\n", var_name[i]);
      asm_line(SPC_OP, var_size[i]);
    }
    else {
      DEBUG("Allocating variable %s\n", var_name[i]);
      asm_line(BYTE_OP, "0");
    }
  }
}

void asm_epilog()
{
  asm_vartbl();
}

/* Compile Source Code*/
void compile()
{
  DEBUG(">Starting Compilation\n",0);
  init();
  asm_prolog();
  get_char();
  while (TRUE)  
  {
    skip_spaces();
    if (match(EOF)) break;
    DEBUG("Checking next character '%c'\n", next_char);
    if (match('#'))
      process_directive();
    else if (match('/')) 
      skip_comment();
    else if (isAlpha()) 
      parse_word();
    else
	  {
      printf("Unexpected character '%c'\n", next_char);	 
		  exit_error(EXIT_FAILURE);
	  }
  }    
  asm_epilog();
}

/* Display "Usage" text and exit*/ 
void usage()
{
  printf("Usage: c02 sourcefile.c02\n");
  exit(EXIT_FAILURE);      
}

/* Parse Command Line Arguments                                 *   
 *  Sets: src_name - Source File Name (from first arg)           *
 *        out_name - Output File Name (from optional second arg) */
void parse_args(int argc, char *argv[])
{ 
 DEBUG(">Parsing %d Arguments\n", argc);
 if (argc < 2) usage(); //at least one argument is required
 strcpy(src_name, argv[1]); //set Source File Name to first arg
 DEBUG(">src_name set to '%s'\n", src_name);
 if (argc > 2) //if second argument exists
   strcpy(out_name, argv[2]); //set Out File Name to second arg
 else strcpy(out_name, ""); //else set to null string
 DEBUG(">out_name set to '%s'\n", out_name);
}

/* Open Source File       s             *
 * Uses: src_name - Source File Name  *
 * Sets: src_file - Source File Handle    */
void open_src_file()
{
  DEBUG(">Processing Source File Name '%s'\n", src_name);
  if (strrchr(src_name, '.') == NULL)   //if no extension
    strcat(src_name, ".c02");           // add ".c02"
  DEBUG(">opening Source File '%s'\n", src_name);
  src_file = fopen(src_name, "r");      //open file
  if (src_file == NULL) exit_sys_error(src_name);
}

/* Open Output File                    *
 * Uses: out_name - Output File Name  *
 * Sets: out_file - Output File Handle    */
void open_out_file()
{
  DEBUG(">Processing Output File Name '%s'\n", out_name);
  if (strlen(out_name) == 0)  //if Output File not specified
  {
    strcpy(out_name, src_name);         //copy Source Name to Ouput Name
    char *dot = strrchr(out_name, '.'); //find extension
    if (dot != NULL) *dot = 0;          //and remove it
    DEBUG(">set Output File Name to '%s'\n", out_name);
  } 
  if (strrchr(out_name, '.') == NULL)   //if no extension
    strcat(out_name, ".asm");           // add ".asm"
  DEBUG(">opening Output File '%s'\n", out_name);
  out_file = fopen(out_name, "w");      //open file
  if (out_file == NULL) exit_sys_error(out_name);
}

/* Open Log File                       *
 * Uses: src_name - Source File Name   *
 * Sets: log_file - Log File Handle    */
void open_log_file()
{
  strcpy(log_name, src_name); //set Log File Name to Source File Name
  char *dot = strrchr(log_name, '.'); //find file extension
  if (dot != NULL) *dot = 0;          //and remove it
  strcat(log_name, ".log");           //add extension ".asm"
  DEBUG(">Opening Log File '%s'\n", log_name);
  log_file = fopen(log_name, "w");
  if (log_file == NULL)  exit_sys_error(log_name);
}

/* Print Variable Table to Log File */
void log_vars()
{
  int i;
  fprintf(log_file, "\n%-31s %s\n", "Variable", "Type");
  for (i=0; i<var_count; i++)
  {
    fprintf(log_file, "%-31s %4d\n", var_name[i], var_type[i]);
  }
}

/* Close Source File */
void close_src_file() { fclose(src_file); }

/* Close Output File */
void close_out_file() { fclose(out_file); }

/* Close Log File */
void close_log_file() { fclose(log_file); }

int main (int argc, char *argv[])
{
  debug = TRUE;
  
  printf("C02 Compiler (C) 2012 Curtis F Kaylor\n" );
  
  parse_args(argc, argv);
  open_src_file();
  open_out_file();
  open_log_file();

  compile();

  log_vars();

  close_src_file();
  close_out_file();
  close_log_file();
}


