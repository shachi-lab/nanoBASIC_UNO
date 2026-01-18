/*
 * nanoBASIC UNO
 * ------------------------------------------
 * A minimal BASIC interpreter for 8-bit AVR.
 *
 * Original concept and implementation:
 *    shachi-lab
 *
 * Based on the first STM8S prototype (2012),
 * reconstructed and refined for Arduino UNO.
 *
 * This interpreter keeps a minimalist design:
 *   - Program lines are stored in input order
 *   - Line numbers act as jump labels
 *   - No automatic renumbering
 *   - Recursive-descent expression evaluator
 *
 * Designed with the philosophy:
 *   "Modern BASIC for small microcontrollers."
 *
 * GitHub: https://github.com/shachi-lab
 * Copyright (c) 2025-2026 shachi-lab
 * License: MIT
 */

#ifndef ARDUINO
  #include <stdio.h>
  #include <stdlib.h>
  #include <stdint.h>
  #include <string.h>
  #include <ctype.h>

  #define PROGMEM
  typedef char __FlashStringHelper;
  typedef const char* PGM_P;
  #define pgm_read_byte(addr) (*(const unsigned char *)(addr))
  #define pgm_read_ptr(addr)  (*(const void **)(addr))
  #define FPSTR(x) (x)
  #define F(x)     (x)
#else
  #include <Arduino.h>

  #ifndef FPSTR
    #define FPSTR(pstr_pointer) (reinterpret_cast<const __FlashStringHelper *>(pstr_pointer))
  #endif
#endif

#include "nano_basic_uno.h"
#include "nano_basic_uno_conf.h"
#include "nano_basic_defs.h"
#include "bios_uno.h"

#define inputChar()       bios_consoleGetChar()
#define printChar(c)      bios_consolePutChar((char)c)

static char inputBuff[INPUT_BUFF_SIZE];
static uint8_t internalcodeBuff[CODE_BUFF_SIZE];
static nb_int_t globalVariables[VARIABLE_NUM];
static nb_int_t arrayValiables[ARRAY_INDEX_NUM];
static stack_t stacks[STACK_NUM];
static int16_t lineNumber;
static uint8_t *executionPointer;
static error_code_t errorCode;
static uint8_t returnRequest;
static uint8_t stackPointer;
static uint8_t exprDepth;
static uint8_t *dataReadPointer;
static uint8_t *resumePointer;
static int16_t resumeLineNumber;
static int16_t progLength;
static uint8_t programArea[PROGRAM_AREA_SIZE];

#define PROGRAM_AREA_TOP  programArea

static void proc_print(void);
static void proc_input(void);
static void proc_goto(void);
static void proc_gosub(void);
static void proc_return(void);
static void proc_for(void);
static void proc_next(void);
static void proc_do(void);
static void proc_loop(void);
static void proc_while(void);
static void proc_if (void);
static void proc_else(void);
static void proc_elseif (void);
static void proc_endif (void);
static void proc_run(void);
static void proc_resume(void);
static void proc_stop(void);
static void proc_end(void);
static void proc_new(void);
static void proc_list(void);
static void proc_prog(void);
static void proc_load(void);
static void proc_save(void);
static void proc_comment(void);
static void proc_outp(void);
static void proc_delay(void);
static void proc_pause(void);
static void proc_reset(void);
static void proc_exit(void);
static void proc_continue(void);
static void proc_randomize(void);
static void proc_data(void);
static void proc_read(void);
static void proc_restore(void);
static void proc_pwm(void);

static void interpreterMain(void);
static uint8_t inputString(uint8_t history_flag);
static uint8_t convertInternalCode(uint8_t *dst, char *src);
static void proc_let(nb_int_t*pvar);
static void initializeValiables(void);
static nb_int_t *getArrayReference(void);
static nb_int_t calcValue(void);
static nb_int_t expr4th(void);
static nb_int_t expr3nd(void);
static nb_int_t expr2nd(void);
static nb_int_t expr(void);
static nb_int_t calcValueFunc(void);
static void printInternalcode(void);
static void printError(void);
static void executeBreak(void);
static error_code_t checkST(uint8_t ch);
static error_code_t checkDelimiter(void);
static error_code_t delayMs(nb_int_t val);
static int16_t checkBreakKey(void);
static uint8_t* findST(const uint8_t* st_list, int16_t* lnum);
static int8_t progLoad(void);
static void programNew(void);
static void programInit(void);
static void programRun(void);
static void printVal(nb_int_t val);
static void printString(const char *str);
static void printStringFlash(const __FlashStringHelper* ifsh);
static void printNewline(void);
static char *int2str(nb_int_t para, uint8_t ff, int16_t len);
static uint8_t* get_dec_val(uint8_t* ptr, nb_int_t* val);
static uint8_t* set_dec_val(uint8_t* ptr, nb_int_t val);
static uint8_t* get_next_ptr(uint8_t* ptr);
static uint8_t* findNextLoop(uint8_t* ptr, uint8_t ch);

typedef void (*PROC)(void);

static const PROC procCodeList[] = {
  proc_print    , // 0x80 : ST_PRINT
  proc_input    , // 0x81 : ST_INPUT
  proc_goto     , // 0x82 : ST_GOTO
  proc_gosub    , // 0x83 : ST_GOSUB
  proc_return   , // 0x84 : ST_RETURN
  proc_for      , // 0x85 : ST_FOR
  proc_next     , // 0x86 : ST_NEXT
  proc_do       , // 0x87 : ST_DO
  proc_loop     , // 0x88 : ST_LOOP
  proc_while    , // 0x89 : ST_WHILE
  proc_if       , // 0x8a : ST_IF
  proc_run      , // 0x8b : ST_RUN
  proc_resume   , // 0x8c : ST_RESUME
  proc_stop     , // 0x8d : ST_STOP
  proc_end      , // 0x8e : ST_END
  proc_new      , // 0x8f : ST_NEW
  proc_list     , // 0x90 : ST_LIST
  proc_prog     , // 0x91 : ST_PROG
  proc_save     , // 0x92 : ST_SAVE
  proc_load     , // 0x93 : ST_LOAD
  proc_delay    , // 0x94 : ST_DELAY
  proc_pause    , // 0x95 : ST_PAUSE
  proc_reset    , // 0x96 : ST_RESET
  proc_exit     , // 0x97 : ST_EXIT
  proc_continue , // 0x98 : ST_CONTINUE
  proc_randomize, // 0x99 : ST_RONDOMIZE
  proc_data     , // 0x9a : ST_DATA
  proc_read     , // 0x9b : ST_READ
  proc_restore  , // 0x9c : ST_RESTORE
  proc_outp     , // 0x9d : ST_OUTP
  proc_pwm      , // 0x9e : ST_PWM
  proc_else     , // 0x9f : ST_ELSE
  proc_elseif   , // 0xa0 : ST_ELSEIF
  proc_endif    , // 0xa1 : ST_ENDIF
};

