/*
 * NanoBASIC - Core Language Definitions
 * --------------------------------------------
 * Platform-independent definitions used by the
 * NanoBASIC interpreter engine.
 *
 * This header provides:
 *   - Statement and token IDs
 *   - Built-in value/function identifiers
 *   - Error code definitions
 *   - Stack structures for FOR/NEXT loops
 *
 * These definitions form the core specification
 * of the NanoBASIC language and remain common
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

#define VARIABLE_NUM  (('Z' - 'A') + 1)

enum {
  ASCII_CR      = 0x0d,
  ASCII_LF      = 0x0a,
  ASCII_BS      = 0x08,
  ASCII_NUL     = 0x00,
} ascii_code_e;

#define CHR_BREAK       0x03
#define CHR_PROG_TERM   '#'

// Internal Code 
enum {
  ST_EOL        = 0x00,
  ST_DECVAL     = 0x01,
  ST_HEXVAL     = 0x02,
  ST_STRING     = 0x22,
  ST_ARRAY      = '@',
  ST_HEXCHR     = '$',
  ST_COMMENT    = 0x27,
  VAL_ZERO      = 0x30,

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

  FUNC_RND      = 0xa5,
  FUNC_ABS      = 0xa6,
  FUNC_INP      = 0xa7,
  FUNC_ADC      = 0xa8,
  FUNC_CHR      = 0xa9,
  VAL_TICK      = 0xaa,
  VAL_INKEY     = 0xab,
} internal_code_e;

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
  ERROR_PRGMODE   = 8,
  ERROR_PGOVER    = 9,
  ERROR_PGEMPTY   = 10,
  ERROR_NOLOOP    = 11,
  ERROR_NOENDIF   = 12,
  ERROR_UXNEXT    = 13,
  ERROR_UXRETURN  = 14,
  ERROR_UXLOOP    = 15,
  ERROR_UXEXIT    = 16,
  ERROR_UXCONTINUE= 17,
  ERROR_CODE_MAX  = 17,
} error_code_t;

// Request Code
typedef enum {
  REQUEST_NOTHING = 0,
  REQUEST_GOTO    = 1,
  REQUEST_EXIT    = 2,
} request_code_t;

typedef struct {
  uint8_t   type;
  uint8_t   *returnPointer;
  int16_t   returnLineNumber;
  int16_t   *pvar;        /* counter variable */
  int16_t   limit;        /* limit value */
  int16_t   step;       /* step value */
} stack_t;

#define FORM_NONE   0x00
#define FORM_FLAG   0x01
#define FORM_PLUS   0x02
#define FORM_HEX    0x04
#define FORM_DEC    0x80
#define FORM_ZERO   0x10
#define FORM_LOWER  0x20
#define FORM_HEXU   FORM_HEX
#define FORM_HEXL   (FORM_HEX | FORM_LOWER)
#define FORM_FHEX   (FORM_HEX | FORM_FLAG)

#endif
