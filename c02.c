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

#define VARLEN 7  /*Maximum Variable Length*/
#define MAXVAR 255 /*Maximum Number of Variables*/

#define DEBUG(s, val) if (debug) printf(s, val)

enum data_type {BYTE, CHAR};
enum term_type {NUMBER, VARIABLE};

FILE *src_file;  /*Source File (Input)*/
FILE *out_file;  /*Assembler File (Output)*/
FILE *log_file;  /*Log File (Output) */

char src_name[255]; /*Source File Name*/
char out_name[255]; /*Assembler File Name*/
char log_name[255]; /*Log File Name */

int debug; /*Print Debug Info*/

int nextChar; /*Next Character of Source File to Process*/
int curCol, curLine; /*Position in Source Code*/

char word[255]; /*Word parsed from source file*/
char term[255]; /*Term parsed from equation*/
char operator;  /*Arithmetic or Bitwise Operator*/

char var_name[MAXVAR+1][VARLEN+1]; /*Variable Name Table*/
char var_type[MAXVAR+1];           /*Variable Type Table*/
int  var_count;                    /*Number of Variables Assigned*/



         /* Error - print Source File position and exit */
          void exit_error(int err_no)
          {
            printf("Line %d Column %d\n", curLine, curCol);
            exit(err_no);
          }

       /* Error - Print perror() and exit */
        void exit_perror()
        {
          perror("C02");
          exit_error(errno);
        }

       /* Error - print "Expected" message and exit  
          Args: s - Expected string */ 
        void expected(char* s)
        {
          printf("Expected %s\n", s);
          exit_error(EXIT_FAILURE);
        }

         /* Get Next Character from Source File            *
          * Uses: src_file - Source File Handle            *
          * Sets: nextChar - Next Character in Source File */
          void getChar() 
          {
            nextChar = fgetc(src_file);	
            if (nextChar == '\n') curCol=1; else curCol++;
            if (curCol == 1) curLine++;
          }

         /* Various tests against nextChar */
          int isAlpha()    {return isalpha(nextChar);}
          int isAlphaNum() {return isalnum(nextChar);}
          int isDigit()    {return isdigit(nextChar);}
          int isNewLine()  {return (nextChar == '\n' || nextChar == '\r');}
          int isSpace()    {return isspace(nextChar);}
          int match(int c) {return (nextChar == c);}
        
       /* Advance Source File to next printable character */
        void skip_spaces() {while (isSpace()) getChar();}

       /* Advance Source File to end of line */
        void skip_to_eol() {while (!isNewLine()) getChar();}

   /* Initilize Compiler Variables */
    void init()
    {
      DEBUG(">Initializing Compiler Variables\n",0);
      var_count = 0;
      curCol = 0;
      curLine = 0;
    }

      /* Reads next Word in Source File, where            *
       *  a Word is a sequence of AlphaNumeric characters *
       * Sets: word - the Word read from the source file  */
      get_word()
      {
        int wordLen = 0;
        skip_spaces();
        if (!isAlpha()) expected("Alphabetic Character");
	      while (isAlphaNum())
	      {
	        word[wordLen++] = nextChar;
		      getChar();
	      }
	      word[wordLen] = 0;
      }

      /* Reads Decimal number from source file    *
       *  a Decimal is a series of digits (0-9)   *
       * Returns: integer value of number         */ 
      int get_decimal()
      {
        int digit;
        int number = 0;
        skip_spaces();
        if (!isDigit()) expected("Digit");
        while (!isDigit())
        {
          digit = nextChar - '0';
          number = number * 10 + digit;
        }
        return(number);
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
          if (nextChar == c) { getChar(); return; }
          printf("Expected Character '%c' ", c);
          exit_error(EXIT_FAILURE);
        }

     /* add identifier(s) of type t*/
      void add_idents(int t)
      {
        DEBUG(">Processing Identifier(s) of type %d\n", t);
        skip_spaces();
        if (isAlpha()) 
        {
          get_word();
          DEBUG(">adding Identifier %s\n", word);
          strncpy(var_name[var_count], word, VARLEN);
          var_type[var_count] = t;
          var_count++;
          expect_char(';');
        }
      }

             /* output a single line of assembly code */
              void asm_line(char *label, char *opcode, char *operand)
              {
                fprintf(out_file, "%-7s %-3s %s\n", label, opcode, operand);
              }

              void parse_term()
              {
                //todo: differentiate between numbers and variables
                //check variable name against variable table
                if (!isAlphaNum())
                  expected("term");
                get_word();
                strcpy(term, word);
                DEBUG(">parsing term '%s'\n", term);
                skip_spaces();
              }

            void first_term()
            {
              parse_term();
              DEBUG(">processing term '%s'\n", term);
              asm_line("", "LDA", term);
            }

            void parse_operator()
            {
              if (strchr("+-&|^", nextChar) == NULL)
                expected("operator");
              operator = nextChar;
              DEBUG(">parsing operator '%c'\n", operator);
              getChar();
              skip_spaces();
            }

            void process_operator()
            {
              DEBUG(">processing operator '%c'\n", operator);
              switch(operator)
              {
                case '+': 
                  asm_line("","CLC", "");
                  asm_line("","ADC", term);
                  break;
                case '-':
                  asm_line("","SEC", "");
                  asm_line("","SBC", term);
                  break;
                case '&':
                  asm_line("","AND", term);
                  break;
                case '|':
                  asm_line("","ORA", term);
                  break;
                case '^':
                  asm_line("","EOR", term);
                  break;
                default:
                  printf("Unrecognized operator '%c'\n", operator);
                  exit_error(EXIT_FAILURE);
              }
            }

          void parse_expr()
          {
            DEBUG(">parsing expression\n", 0);
            skip_spaces();
            if (match('-')) 
              asm_line("", "LDA", "#0"); //handle unary minus            
            else 
              first_term();
            while (!match(';'))
            {
              parse_operator();
              parse_term();
              process_operator();
            } 
          }

       /* parse and compile assignment */
        void parse_assignment()
        {
          DEBUG(">parsing assignment", 0);
          char var_assign[VARLEN+1];          
          strcpy(var_assign, word);  //save variable to assign to
          getChar(); //skip equals sign
          parse_expr();
          asm_line("", "STA", var_assign);
        }

     /* parse and compile program statement */
      void parse_statement()
      {
        DEBUG(">parsing statement\n", 0);
        skip_spaces();
        if (match('='))
          parse_assignment();
        else
          expected("=");
      }

   /* Reads and parses the next Word in Source File */
    parse_word()
    {
      get_word();
      DEBUG(">Parsing Word '%s'\n", word);
      if (word_is("byte")) add_idents(BYTE);
      else if (word_is("char")) add_idents(CHAR);
      else parse_statement();
    }

   /* Advance Source File to end of comment    *
    * Recognizes both C and C++ style comments */
    void skip_comment()
    {
      DEBUG(">Skipping Comment\n", 0);     
      getChar();
	    if (match('/'))         //if C style comment
	      skip_to_eol();        //  skip rest of line
      else if (match('*'))    //if C++ style comment
        while (TRUE)          //  skip to "*/"
        {
          getChar(); if (!match('*')) continue;
          getChar(); if (!match('/')) continue;
			    getChar();
          break;	
          //todo: add code to catch unterminated comment
      	}
	    else                    //if neither  
	      expected("/ or *");   // error out
    }

    void asm_prolog()
    {
      fprintf(out_file, ";Program %s\n", src_name);
      fprintf(out_file, "        CPU 6502\n");
    }

     /* Write Variable Table */
      void asm_vartbl()
      {
        int i;
        for (i=0; i<var_count; i++)
        {
          asm_line(var_name[i], "DB", "0");
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
    getChar();
    while ( TRUE )  
    {
      if (match(EOF)) break;
      if (match('/')) 
        skip_comment();
      else if (isAlpha()) 
        parse_word();
      else
		  {
        fprintf(log_file, "%c", nextChar);	 
			  getChar();
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
    if (src_file == NULL) exit_perror();
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
    if (out_file == NULL) exit_perror();
  }

 /* Open Log File                    *
  * Uses: src_name - Source File Name  *
  * Sets: log_file - Log File Handle    */
  void open_log_file()
  {
    strcpy(log_name, src_name); //set Log File Name to Source File Name
    char *dot = strrchr(log_name, '.'); //find file extension
    if (dot != NULL) *dot = 0;          //and remove it
    strcat(log_name, ".log");           //add extension ".asm"
    DEBUG(">Opening Log File '%s'\n", log_name);
    log_file = fopen(log_name, "w");
    if (log_file == NULL)  exit_perror();
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
}