const char token_st_80[] PROGMEM = "Print"    ; // 0x80 : ST_PRINT
const char token_st_81[] PROGMEM = "Input"    ; // 0x81 : ST_INPUT
const char token_st_82[] PROGMEM = "Goto"     ; // 0x82 : ST_GOTO
const char token_st_83[] PROGMEM = "Gosub"    ; // 0x83 : ST_GOSUB
const char token_st_84[] PROGMEM = "Return"   ; // 0x84 : ST_RETURN
const char token_st_85[] PROGMEM = "For"      ; // 0x85 : ST_FOR
const char token_st_86[] PROGMEM = "Next"     ; // 0x86 : ST_NEXT
const char token_st_87[] PROGMEM = "Do"       ; // 0x87 : ST_DO
const char token_st_88[] PROGMEM = "Loop"     ; // 0x88 : ST_LOOP
const char token_st_89[] PROGMEM = "While"    ; // 0x89 : ST_WHILE
const char token_st_8a[] PROGMEM = "If"       ; // 0x8a : ST_IF
const char token_st_8b[] PROGMEM = "Run"      ; // 0x8b : ST_RUN
const char token_st_8c[] PROGMEM = "Resume"   ; // 0x8c : ST_RESUME
const char token_st_8d[] PROGMEM = "Stop"     ; // 0x8d : ST_STOP
const char token_st_8e[] PROGMEM = "End"      ; // 0x8e : ST_END
const char token_st_8f[] PROGMEM = "New"      ; // 0x8f : ST_NEW
const char token_st_90[] PROGMEM = "List"     ; // 0x90 : ST_LIST
const char token_st_91[] PROGMEM = "Prog"     ; // 0x91 : ST_PROG
const char token_st_92[] PROGMEM = "Save"     ; // 0x92 : ST_SAVE
const char token_st_93[] PROGMEM = "Load"     ; // 0x93 : ST_LOAD
const char token_st_94[] PROGMEM = "Delay"    ; // 0x94 : ST_DELAY
const char token_st_95[] PROGMEM = "Pause"    ; // 0x95 : ST_PAUSE
const char token_st_96[] PROGMEM = "Reset"    ; // 0x96 : ST_RESET
const char token_st_97[] PROGMEM = "Exit"     ; // 0x97 : ST_EXIT
const char token_st_98[] PROGMEM = "Continue" ; // 0x98 : ST_CONTINUE
const char token_st_99[] PROGMEM = "Randomize"; // 0x99 : ST_RONDOMIZE
const char token_st_9a[] PROGMEM = "Data"     ; // 0x9a : ST_DATA
const char token_st_9b[] PROGMEM = "Read"     ; // 0x9b : ST_READ
const char token_st_9c[] PROGMEM = "Restore"  ; // 0x9c : ST_RESTORE
const char token_st_9d[] PROGMEM = "Outp"     ; // 0x9d : ST_OUTP
const char token_st_9e[] PROGMEM = "Pwm"      ; // 0x9e : ST_PWM
const char token_st_9f[] PROGMEM = "Else"     ; // 0x9f : ST_ELSE
const char token_st_a0[] PROGMEM = "ElseIf"   ; // 0xa0 : ST_ELSEIF
const char token_st_a1[] PROGMEM = "EndIf"    ; // 0xa1 : ST_ENDIF
const char token_st_a2[] PROGMEM = "Then"     ; // 0xa2 : ST_THEN
const char token_st_a3[] PROGMEM = "To"       ; // 0xa3 : ST_TO
const char token_st_a4[] PROGMEM = "Step"     ; // 0xa4 : ST_STEP
const char token_fn_a5[] PROGMEM = "Rnd"      ; // 0xa5 : FUNC_RND
const char token_fn_a6[] PROGMEM = "Abs"      ; // 0xa6 : FUNC_ABS
const char token_fn_a7[] PROGMEM = "Inp"      ; // 0xa7 : FUNC_INP
const char token_fn_a8[] PROGMEM = "Adc"      ; // 0xa8 : FUNC_ADC
const char token_fn_a9[] PROGMEM = "Inkey"    ; // 0xa9 : VAL_INKEY
const char token_fn_aa[] PROGMEM = "Chr"      ; // 0xaa : FUNC_CHR
const char token_fn_ab[] PROGMEM = "Dec"      ; // 0xaa : FUNC_DEC
const char token_fn_ac[] PROGMEM = "Hex"      ; // 0xab : FUNC_HEX
const char token_va_ad[] PROGMEM = "Tick"     ; // 0xac : VAL_TICK

static const char * const keyWordList[] PROGMEM = {
  token_st_80, token_st_81, token_st_82, token_st_83, token_st_84, token_st_85, token_st_86, token_st_87,
  token_st_88, token_st_89, token_st_8a, token_st_8b, token_st_8c, token_st_8d, token_st_8e, token_st_8f,
  token_st_90, token_st_91, token_st_92, token_st_93, token_st_94, token_st_95, token_st_96, token_st_97,
  token_st_98, token_st_99, token_st_9a, token_st_9b, token_st_9c, token_st_9d, token_st_9e, token_st_9f,
  token_st_a0, token_st_a1, token_st_a2, token_st_a3, token_st_a4,
  token_fn_a5, token_fn_a6, token_fn_a7, token_fn_a8, token_fn_a9, token_fn_aa, token_fn_ab, token_fn_ac,
  token_va_ad,
  NULL
};

const char error00[] PROGMEM = "";                    // 00 : No error
const char error01[] PROGMEM = "Syntax";              // 01 : ERROR_SYNTAX
const char error02[] PROGMEM = "Division by 0";       // 02 : ERROR_DIVZERO
const char error03[] PROGMEM = "Array index over";    // 03 : ERROR_ARRAY
const char error04[] PROGMEM = "Parameter";           // 04 : ERROR_PARA
const char error05[] PROGMEM = "Stack overflow";      // 05 : ERROR_STACK
const char error06[] PROGMEM = "Can't resume";        // 06 : ERROR_RESUME
const char error07[] PROGMEM = "Label not found";     // 07 : ERROR_LABEL
const char error08[] PROGMEM = "Not in run-mode";     // 08 : ERROR_NOTINRUN
const char error09[] PROGMEM = "PG area overflow";    // 09 : ERROR_PGOVER
const char error10[] PROGMEM = "PG empty";            // 10 : ERROR_PGOVER
const char error11[] PROGMEM = "Loop nothing";        // 11 : ERROR_NOLOOP
const char error12[] PROGMEM = "Endif not found";     // 12 : ERROR_NOENDIF
const char error13[] PROGMEM = "Expr too deep";       // 13 : ERROR_TOODEEP
const char error14[] PROGMEM = "Next";                // 14 : ERROR_UXNEXT
const char error15[] PROGMEM = "Return";              // 15 : ERROR_UXRETURN
const char error16[] PROGMEM = "Loop";                // 16 : ERROR_UXLOOP
const char error17[] PROGMEM = "Exit";                // 17 : ERROR_UXEXIT
const char error18[] PROGMEM = "Continue";            // 18 : ERROR_UXCONTINUE
const char error19[] PROGMEM = "Read";                // 18 : ERROR_UXREAD

static const char * const errorSting[] PROGMEM = {
  error00, error01, error02, error03, error04, error05, error06, error07,
  error08, error09, error10, error11, error12, error13,
  error14, error15, error16, error17, error18, error19
};

#define IS_ST_VAL(c)        (((c) & VAL_ST_MASK) == ST_VAL)
#define IS_ST_VAL_DEC(c)    (((c) & (VAL_ST_MASK | VAL_BASE_MASK)) == ST_VAL_DEC)
#define IS_VAL(c)           (IS_ST_VAL(c) || ((c) >= '0' && (c) <= '9'))
#define GET_VAL_SIZE(c)     (((c) & VAL_SIZE_MASK) + 1)
#define IS_VALID_CHR(c)     ((c)<0x3f || (c)=='^' || (c)=='|' || (c)=='~' || (c)=='[' || (c)==']')

//*************************************************
void basicInit(void)
{
  bios_init();
  initializeValiables();
  printStringFlash(F("\r\n" NAME_STR EXT_NAME_STR " " VERSION_STR "\r\n"));

  if (progLoad() > 0) {
    printStringFlash(F("Auto run\r\n"));
    if (delayMs(AUTORUN_WAIT_TIME) == ERROR_NONE) {
      programRun();
      interpreterMain();
      return;
    }
    printError();
  }
  programNew();
}

//*************************************************
void basicMain(void)
{
  uint8_t len;

  errorCode = ERROR_NONE;
  lineNumber = 0;
  returnRequest = 0;
  printStringFlash(F("OK\r\n"));
  while(true) {
    if (inputString(true) == 0) {
      if (!errorCode) continue;
      printError();
      return;
    }
    if (inputBuff[0] == '\0') continue;
    len = convertInternalCode(internalcodeBuff, inputBuff);
    if (errorCode != ERROR_NONE) {
      printError();
      return;
    }
    if (len > 1) {
      executionPointer = internalcodeBuff;
      interpreterMain();
      return;
    }
  }
}

//*************************************************
static void initializeValiables(void)
{
  programInit();
  memset(globalVariables, 0, sizeof(globalVariables));
  memset(arrayValiables , 0, sizeof(arrayValiables));
}

//*************************************************
static void programInit(void)
{
  stackPointer = 0;
  resumePointer = NULL;
  resumeLineNumber = 0;
  dataReadPointer = 0;
}

//*************************************************
static void programNew(void)
{
  progLength = 0;
  *((uint8_t*)PROGRAM_AREA_TOP) = ST_EOL;
}

//*************************************************
static void printError(void)
{
  if (errorCode != ERROR_NONE) {
    if (errorCode == ERROR_BREAK) {
      printStringFlash(F("\r\nBreak"));
    }
    else{
      printNewline();
      if (errorCode >= ERROR_UXNEXT) {
        printStringFlash(F("Unexpected "));
      }
      if (errorCode > ERROR_CODE_MAX) errorCode = ERROR_SYNTAX;
      PGM_P p = (PGM_P)pgm_read_ptr(&errorSting[errorCode]);
      printStringFlash(FPSTR(p));
      printStringFlash(F(" error"));
    }
    if (lineNumber) {
      printStringFlash(F(" in "));
      printVal(lineNumber);
    }
  }
  printNewline();
}

//*************************************************
static void executeBreak(void)
{
  if (lineNumber) {
    resumePointer = executionPointer;
    resumeLineNumber = lineNumber;
  }
  errorCode = ERROR_BREAK;
}

#if CODE_DEBUG_ENABLE
//*************************************************
static void printInternalcode(void)
{
  uint8_t len, ch, *ptr;

  ptr = executionPointer;
  len = *ptr;
  if (len > 0) {
    len++;
    while(len--) {
      ch = *ptr++;
      printString(int2str(ch, FORM_HEX, -2));
      printChar(ASCII_SP);
    }
    printNewline();
  }
}
#endif

