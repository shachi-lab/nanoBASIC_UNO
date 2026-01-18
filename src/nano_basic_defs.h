/*
 * nanoBASIC - Core Language Definitions
 * --------------------------------------------
 * Platform-independent definitions used by the
 * nanoBASIC interpreter engine.
 *
 * This header provides:
 *   - Statement and token IDs
 *   - Built-in value/function identifiers
 *   - Error code definitions
 *   - Stack structures for FOR/NEXT loops
 *
 * These definitions form the core specification
 * of the nanoBASIC language and remain common
 * across all platforms (UNO, ESP32, ARM, etc.).
 *
 * Only the BIOS layer is platform-dependent.
 *
 * GitHub: https://github.com/shachi-lab
 * Copyright (c) 2025 shachi-lab
 * License: MIT
 */

#ifndef __NANO_BASIC_DEFS_H
#define __NANO_BASIC_DEFS_H

#include "nano_basic_uno_conf.h"

// Number of variables
#define VARIABLE_NUM  (('Z' - 'A') + 1)

// ASCII Code
typedef enum {
  ASCII_NUL     = 0x00,
  ASCII_SOH     = 0x01,
  ASCII_STX     = 0x02,
  ASCII_ETX     = 0x03,
  ASCII_EOT     = 0x04,
  ASCII_ENQ     = 0x05,
  ASCII_ACK     = 0x06,
  ASCII_BEL     = 0x07,
  ASCII_BS      = 0x08,
  ASCII_HT      = 0x09,
  ASCII_LF      = 0x0a,
  ASCII_VT      = 0x0b,
  ASCII_FF      = 0x0c,
  ASCII_CR      = 0x0d,
  ASCII_SO      = 0x0e,
  ASCII_SI      = 0x0f,
  ASCII_ESC     = 0x1b,
  ASCII_SP      = 0x20,
  ASCII_DEL     = 0x7f,
} ascii_code_e;
typedef uint8_t ascii_code_t;

// Internal Code 
typedef enum {
  ST_EOL        = 0x00,
  ST_VAL        = 0x08,  // 0000 1xxx
  ST_VAL_DEC    = 0x08,  // 0000 10xx
  ST_VAL_HEX    = 0x0c,  // 0000 11xx
  ST_STRING     = 0x22,
  ST_ARRAY      = '@',
  ST_COMMENT    = 0x27,

  TOKEN_START   = 0x80,
  STCODE_START  = 0x80,
  ST_PRINT      = 0x80,
  ST_INPUT      = 0x81,
  ST_GOTO       = 0x82,
  ST_GOSUB      = 0x83,
  ST_RETURN     = 0x84,
  ST_FOR        = 0x85,
  ST_NEXT       = 0x86,
  ST_DO         = 0x87,
  ST_LOOP       = 0x88,
  ST_WHILE      = 0x89,
  ST_IF         = 0x8a,
  ST_RUN        = 0x8b,
  ST_RESUME     = 0x8c,
  ST_STOP       = 0x8d,
  ST_END        = 0x8e,
  ST_NEW        = 0x8f,
  ST_LIST       = 0x90,
  ST_PROG       = 0x91,
  ST_SAVE       = 0x92,
  ST_LOAD       = 0x93,
  ST_DELAY      = 0x94,
  ST_PAUSE      = 0x95,
  ST_RESET      = 0x96,
  ST_EXIT       = 0x97,
  ST_CONTINUE   = 0x98,
  ST_RONDOMIZE  = 0x99,
  ST_DATA       = 0x9a,
  ST_READ       = 0x9b,
  ST_RESTORE    = 0x9c,
  ST_OUTP       = 0x9d,
  ST_PWM        = 0x9e,

  STSP_START    = 0x9f,
  ST_ELSE       = 0x9f,
  ST_ELSEIF     = 0xa0,
  ST_ENDIF      = 0xa1,
  STCODE_END    = 0xa1,

  ST_THEN       = 0xa2,
  ST_TO         = 0xa3,
  ST_STEP       = 0xa4,
  STSP_END      = 0xa4,

  FUNC_START    = 0xa5,
  FUNC_RND      = 0xa5,
  FUNC_ABS      = 0xa6,
  FUNC_INP      = 0xa7,
  FUNC_ADC      = 0xa8,
  FUNC_INKEY    = 0xa9,
  FUNC_CHR      = 0xaa,
  FUNC_DEC      = 0xab,
  FUNC_HEX      = 0xac,
  FUNC_END      = 0xac,

  SVAR_START    = 0xad,
  SVAR_TICK     = 0xad,
  SVAR_END      = 0xad
} internal_code_e;
typedef uint8_t internal_code_t;