//*************************************************
static void interpreterMain(void)
{
  uint8_t ch;
  nb_int_t *pvar;

  while(true) {
#if CODE_DEBUG_ENABLE
    printInternalcode();
#endif
    ch = *executionPointer++;
    if (ch == ST_EOL || returnRequest == REQUEST_END) {
      if (lineNumber) {
        programInit();
      }
      return;
    }
    if (lineNumber) {
      ch = *executionPointer;
	  if (ch >= '0' && ch <= '9') executionPointer++;
      else
      if (IS_ST_VAL_DEC(ch)) {
        executionPointer += GET_VAL_SIZE(ch) + 1;
      }
    }
    while(true) {
      if (checkBreakKey() < 0) {
        printError();
        return;
      }
      exprDepth = 0;
      returnRequest = 0;
      ch = *executionPointer++;
      if (ch == ST_EOL) {
        if (lineNumber == 0) {
          return;
        }
        lineNumber++;
        break;
      }
      else
      if (ch == ' ' || ch == '\t' || ch == ':') {
        /* nop */
      }
      else
      if (ch == ST_ARRAY) {
        pvar = getArrayReference();
        if (pvar == NULL) {
          printError();
          return;
        }
        proc_let(pvar);
      }
      else
      if (isupper(ch)) {
        ch -= 'A';
        proc_let(&globalVariables[ch]);
      }
      else
      if (ch == ST_COMMENT) {
        proc_comment();
      }
      else
      if (ch >= STCODE_START && ch <= STCODE_END) {
        ch -= STCODE_START;
        (*procCodeList[ch])();
      }
      else {
        errorCode = ERROR_SYNTAX;
      }
      if (errorCode != ERROR_NONE) {
        printError();
        return;
      }
      if (returnRequest) {
        break;
      }
    }
  }
}

//*************************************************
#if REPL_EDIT_ENABLE
#define CSI_SEQ     "\x1b["
#define CSI_CUF     CSI_SEQ "C"
#define CSI_CUB     CSI_SEQ "D"
#define CSI_ED      CSI_SEQ "J"
#define CSI_SCP     CSI_SEQ "s"
#define CSI_RCP     CSI_SEQ "u"

#if REPL_HISTORY_ENABLE
#define HISTOTY_BUFF_SIZE	INPUT_BUFF_SIZE
static char historyBuff[HISTOTY_BUFF_SIZE] = "";
#endif
#endif
	
//*************************************************
static uint8_t get_utf8_bytes(uint8_t ch)
{
  uint8_t c = (uint8_t)ch;
  if (c < 0b10000000) return 1;  // ASCII character (1 byte)
  if (c < 0b11000000) return 0;  // UTF-8 continuation byte
  if (c < 0b11100000) return 2;  // Start of a 2-byte UTF-8 character
  if (c < 0b11110000) return 3;  // Start of a 3-byte UTF-8 character
  if (c < 0b11111000) return 4;  // Start of a 4-byte UTF-8 character
  if (c < 0b11111100) return 5;  // Start of a 5-byte UTF-8 character
  return 6;                       // Start of a 6-byte UTF-8 character
}

//*************************************************
static uint8_t get_utf8_last_len(char* ptr)
{
  uint8_t n;
  while ((n = get_utf8_bytes(*ptr--)) == 0);
  return n;
}

//*************************************************
static uint8_t inputStringCSI(char* ptr, const char* str)
{
  uint8_t n = get_utf8_last_len(ptr);
  printString(str);
  if (n > 1) printString(str);
  return n;
}

#if REPL_EDIT_ENABLE
//*************************************************
static uint8_t get_utf8_len(char* ptr)
{
  uint8_t n;
  while ((n = get_utf8_bytes(*ptr++)) == 0);
  return n;
}

//*************************************************
static uint8_t inputStringRight(char* buff, uint8_t pos)
{
  uint8_t n = inputStringCSI(&buff[pos], CSI_CUF);
  return pos + n;
}
#endif

//*************************************************
static uint8_t inputStringLeft(char* buff, uint8_t pos)
{
  if (!pos) return pos;
  uint8_t n = inputStringCSI(&buff[pos - 1], "\b");
  return pos - n;
}

//*************************************************
static uint8_t inputString(uint8_t history_flag)
{
  uint8_t len = 0;
  uint8_t pos = 0;
#if REPL_EDIT_ENABLE
  uint8_t utf_count = 0;
  uint8_t utf_bytes = 0;
  uint8_t esc_count = 0;
  char esc_digit = 0;
#endif

  *inputBuff = '\0';

  while (1)
  {
    int16_t c = inputChar();
    if (c < 0) continue;
    uint8_t ch = (uint8_t)c;
    switch (ch)
    {
    case CHR_BREAK:
      executeBreak();
      return 0;

    case ASCII_CR:
      inputBuff[len] = '\0';
      if (len) {
#if REPL_EDIT_ENABLE
#if REPL_HISTORY_ENABLE
	    if (history_flag) {
	      memcpy(historyBuff, inputBuff, sizeof(historyBuff));
	    }
#endif
        printString(&inputBuff[pos]);
#endif
      }
      printString("\r\n");
      return (len);

#if REPL_EDIT_ENABLE
    case ASCII_ESC:
      esc_count = 1;
      esc_digit = 0;
      break;
#endif
    case ASCII_BS:
      if (pos == 0 || len == 0) break;
      if (pos == len) {
        uint8_t n = get_utf8_last_len(&inputBuff[pos - 1]);
        printString(n > 1 ? "\b\b  \b\b" : "\b \b");
        pos -= n;
        len = pos;
        inputBuff[len] = 0;
        break;
      }
#if REPL_EDIT_ENABLE
      pos = inputStringLeft(inputBuff, pos);
      /* fall through */
    case ASCII_DEL:
      if (pos != len) {
        uint16_t n = get_utf8_len(&inputBuff[pos]);
        memcpy(&inputBuff[pos], &inputBuff[pos + n], len - pos);
        len -= n;
        printString(CSI_SCP CSI_ED);    // cursor save, ED
        printString(&inputBuff[pos]);
        printString(CSI_RCP);           // cursor restore
      }
#endif
      break;

    case ASCII_HT:
      ch = ASCII_SP;
      /* fall through */
    default:
#if REPL_EDIT_ENABLE
      if (esc_count == 1) {
        esc_count = (ch == '[') ? 2 : 0;    // CSI sequence
      }
      else
      if (esc_count == 2) {
        if (isdigit(ch)) {
          esc_digit = ch;
          break;
        }
        esc_count = 0;
#if REPL_HISTORY_ENABLE
        if (ch == 'A') {                // Up
          if (historyBuff[0] == '\0') break;
          while (pos) pos = inputStringLeft(inputBuff, pos);
          printString(CSI_ED);
          memcpy(inputBuff, historyBuff, sizeof(historyBuff));
          printString(inputBuff);
          pos = len = (uint8_t)strlen(inputBuff);
        }
        else
        if (ch == 'B') {                // Down
          // Not implemented
        }
        else
#endif
        if (ch == 'C') {                 // Right
          if (pos < len) pos = inputStringRight(inputBuff, pos);
        }
        else
        if (ch == 'D') {                  // Left
          pos = inputStringLeft(inputBuff, pos);
        }
        else
        if (ch == 'H' || (ch == '~' && esc_digit == '1')) {  // Home
          while (pos) pos = inputStringLeft(inputBuff, pos);
        }
        else
        if (ch == 'F' || (ch == '~' && esc_digit == '4')) {  // End
          if (len) printString(&inputBuff[pos]);
            pos = len;
        }
      }
      else
      if (ch >= ASCII_SP) {
        if (utf_count == 0) {
          utf_bytes = get_utf8_bytes(ch);
          if (utf_bytes == 0) break;
          if (len + utf_bytes > (sizeof(inputBuff) - 2)) break;
          utf_count = utf_bytes;
        }
        for (int16_t i = len; i > pos; i--) {
          inputBuff[i] = inputBuff[i - 1];
        }
        inputBuff[pos++] = ch;
        inputBuff[++len] = 0;
        if (--utf_count) break;
        if (pos == len) {
          printString(&inputBuff[pos - utf_bytes]);
        }
        else {
          pos -= utf_bytes;
          printString(CSI_SCP); // cursor save
          printString(&inputBuff[pos]);
          printString(CSI_RCP); // cursor restore
          pos = inputStringRight(inputBuff, pos);
        }
      }
#else
      if (ch >= ASCII_SP) {
        if (len < (sizeof(inputBuff) - 2)) {
          inputBuff[pos++] = (char)ch;
          inputBuff[++len] = '\0';
          printChar(ch);
        }
      }
#endif
      break;
    }
  }
}

//*************************************************
static uint8_t hex2byte(char ch)
{
  if (ch >= '0' && ch <= '9') {
    return ch - '0';
  }
  if (ch >= 'A' && ch <= 'F') {
    return ch - 'A' + 10;
  }
  if (ch >= 'a' && ch <= 'f') {
    return ch - 'a' + 10;
  }
  return 0x10;
}

//*************************************************
static char *hex2val(char* str, nb_int_t* val)
{
  nb_int_t v = 0;
  uint8_t ch;
  while ((ch = hex2byte(*str)) < 0x10) {
    v = (v << 4) + (nb_int_t)ch;
    str++;
  }
  *val = v;
  return str;
}

//*************************************************
static char* dec2val(char* str, nb_int_t* val)
{
  nb_int_t v = 0;
  uint8_t ch;
  while (isdigit(ch = *str)) {
    v = v * 10 + (ch - '0');
    str++;
  }
  *val = v;
  return str;
}

//*************************************************
static void printVal(nb_int_t val)
{
  printString(int2str(val, 0, 0));
}

//*************************************************
static void printString(const char *str)
{
  while(*str != 0) {
    printChar(*str++);
  }
}

//*************************************************
static void printStringFlash(const __FlashStringHelper* ifsh)
{
    PGM_P p = (PGM_P)ifsh;   // Flashアドレスに変換
    char c;
    while ((c = pgm_read_byte(p++)) != 0) {
      printChar(c);
    }
}

//*************************************************
static void printNewline(void)
{
  printChar('\n');
  printChar('\r');
}

//*************************************************
static uint8_t convertInternalCode(uint8_t *dst, char *src)
{
  nb_int_t val;
  uint8_t ch, cx, cc;
  uint8_t last_st = 0;
  uint8_t *topptr = dst;
  uint8_t len = 0;

  dst++;
  while(true) {

    len = (uint8_t)(dst - topptr);
    if (len > CODE_BUFF_SIZE - 2) {
      errorCode = ERROR_PGOVER;
      return 0;
    }

    while((ch = (uint8_t)*src) <= ASCII_SP) {
      if (*src == '\0') {
        if (len <=1) len = 0;
        *dst++ = ST_EOL;
        *topptr = len;
        return len;
      }
      src++;
    }

    ch = toupper(ch);
    cx = toupper(src[1]);
    if (ch == '?') {
      *dst++ = ST_PRINT;
      src++;
    }else
    if (ch == '0' && cx == 'X') {
      src += 2;
      if (isxdigit(*src)) {
        src = hex2val(src, &val);
        *dst = ST_VAL_HEX;
        dst = set_dec_val(dst, val);
      }
      ch = ST_VAL;
    }
    else
    if (isdigit(ch)) {
      src = dec2val(src, &val);
      if (last_st == '-' || last_st == '+') {
        if (last_st == '-') {
          if (len == 1) {
            errorCode = ERROR_SYNTAX;
            return 0;
          }
          val = -val;
        }
        dst--;
      }
      *dst = ST_VAL_DEC;
      dst = set_dec_val(dst, val);
      ch = ST_VAL;
    }
    else
    if (isupper(ch)) {
      if (!isupper(cx)) {
        *dst++ = ch;
        src++;
      }
      else{
        uint8_t index = 0;
        while(true) {
          PGM_P s1 = (PGM_P)pgm_read_ptr(&keyWordList[index]);
          if (s1 == NULL) {
            errorCode = ERROR_SYNTAX;
            return 0;
          }
          char *s2 = src;
          while(true) {
            ch = toupper(*s2);
            cc = toupper(pgm_read_byte(s1));
            if (ch != cc || cc == 0) {
              break;
            }
            s1++;
            s2++;
          }
          if (cc == 0 && !isupper(ch)) {
            ch = TOKEN_START + index;
            *dst++ = ch;
            src = s2;
            break;
          }
          index++;
        }
      }
    }else
    if (ch == ST_STRING) {
      *dst++ = *src++;
      len++;
      while (true) {
      	ch = *src++;
        if (ch == '\0') {
          errorCode = ERROR_SYNTAX;
          return 0;
        }
        *dst++ = ch;
        len++;
        if (ch == ST_STRING) {
          break;
        }
        if (ch == '\\' && *src == ST_STRING) {
          *dst++ = *src++;
          len++;
        }
        if (len >= CODE_BUFF_SIZE - 3) {
          errorCode = ERROR_PGOVER;
          return 0;
        }
      }
    }else
    if (ch == ST_COMMENT) {
      if (cx == ST_COMMENT) {
        while (*++src != '\0');
      }
      else
      while (true) {
        if (++len >= CODE_BUFF_SIZE - 2) {
          errorCode = ERROR_PGOVER;
          return 0;
        }
        *dst++ = *src++;
        if (*src == '\0') {
          break;
        }
      }
    }else
    if (ch == ST_ARRAY && cx == '[') {
      *dst++ = ch;
      src++;
    }else
    if (IS_VALID_CHR(ch)) {
      *dst++ = ch;
      if ((ch == '-' || ch == '+') &&
          (last_st == ')' ||
           last_st == ']' ||
           last_st == ST_VAL ||
           isupper(last_st) ||
           last_st >= FUNC_START)) {
        ch = 0xff;
      }
      src++;
    }
    else {
      errorCode = ERROR_SYNTAX;
      return 0;
    }
    last_st = ch;
  }
}

//*************************************************
static uint8_t *label2exeptr(nb_int_t val)
{
  uint8_t ch, *ptr;
  int16_t lnum;

  lnum = 1;
  ptr = (uint8_t*)PROGRAM_AREA_TOP;
  while(true) {
    ch = *ptr++;
    if (ch == ST_EOL) {
      return NULL;
    }

    nb_int_t dec;
    uint8_t* p = get_dec_val(ptr, &dec);
    if (p != NULL) {
      if (dec == val) {
        executionPointer = ptr - 1;
        lineNumber = lnum;
        return p;
      }
      ptr = p;
    }
    while(*ptr != ST_EOL) {
      ptr = get_next_ptr(ptr);
    }
    ptr++;
    lnum++;
  }
}

//*************************************************
static uint8_t isDelimiter(uint8_t ch)
{
  return ((ch == ':')
        ||(ch == ST_EOL)
        ||(ch == ST_ELSE)
        ||(ch == ST_ELSEIF)
        ||(ch == ST_ENDIF)
        ||(ch == ST_COMMENT));
}

//*************************************************
static error_code_t checkDelimiter(void)
{
  if (errorCode == ERROR_NONE) {
    if (!isDelimiter(*executionPointer)) {
      errorCode = ERROR_SYNTAX;
    }
  }
  return errorCode;
}

//*************************************************
static error_code_t checkST(uint8_t ch)
{
  if (errorCode == ERROR_NONE) {
    if (*executionPointer != ch) {
      errorCode = ERROR_SYNTAX;
    }
    executionPointer++;
  }
  return errorCode;
}

//*************************************************
static nb_int_t *getParameterPointer(void)
{
  uint8_t ch;

  ch = *executionPointer++;
  if (ch == ST_ARRAY) {
    return getArrayReference();
  }
  else
  if (isupper(ch)) {
    return &globalVariables[ch-'A'];
  }
  errorCode = ERROR_SYNTAX;
  return NULL;
}

//*************************************************
static uint8_t *findST(const uint8_t *st_list, int16_t *lnum)
{
  uint8_t ch, count_if, *ptr;
  int16_t num;

  count_if = 0;
  ptr = executionPointer;
  num = *lnum;
  while (true) {
    while (true) {
      ch = *ptr++;
      if (ch == ST_EOL) break;
      switch(ch) {
      case ST_COMMENT :
        while (*ptr++ != ST_EOL);
        break;
      case ST_STRING :
        do {
          ch = *ptr++;
          if (ch == '\\') ptr++;
        } while (ch != ST_STRING && ch != ST_EOL);
        break;
      case ST_IF :
        count_if++;
        break;
      case ST_ENDIF :
        if (count_if) {
          count_if--;
          break;
        }
        /* fall through */
      default :
        if (IS_ST_VAL(ch)) {
          ptr += GET_VAL_SIZE(ch);
        }
        else
        if (count_if == 0) {
          const uint8_t* lp = st_list;
          while (*lp) {
            if (*lp == ch) {
              lineNumber = num;
              return (ptr);
            }
            lp++;
          } 
        }
      }
    }
    if (num == 0)  break;
    if (*ptr++ == ST_EOL) break;
    num++;
  }
  return NULL;
}

//*************************************************
static uint8_t* findNextLoop(uint8_t* ptr, uint8_t ch)
{
  static const uint8_t st_list_next_loop[] = { ST_NEXT, ST_FOR, 0, ST_LOOP, ST_WHILE, ST_DO, 0 };

  const uint8_t* st_list = st_list_next_loop;
  if (ch == ST_LOOP) st_list += 3;

  int16_t num = lineNumber;
  uint8_t count = 1;
  while (count) {
    executionPointer = ptr;
    num = lineNumber;
    ptr = findST(st_list, &num);
    if (ptr == NULL)	return ptr;
    ch = *(ptr - 1);
    if (*st_list == ch) {
      if (ch == ST_LOOP && *ptr == ST_WHILE) ptr++;
      count--;
    }
    else {
      count++;
    }
  }
  lineNumber = num;
  return ptr;
}