// Error Code
typedef enum {
  ERROR_NONE      = 0,
  ERROR_BREAK     = 255,
  ERROR_SYNTAX    = 1,
  ERROR_DIVZERO   = 2,
  ERROR_ARRAY     = 3,
  ERROR_PARA      = 4,
  ERROR_STACK     = 5,
  ERROR_RESUME    = 6,
  ERROR_LABEL     = 7,
  ERROR_NOTINRUN  = 8,
  ERROR_PGOVER    = 9,
  ERROR_PGEMPTY   = 10,
  ERROR_NOLOOP    = 11,
  ERROR_NOENDIF   = 12,
  ERROR_TOODEEP   = 13,
  ERROR_UXNEXT    = 14,
  ERROR_UXRETURN  = 15,
  ERROR_UXLOOP    = 16,
  ERROR_UXEXIT    = 17,
  ERROR_UXCONTINUE= 18,
  ERROR_UXREAD    = 19,
  ERROR_CODE_MAX  = 19,
} error_code_e;
typedef uint8_t error_code_t;

// Request Code
typedef enum {
  REQUEST_NOTHING = 0,
  REQUEST_GOTO    = 1,
  REQUEST_END     = 2,
} request_code_e;
typedef uint8_t request_code_t;

// Integer type definition
#if NANOBASIC_INT32_EN
typedef int32_t nb_int_t;
typedef uint32_t nb_uint_t;
#else
typedef int16_t nb_int_t;
typedef uint16_t nb_uint_t;
#endif

// Stack structure
typedef struct {
  uint8_t   type;             // ST type (DO/FOR/WHILE)
  uint8_t   *returnPointer;
  int16_t   returnLineNumber;
  nb_int_t  *pvar;            // counter variable
  nb_int_t  limit;            // limit value
  nb_int_t  step;             // step value
} stack_t;

// Special character definitions
#define CHR_BREAK       ASCII_ETX
#define CHR_PROG_TERM   '#'

// ST_VAL bytecode format (value literal)
#define VAL_ST_MASK   0xf8    // 1111 1xxx 
#define VAL_BASE_DEC  0x00    // xxxx x0xx 
#define VAL_BASE_HEX  0x04    // xxxx x1xx 
#define VAL_BASE_MASK 0x04    // xxxx x1xx 
#define VAL_SIZE_8    0x00    // xxxx xx00 
#define VAL_SIZE_16   0x01    // xxxx xx01 
#define VAL_SIZE_24   0x02    // xxxx xx10 
#define VAL_SIZE_32   0x03    // xxxx xx11 
#define VAL_SIZE_MASK 0x03    // xxxx xx11 
// ST_VAL bytecode format (bit structures)
typedef struct {
  uint8_t code : 5;	  // fixed (0x01)
  uint8_t hex  : 1;	  // 0:Dec / 1:Hex
  uint8_t size : 2;   // bytes - 1
} st_val_t;

// int2str format
#define FORM_NONE     0x00
#define FORM_FLAG     0x01
#define FORM_PLUS     0x02
#define FORM_HEX      0x04
#define FORM_DEC      0x80
#define FORM_ZERO     0x10
#define FORM_LOWER    0x20
#define FORM_HEXU     FORM_HEX
#define FORM_HEXL     (FORM_HEX | FORM_LOWER)
#define FORM_FHEX     (FORM_HEX | FORM_FLAG)

// EEPROM Program Storage Header
typedef struct {
  uint8_t magic1;       // 'n'
  uint8_t magic2;       // 'B'
  uint8_t verMajor;
  uint8_t verMinor;
  int16_t progLength;
  uint8_t autoRun;
  uint8_t reserved;
} EEP_Header_t;

#define EEP_MAGIC_1         'n'
#define EEP_MAGIC_2         'B'
#define EEP_HEADER_ADDR      0
#define EEP_HEADER_SIZE      sizeof(EEP_Header_t)
#define EEP_PROGRAM_ADDR     (EEP_HEADER_ADDR + EEP_HEADER_SIZE)

#endif