//*************************************************
static stack_t *pushStack(uint8_t st)
{
  stack_t *prevsp;

  if (stackPointer >= STACK_NUM) {
    errorCode = ERROR_STACK;
    return NULL;
  }
  prevsp = &stacks[stackPointer];
  prevsp->type = st;
  prevsp->returnPointer = executionPointer;
  prevsp->returnLineNumber = lineNumber;
  stackPointer++;
  return prevsp;
}

//*************************************************
static stack_t *popStack(uint8_t st)
{
  stack_t *prevsp;

  if (stackPointer == 0) {
    return NULL;
  }
  stackPointer--;
  prevsp = &stacks[stackPointer];
  if (prevsp->type != st) {
    return NULL;
  }
  return prevsp;
}

//*************************************************
static int16_t checkBreakKey(void)
{
  int16_t ch = inputChar();

  if (ch < 0) return 0;
  if (ch == CHR_BREAK) {
    executeBreak();
    return -1;
  }
  return ch;
}

//*************************************************
static char *get_StringPara_Form(uint8_t fm)
{
  int16_t len = 0;
  nb_int_t val;

  if (checkST('(')) return NULL;
  val = expr();
  if (errorCode) return NULL;
  if (*executionPointer == ',')
  {
    executionPointer++;
    len = (int16_t)expr();
  }
  if (checkST(')')) return NULL;
  return int2str(val, fm, len);
}

//*************************************************
static uint8_t *print_escaped(uint8_t* s)
{
  uint8_t val;
  uint8_t count;

  while (*s) {
    if (*s == ST_STRING) {
      s++;
      break;
    }
    if (*s == '\\') {
      s++;
      switch (*s) {
      case 'a':  printChar('\a'); break;
      case 'b':  printChar('\b'); break;
      case 'f':  printChar('\f'); break;
      case 'n':  printChar('\n'); break;
      case 'r':  printChar('\r'); break;
      case 't':  printChar('\t'); break;
      case 'v':  printChar('\v'); break;
      case '\\': printChar('\\'); break;
      case '\'': printChar('\''); break;
      case '\"': printChar('\"'); break;
      case '\?': printChar('\?'); break;

      case 'x':		// hex escape : \xHH
        s++;
        val = 0;
        count = 0;
        while(count < 2) {
          uint8_t ch = hex2byte(*s);
          if (ch > 0x0f) break;
          val = (val << 4) + ch;
          s++;
          count++;
        }
        printChar(val);
        s--;
        break;
      default:
        if (*s < '0' || *s > '7') {
          if (*s) printChar(*s); // Unknown
          break;
        }
        // oct escape: \OOO
        val = 0;
        count = 0;
        while (count < 3) {
          if (*s < '0' || *s > '7') break;
          val = (val << 3) + (*s - '0');
          s++;
          count++;
        }
        printChar(val);
        s--;
        break;
      }
    }
    else {
      printChar(*s);
    }
    s++;
  }
  return s;
}

//*************************************************
static void proc_print(void)
{
  uint8_t ch;
  nb_int_t val;
  char *p;

  uint8_t exp_flag = false;
  uint8_t lastChar = 0;
  while(true) {
    if (isDelimiter(*executionPointer)) {
      if (lastChar != ';' && lastChar != ',') {
        printNewline();
      }
      return;
    }
    lastChar = ch = *executionPointer++;
    switch (ch) {
    case ST_STRING :
      executionPointer = print_escaped(executionPointer);
      exp_flag = false;
      break;

    case ';':
      exp_flag = false;
      break;

    case ',':
      printChar('\t');
      exp_flag = false;
      break;

    case FUNC_CHR :
      val = calcValueFunc();
      if (errorCode != ERROR_NONE) return;
      if (val >= 0x100) {
        printChar(val >> 8);
      }
      printChar(val);
      exp_flag = false;
      break;

    case FUNC_HEX: 
      p = get_StringPara_Form(FORM_HEX);
      if (!errorCode) printString(p);
      exp_flag = false;
      break;

    case FUNC_DEC:
      p = get_StringPara_Form(FORM_DEC);
      if (!errorCode) printString(p);
      exp_flag = false;
      break;

    default:
      if (exp_flag) {
        errorCode = ERROR_SYNTAX;
        return;
      }
      executionPointer--;
      val = expr();
      if (errorCode != ERROR_NONE) {
        return;
      }
      printVal(val);
      exp_flag = true;
      break;
    }
  }
}

//*************************************************
static nb_int_t str2val(char* str)
{
  nb_int_t val = 0;
  uint8_t ch;

  while ((ch = (uint8_t)*str) <= ASCII_SP) {
    if (ch == '\0') {
      return val;
    }
    str++;
  }
  bool flag = (ch == '-');
  if (flag) {
    ch = *++str;
  }
  if (ch == '0' && (str[1] =='X' || str[1] =='x')) {
    str += 2;
    hex2val(str, &val);
  }
  else {
    dec2val(str, &val);
  }
  if (flag) {
    val = -val;
  }
  return val;
}

//*************************************************
static void proc_input(void)
{
  nb_int_t* pvar;

  pvar = getParameterPointer();
  if (pvar == NULL)  return;

  if (checkDelimiter()) return;

  if (inputString(false) > 0) {
    *pvar = str2val(inputBuff);
  }
}

//*************************************************
static uint8_t *goto_sub(void)
{
  nb_int_t val;
  uint8_t *rptr;

  val = expr();
  if (checkDelimiter())  return NULL;
  rptr = executionPointer;
  if (errorCode != ERROR_NONE) return NULL;
  if (label2exeptr(val) == NULL) {
    errorCode = ERROR_LABEL;
    return NULL;
  }
  returnRequest = REQUEST_GOTO;
  return rptr;
}

//*************************************************
static void proc_goto(void)
{
  goto_sub();
}

//*************************************************
static void proc_gosub(void)
{
  stack_t *prevsp;

  prevsp = pushStack(ST_GOSUB);
  if (prevsp == NULL)  return;
  prevsp->returnPointer = goto_sub();
  if (errorCode != ERROR_NONE) {
    stackPointer--;
  }
}

//*************************************************
static void proc_return(void)
{
  stack_t *prevsp;
  prevsp = &stacks[stackPointer];

  if (checkDelimiter())  return;
  while(true) {
    if (stackPointer == 0) {
      errorCode = ERROR_UXRETURN;
      return;
    }
    stackPointer--;
    prevsp--;
    if (prevsp->type == ST_GOSUB) break;
  }
  executionPointer = prevsp->returnPointer;
  lineNumber = prevsp->returnLineNumber;
}

//*************************************************
static void proc_for(void)
{
  uint8_t ch;
  nb_int_t from, to, step, *pvar;
  stack_t *prevsp;

  pvar = getParameterPointer();
  if (pvar == NULL)  return;
  if (checkST('=')) return;
  from = expr();
  if (checkST(ST_TO)) return;
  to = expr();
  if (errorCode != ERROR_NONE) return;
  ch = *executionPointer++;
  if (ch == ST_STEP) {
    step = expr();
    if (errorCode != ERROR_NONE) return;
  }
  else {
    step = 1;
    executionPointer--; /* unget it */
  }
  prevsp = pushStack(ST_FOR);
  if (prevsp == NULL)  return;
  *pvar = from;
  prevsp->pvar = pvar;
  prevsp->limit = to;
  prevsp->step = step;
}

//*************************************************
static void proc_next(void)
{
  stack_t *prevsp;

  if (checkDelimiter()) return;
  prevsp = popStack(ST_FOR);
  if (prevsp == NULL) {
    errorCode = ERROR_UXNEXT;
    return;
  }
  if (prevsp->limit == *(prevsp->pvar)) {
    return;
  }
  *(prevsp->pvar) += prevsp->step;
  if (prevsp->step > 0) {
    if (prevsp->limit < *(prevsp->pvar)) {
      return;
    }
  }
  else {
    if (prevsp->limit > *(prevsp->pvar)) {
      return;
    }
  }
  stackPointer++;
  executionPointer = prevsp->returnPointer;
  lineNumber = prevsp->returnLineNumber;
}

//*************************************************
static void proc_do(void)
{
  stack_t *prevsp;

  if (checkDelimiter()) return;
  prevsp = pushStack(ST_DO);
  if (prevsp == NULL)  return;
  prevsp->returnPointer = executionPointer - 1;
}

//*************************************************
static void proc_loop(void)
{
  nb_int_t val;
  stack_t *prevsp;

  prevsp = popStack(ST_DO);
  if (prevsp == NULL) {
    errorCode = ERROR_UXLOOP;
    return;
  }

  if (*executionPointer == ST_WHILE) {
    executionPointer++;
    val = expr();
    if (checkDelimiter()) return;
    if (!val) return;
  }
  else
  if (checkDelimiter()) return;
  executionPointer = prevsp->returnPointer;
  lineNumber = prevsp->returnLineNumber;
}

//*************************************************
static uint8_t* skipToDelimiter(uint8_t* ptr)
{
  while (!isDelimiter(*ptr)) ptr++;
  return ptr;
}

//*************************************************
static void proc_while(void)
{
  nb_int_t val;
  stack_t *prevsp;
  uint8_t *ptr;

  ptr = executionPointer;
  val = expr();
  if (checkDelimiter()) return;
  if (val) {
    prevsp = pushStack(ST_DO);
    if (prevsp == NULL)  return;
    prevsp->returnPointer = ptr - 1;
  }
  else {
    ptr = findNextLoop(ptr, ST_LOOP);
    if (ptr == NULL) {
      errorCode = ERROR_NOLOOP;
      return;
    }
    executionPointer = skipToDelimiter(ptr);
  }
}

//*************************************************
static void proc_exit(void)
{
  uint8_t* ptr = NULL;
  if (checkDelimiter()) return;
  if (stackPointer) {
    stack_t* prevsp = &stacks[stackPointer - 1];

    if (prevsp->type == ST_DO) {
      ptr = findNextLoop(executionPointer, ST_LOOP);
    }
    else
    if (prevsp->type == ST_FOR) {
      ptr = findNextLoop(executionPointer, ST_NEXT);
    }
  }
  if (ptr) {
    stackPointer--;
    executionPointer = skipToDelimiter(ptr);
    return;
  }
  errorCode = ERROR_UXEXIT;
}

//*************************************************
static void proc_continue(void)
{
  uint8_t* ptr = NULL;
  if (checkDelimiter()) return;
  if (stackPointer) {
    stack_t* prevsp = &stacks[stackPointer - 1];

    if (prevsp->type == ST_DO) {
    	stackPointer--;
      executionPointer = prevsp->returnPointer;
      lineNumber = prevsp->returnLineNumber;
      return;
    }
    else
    if (prevsp->type == ST_FOR) {
      ptr = findNextLoop(executionPointer, ST_NEXT);
    }
    if (ptr) return;
  }
  errorCode = ERROR_UXCONTINUE;
}

//*************************************************
static void proc_if (void)
{
  static const uint8_t st_list[] = { ST_ENDIF, ST_ELSE, ST_ELSEIF, 0 };
  nb_int_t val;
  uint8_t ch, *ptr;

  do{
    val = expr();
    if (checkST(ST_THEN)) return;
    if (val) {
      if (IS_VAL(*executionPointer)) {
        proc_goto();
      }
      return;
    }
    ptr = findST(st_list, &lineNumber);
    if (ptr == NULL) {
      errorCode = ERROR_NOENDIF;
      return;
    }
    executionPointer = ptr;
    ch =  *(executionPointer - 1);
  }while(ch == ST_ELSEIF);

  if (ch == ST_ELSE) {
    if (IS_VAL(*executionPointer)) {
      proc_goto();
    }
  }
}

//*************************************************
static void proc_else(void)
{
  static const uint8_t st_list[] = { ST_ENDIF, 0 };
  uint8_t *ptr;

  ptr = findST(st_list, &lineNumber);
  if (ptr == NULL) {
    errorCode = ERROR_NOENDIF;
    return;
  }
  executionPointer = ptr;
}

//*************************************************
static void proc_elseif (void)
{
  proc_else();
}

//*************************************************
static void proc_endif (void)
{
  /* Nothing to do */
  checkDelimiter();
}

//*************************************************
static void programRun(void)
{
  initializeValiables();
  errorCode = ERROR_NONE;
  lineNumber = 1;
  executionPointer = (uint8_t*)PROGRAM_AREA_TOP;
  returnRequest = REQUEST_GOTO;
}

//*************************************************
static void proc_run(void)
{
  if (checkDelimiter()) return;
  programRun();
}

//*************************************************
static void proc_resume(void)
{
  if (checkDelimiter()) return;
  if (resumePointer == NULL) {
    errorCode = ERROR_RESUME;
    return;
  }
  executionPointer = resumePointer;
  lineNumber = resumeLineNumber;
}

//*************************************************
static void proc_stop(void)
{
  if (checkDelimiter()) return;
  executeBreak();
}

//*************************************************
static void proc_end(void)
{
  if (checkDelimiter()) return;
  returnRequest = REQUEST_END;
  programInit();
}

//*************************************************
static void proc_new(void)
{
  if (checkDelimiter()) return;
  initializeValiables();
  programNew();
}

//*************************************************
static void proc_list(void)
{
  nb_int_t val;
  uint8_t flag, ch, *ptr;

  if (checkDelimiter()) return;
  ptr = (uint8_t*)PROGRAM_AREA_TOP;
  while(*ptr++ != ST_EOL) {
    flag = true;
    while(true) {
      ch = *ptr;
      uint8_t *p = get_dec_val(ptr, &val);
      if (p != NULL) {
        if (ch & VAL_BASE_HEX) {
          printString("0x");
          printString(int2str(val, FORM_HEX, 0));
        }
        else {
          printVal(val);
          if (flag) {
            printChar(ASCII_SP);
          }
        }
        ptr = p;
        continue;
      }
      ptr++;
      if (ch == ST_EOL) {
        printNewline();
        break;
      }
      else
      if (ch == ST_STRING) {
        printChar(ch);
        do{
          ch = *ptr++;
          printChar(ch);
          if (ch == '\\') {
            printChar(*ptr++);
          }
        }while(ch != ST_STRING);
      }
      else
      if (ch == ST_COMMENT) {
        printChar(ch);
        while (*ptr != ST_EOL) {
          printChar(*ptr++);
        }
      }
      else
      if (ch >= TOKEN_START) {
        if (!flag && (ch >= STSP_START && ch <= STSP_END)) {
          printChar(ASCII_SP);
        }
        PGM_P p = (PGM_P)pgm_read_ptr(&keyWordList[ch-TOKEN_START]);
        char c;
        while ((c = pgm_read_byte(p++)) != 0) {
#if LIST_STYLE == 0
          c = toupper(c);
#elif LIST_STYLE == 1
          c = tolower(c);
#endif
          printChar(c);
        }
        if (ch <= STSP_END && !isDelimiter(*ptr)) {
          printChar(ASCII_SP);
        }
      }
      else {
#if LIST_STYLE != 0
        ch = tolower(ch);
#endif
        printChar(ch);
      }
      flag = false;
    }
  }

  if (progLength < 2) progLength = 0;
  printStringFlash(F("["));
  printVal(progLength);
  printStringFlash(F(" bytes]\r\n"));
}

//*************************************************
static void proc_prog(void)
{
  uint8_t len, *ptr, *src;
  uint16_t  remain;

  if (checkDelimiter()) return;
  if (lineNumber) {
    errorCode = ERROR_NOTINRUN;
    return;
  }
  remain = PROGRAM_AREA_SIZE - 3;
  progLength = 0;
  ptr = (uint8_t*)PROGRAM_AREA_TOP;
  while(true) {
    errorCode = ERROR_NONE;
    printChar('>');
    if (inputString(false) > 0) {
	    if (inputBuff[0] == CHR_PROG_TERM) {
	      returnRequest = REQUEST_END;
	      break;
	    }
	    len = convertInternalCode(internalcodeBuff, inputBuff);
	    if (remain < len) {
	      errorCode = ERROR_PGOVER;
	    }
	    if (errorCode != ERROR_NONE) {
	      printError();
	    }
	    else
	    if (len > 0) {
	      len++;
	      progLength += len;
	      remain -= len;
	      src = internalcodeBuff;
	      while(len-- > 0) {
	        *ptr++ = *src++;
	      }
	    }
		}
  }
  *ptr++ = ST_EOL;
  if (progLength > 1) progLength++;
}

//*************************************************
static void proc_save(void)
{
  uint8_t flag = *executionPointer;
  if (flag == '0' || flag == '!') {
    executionPointer++;
  }
  if (checkDelimiter()) return;
  if (lineNumber) {
    errorCode = ERROR_NOTINRUN;
    return;
  }

  if (flag == '0') {
    bios_eepEraseBlock(EEP_HEADER_ADDR, EEP_HEADER_SIZE + PROGRAM_AREA_SIZE);
    return;
  }

  uint8_t *ptr = (uint8_t*)PROGRAM_AREA_TOP;
  if (*ptr == ST_EOL) {
    errorCode = ERROR_PGEMPTY;
    return;
  }

  EEP_Header_t eep;
  eep.magic1 = EEP_MAGIC_1;
  eep.magic2 = EEP_MAGIC_2;
  eep.verMajor = VERSION_MAJOR;
  eep.verMinor = VERSION_MINOR;
  eep.progLength = progLength;
  eep.autoRun = (flag == '!');
  eep.reserved = 0x00;

  bios_eepWriteBlock(EEP_HEADER_ADDR, (uint8_t*)&eep, EEP_HEADER_SIZE);
  bios_eepWriteBlock(EEP_PROGRAM_ADDR, ptr,  (uint16_t)progLength);
}

//*************************************************
static int8_t progLoad(void)
{
  EEP_Header_t eep;
  bios_eepReadBlock(EEP_HEADER_ADDR, (uint8_t*)&eep, EEP_HEADER_SIZE);
  if (eep.magic1 != EEP_MAGIC_1 || eep.magic2 != EEP_MAGIC_2) {
     errorCode = ERROR_PGEMPTY;
    return -1;
  }
  if (eep.progLength < 2) {
     errorCode = ERROR_PGEMPTY;
    return -1;
  }
  if (eep.progLength > PROGRAM_AREA_SIZE) {
    errorCode = ERROR_PGOVER;
    return -1;
  }
  progLength = eep.progLength;
  bios_eepReadBlock(EEP_PROGRAM_ADDR, PROGRAM_AREA_TOP, (uint16_t)progLength);
  return eep.autoRun;
}

//*************************************************
static void proc_load(void)
{
  if (checkDelimiter()) return;
  if (lineNumber) {
    errorCode = ERROR_NOTINRUN;
    return;
  }
  progLoad();
}

//*************************************************
static void proc_comment(void)
{
  while(*executionPointer != ST_EOL) {
    executionPointer++;
  }
}

//*************************************************
static uint8_t get_arg2(nb_int_t *val_1, nb_int_t *val_2)
{
  *val_1 = expr();
  if (!checkST(',')) {
    *val_2 = expr();
    checkDelimiter();
  }
  return errorCode;
}

//*************************************************
static void proc_outp(void)
{
  nb_int_t val_1, val_2;

  if (get_arg2( &val_1, &val_2) == ERROR_NONE) {
    if (bios_writeGpio(val_1, val_2)) {
      errorCode = ERROR_PARA;
    }
  }
}

//*************************************************
static error_code_t delayMs(nb_int_t val)
{
  nb_int_t waitStart = bios_getSystemTick();

  while(checkBreakKey() >= 0) {
    nb_int_t elapsed = bios_getSystemTick() - waitStart;
    if (elapsed > val) break;
  }
  return errorCode;
}

//*************************************************
static void proc_delay(void)
{
  nb_int_t val;

  val = expr();
  if (checkDelimiter()) return;
  delayMs(val);
}

//*************************************************
static void proc_pause(void)
{
  if (checkDelimiter()) return;
  while (checkBreakKey() == 0);
}

//*************************************************
static void proc_reset(void)
{
  if (checkDelimiter()) return;
  bios_systemReset();
}

//*************************************************
static error_code_t checkDivZero(nb_int_t val)
{
  if (errorCode == ERROR_NONE) {
    if (val == 0) {
      errorCode = ERROR_DIVZERO;
    }
  }
  return errorCode;
}

//*************************************************
static void let_variable(nb_int_t *pvar)
{
  uint8_t op = *executionPointer;

  if (op == executionPointer[1]) {
    executionPointer += 2;
    if (op == '+') { (*pvar)++; return; }
    if (op == '-') { (*pvar)--; return; }
    if (op !='<' && op != '>') {errorCode = ERROR_SYNTAX;return;}
  }
  else
  if (op=='+' || op=='-' || op=='*' || op=='/' || op=='%' || op=='|' || op=='&' || op=='^') {
    executionPointer++;
  }
  if (checkST('=')) return;
  nb_int_t val = expr();
  if (errorCode) return;

  switch (op) {
  case '+' : *pvar += val; break;
  case '-' : *pvar -= val; break;
  case '*' : *pvar *= val; break;
  case '/' : if (!checkDivZero(val)) *pvar /= val; break;
  case '%' : if (!checkDivZero(val)) *pvar %= val; break;
  case '|' : *pvar |= val; break;
  case '&' : *pvar &= val; break;
  case '^' : *pvar ^= val; break;
  case '<' : *pvar <<= val; break;
  case '>' : *pvar >>= val; break;
  case '=' : *pvar = val; break;
  default : break;
  }
}

//*************************************************
static void proc_let(nb_int_t *pvar)
{
  let_variable(pvar);
  checkDelimiter();
}

//*************************************************
static void proc_randomize(void)
{
  nb_int_t val;

  val = expr();
  if (checkDelimiter()) return;
  bios_randomize(val);
}

//*************************************************
static void proc_data(void)
{
  while (!isDelimiter(*executionPointer)) {
    executionPointer = get_next_ptr(executionPointer);
  }
}

//*************************************************
static void proc_read(void)
{
  static const uint8_t st_list[] = { ST_DATA, 0 };
  nb_int_t *pvar, val;
  uint8_t ch, *ptr, *ptrsave;

  pvar = getParameterPointer();
//if (pvar == NULL)  return;
  if (checkDelimiter()) return;

  ptrsave = executionPointer;
  executionPointer = (dataReadPointer == 0) ? (uint8_t*)PROGRAM_AREA_TOP + 1 : dataReadPointer;

  do{
    if (*executionPointer != ',') {
      int16_t lnum = lineNumber;
      ptr = findST(st_list, &lnum);
      if (ptr == NULL) {
        errorCode = ERROR_UXREAD;
        break;
      }
      executionPointer = ptr;
    }
    else{
      executionPointer++;
    }
    val = expr();
    if (errorCode != ERROR_NONE) {
      break;
    }
    *pvar = val;
    ch = *executionPointer;
    if (isDelimiter(ch) || ch == ',') {
      break;
    }
    errorCode = ERROR_PARA;
  } while (false);
  dataReadPointer = executionPointer;
  executionPointer = ptrsave;
}

//*************************************************
static void proc_restore(void)
{
  if (checkDelimiter()) return;
  dataReadPointer = 0;
}

//*************************************************
static void proc_pwm(void)
{
  nb_int_t val_1, val_2;

  if (get_arg2(&val_1, &val_2) == ERROR_NONE) {
    if (bios_setPwm(val_1, val_2)) {
      errorCode = ERROR_PARA;
    }
  }
}

//*************************************************
static nb_int_t inkey_func(nb_int_t val)
{
  if (val < 0) val = 0;
  nb_int_t waitStart = bios_getSystemTick();

  while (1) {
    int16_t ch = checkBreakKey();
    if (ch != 0) return ch;
    nb_int_t elapsed = bios_getSystemTick() - waitStart;
    if (val && elapsed > val) return -1;
  }
}

//*************************************************
static nb_int_t *getArrayReference(void)
{
  int16_t index;
  nb_int_t *pvar;

  if (checkST('[')) return NULL;
  index = expr();
  if (errorCode != ERROR_NONE) return NULL;
  if (index < 0 || index >= ARRAY_INDEX_NUM) {
    errorCode = ERROR_ARRAY;
    return NULL;
  }
  pvar = &arrayValiables[index];
  if (checkST(']')) return NULL;
  return pvar;
}

//*************************************************
static nb_int_t calcValueFunc(void)
{
  nb_int_t val;

  if (checkST('(')) return -1;
  val = expr();
  if (checkST(')')) return -1;
  return val;
}

//*************************************************
static nb_int_t calcValue(void)
{
  uint8_t ch;
  nb_int_t *pvar, val;

  if (++exprDepth > EXPR_DEPTH_MAX) {
    errorCode = ERROR_TOODEEP;
    return -1;
  }

  uint8_t* p = get_dec_val(executionPointer, &val);
  if (p != NULL) {
    executionPointer = p;
    return val;
  }
  ch = *executionPointer++;
  if (isupper(ch)) {
    ch -= 'A';
    return globalVariables[ch];
  }
  if (ch == ST_ARRAY) {
    pvar = getArrayReference();
    if (pvar == NULL) return -1;
    return *pvar;
  }

  switch(ch) {
  case '(':
    val = expr();
    if (checkST(')')) break;
    return val;
  case '-':
    return -calcValue();
  case '!' :
    return calcValue() == 0;
  case '~' :
    return ~calcValue();
  case FUNC_RND :
    val = calcValueFunc();
    if (errorCode == ERROR_NONE) {
      return bios_rand(val);
    }
    break;
  case FUNC_ABS :
    val = calcValueFunc();
    if (errorCode == ERROR_NONE) {
      if (val < 0) val = -val;
      return val;
    }
    break;
  case FUNC_INP :
    val = calcValueFunc();
    if (errorCode == ERROR_NONE) {
      val = bios_readGpio(val);
      if (val < 0) {
        errorCode = ERROR_PARA;
      }
      return val;
    }
    break;
  case FUNC_ADC :
    val = calcValueFunc();
    if (errorCode == ERROR_NONE) {
      val = bios_readAdc(val);
      if (val < 0) {
        errorCode = ERROR_PARA;
      }
      return val;
    }
    break;
  case FUNC_INKEY :
    val = calcValueFunc();
    if (errorCode == ERROR_NONE) {
      val = inkey_func(val);
      return val;
    }
    break;
  case SVAR_TICK :
    return bios_getSystemTick();
  default :
    errorCode = ERROR_SYNTAX;
  }
  return -1;
}

//*************************************************
static nb_int_t expr4th(void)
{
  nb_int_t acc, val;
  uint8_t ch;

  acc = calcValue();
  if (errorCode != ERROR_NONE) { return -1; }
  while(true) {
    ch = *executionPointer++;
    switch(ch) {
    case '*':
      acc = acc * calcValue();
      break;
    case '/':
      val = calcValue();
      if (!checkDivZero(val)) {
        acc = acc / val;
      }
      break;
    case '%':
      val = calcValue();
      if (!checkDivZero(val)) {
        acc = acc % val;
      }
      break;
    default:
      executionPointer--;
      return acc;
    }
    if (errorCode != ERROR_NONE) { return -1; }
  }
}

//*************************************************
static nb_int_t expr3nd(void)
{
  nb_int_t acc;
  uint8_t ch;

  acc = expr4th();
  if (errorCode != ERROR_NONE) { return -1; }
  while(true) {
    ch = *executionPointer++;
    switch(ch) {
    case '+':
      acc = acc + expr4th();
      break;
    case '-':
      acc = acc - expr4th();
      break;
    default:
      executionPointer--;
      return acc;
    }
    if (errorCode != ERROR_NONE) { return -1; }
  }
}

//*************************************************
static nb_int_t expr2nd(void)
{
  nb_int_t acc, tmp;
  uint8_t ch, ch2;

  acc = expr3nd();
  if (errorCode != ERROR_NONE) { return -1; }
  while(true) {
    ch = *executionPointer++;
    switch(ch) {
    case '>':
      ch2 = *executionPointer++;
      if (ch2 == '=') {
        tmp = expr3nd();
        acc = (acc >= tmp);   // >=
      }
      else
      if (ch2 == ch)  {
        tmp = expr3nd();
        acc = (acc >> tmp);   // >>
      }
      else {
        executionPointer--;
        tmp = expr3nd();
        acc = (acc > tmp);    // >
      }
      break;
    case '<':
      ch2 = *executionPointer++;
      if (ch2 == '=') {
        tmp = expr3nd();
        acc = (acc <= tmp);   // <=
      }
      else
      if (ch2 == '>') {
        tmp = expr3nd();
        acc = (acc != tmp);   // <>
      }
      else
      if (ch2 == ch)  {
        tmp = expr3nd();
        acc = (acc << tmp);   // <<
      }
      else {
        executionPointer--;
        tmp = expr3nd();
        acc = (acc < tmp);    // <
      }
      break;
    case '=':
      if (*executionPointer == ch) executionPointer++;
      tmp = expr3nd();
      acc = (acc == tmp);     // =, ==
      break;
    case '!':
	  if (*executionPointer == '=') {
	    executionPointer++;
	    tmp = expr3nd();
	    acc = (acc != tmp);     // !=
	    break;
	  }
	  /* fall through */
	default:
      executionPointer--;
      return acc;
    }
    if (errorCode != ERROR_NONE) { return -1; }
  }
}

//*************************************************
static nb_int_t expr(void)
{
  nb_int_t acc;
  uint8_t ch;

  acc = expr2nd();
  if (errorCode != ERROR_NONE) { return -1; }
  while(true) {
    ch = *executionPointer++;
    switch(ch) {
    case '&' :
      if (*executionPointer != ch) {
        acc = acc & expr2nd();    // &
        break;
      }
      executionPointer++;
      acc = !!acc && !!expr2nd(); // &&
      break;
    case '|' :
      if (*executionPointer != ch) {
        acc = acc | expr2nd();    // |
        break;
      }
      executionPointer++;
      acc = !!acc || !!expr2nd(); // ||
      break;
    case '^' :
      acc = acc ^ expr2nd();      // ^
      break;
    default:
      executionPointer--;
      return acc;
    }
    if (errorCode != ERROR_NONE) { return -1; }
  }
}

//*************************************************
static char *int2str(nb_int_t para, uint8_t ff, int16_t len)
{
  static char str[13];
  char *s, ch , flag, fx;
  nb_uint_t val;
  int8_t dot ;

#if PRINT_HEX_STYLE == 1
  ff |= FORM_LOWER;
#endif

  if (len < 0) {
    ff |= FORM_ZERO;
    len = -len;
  }

  dot = len / 100;
  len = len % 100;
  if (len > 10) len = 10;
  if (dot == 0) dot = -1;

  fx = str[12] = '\0';
  if ((para < 0) && (ff & FORM_FHEX) != FORM_HEX) {
    fx = flag = '-';
    val = (nb_uint_t) -para;
  }
  else {
    flag = ' ';
    if (ff & FORM_PLUS) fx = flag = '+';
    val = para;
  }
  s = &str[10];
  while (1)
  {
    if (ff & FORM_HEX) {
      ch = (val & 0x0f) + '0';
      if (ch > '9') {
        ch += 0x07 + (ff & FORM_LOWER);
      }
      val >>= 4;
    }
    else {
      ch = (val % 10) + '0';
      val /= 10;
    }
    *s-- = ch;
    if (dot >= 0 && (--dot == 0)) *s-- = '.';
    if (len > 0 && (--len == 0)) break;
    if (dot < 0 && val == 0) break;
  }

  if (ff & FORM_FLAG) {
    while (len > 0) {
      len--;
      *s-- = ASCII_SP + (ff & FORM_ZERO);
    }
    *s = flag;
    return (s);
  }

  if (ff & FORM_ZERO) {
    if (len == 0 && fx) {
      *s = flag;
      return (s);
    }
    while (len > 0) {
      len--;
      *s-- = (len == 0 && fx) ? flag : '0';
    }
  }
  else {
    if (fx) {
      *s-- = flag;
      if (len > 0)
        len--;
    }
    while (len > 0) {
      len--;
      *s-- = ' ';
    }
  }
  return (s + 1);
}

//*************************************************
static uint8_t* get_dec_val(uint8_t* ptr, nb_int_t* val)
{
  if (ptr[0] >= '0' && ptr[0] <= '9') {
    *val = ptr[0] - '0';
    return ptr + 1;
  }

  if ((*ptr & VAL_ST_MASK) != ST_VAL) {
    return NULL;
  }

  uint8_t size = *ptr & VAL_SIZE_MASK;

  if (size == VAL_SIZE_8) {
    *val = (int8_t)ptr[1];
    return ptr + 2;
  }

#if NANOBASIC_INT32_EN == 0
  *val =
    ((nb_int_t)(int8_t)ptr[2] << 8) |
    ((nb_int_t)ptr[1]);
  return ptr + 3;
#else
  if (size == VAL_SIZE_16) {
    *val =
      ((nb_int_t)(int8_t)ptr[2] << 8) |
      ((nb_int_t)ptr[1]);
    return ptr + 3;
  }

  if (size == VAL_SIZE_24) {
    *val =
      ((nb_int_t)(int8_t)ptr[3] << 16) |
      ((nb_int_t)ptr[2] << 8) |
      ((nb_int_t)ptr[1]);
    return ptr + 4;
  }
  *val =
    ((nb_int_t)(int8_t)ptr[4] << 24) |
    ((nb_int_t)ptr[3] << 16) |
    ((nb_int_t)ptr[2] << 8) |
    ((nb_int_t)ptr[1]);
  return ptr + 5;
#endif
}

//*************************************************
static uint8_t* set_dec_val(uint8_t* ptr, nb_int_t val)
{
  if (ptr[0] == ST_VAL_DEC) {
    if (val >= 0 && val <= 9) {
      ptr[0] = val + '0';
      return ptr + 1;
    }
  }

  nb_uint_t u = (nb_uint_t)val;

  ptr[1] = (uint8_t)u;
  if (val >= INT8_MIN && val <= INT8_MAX) {
    ptr[0] |= VAL_SIZE_8;
    return ptr + 2;
  }

  ptr[2] = (uint8_t)(u >> 8);
#if NANOBASIC_INT32_EN == 0
  ptr[0] |= VAL_SIZE_16;
  return ptr + 3;
#else
  if (val >= INT16_MIN && val <= INT16_MAX) {
    ptr[0] |= VAL_SIZE_16;
    return ptr + 3;
  }

  ptr[3] = (uint8_t)(u >> 16);
  if (val >= -8388608 && val <= 8388607) {
    ptr[0] |= VAL_SIZE_24;
    return ptr + 4;
  }

  ptr[4] = (uint8_t)(u >> 24);
  ptr[0] |= VAL_SIZE_32;
  return ptr + 5;
#endif
}

//*************************************************
static uint8_t *get_next_ptr(uint8_t* ptr)
{
  uint8_t ch = *ptr++;

  if ((ch & VAL_ST_MASK) == ST_VAL) {
    ptr += GET_VAL_SIZE(ch);
  }
  return ptr;
}
