/*
 * BIOS layer for nanoBASIC UNO
 * --------------------------------------------
 * Platform-specific backend for Arduino UNO
 * (ATmega328P)
 *
 * This module implements hardware-dependent
 * functions required by the nanoBASIC core:
 *
 *   - Character input/output
 *   - GPIO (digital input/output)
 *   - PWM output
 *   - ADC (analog input)
 *   - Timing utilities (millis / delay)
 *   - Random number support
 *   - System reset
 *   - EEPROM access
 *
 * The design follows a simple abstraction:
 *   nanoBASIC core is platform-agnostic,
 *   and this BIOS layer provides the minimal
 *   hardware services needed for execution.
 *
 * Porting to another MCU only requires
 * creating a different bios_<platform>.cpp
 * with the same function interface.
 *
 * GitHub: https://github.com/shachi-lab
 * Copyright (c) 2025 shachi-lab
 * License: MIT
 */

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <setjmp.h>
#include <cstring>
#include "bios_uno.h"

extern jmp_buf reset_env;

static void bios_consoleInit( void );
static void bios_systemTickInit( void );
static void bios_polling( void );

//*************************************************
void bios_init(void)
{
  bios_consoleInit();
  bios_systemTickInit();
  bios_randomize( 0 );
}

//*************************************************
//    Character input/output
//*************************************************
#ifdef _WIN32

#include <conio.h>
#include <windows.h>

static volatile int ctrl_c_flag = 0;

static BOOL WINAPI consoleHandler(DWORD type)
{
  if (type == CTRL_C_EVENT) {
    ctrl_c_flag = 1;
    return TRUE;
  }
  return FALSE;
}

static void bios_consoleInit( void )
{
  SetConsoleCtrlHandler(consoleHandler, TRUE);
}

#else

#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>

static struct termios original_termios;
static int original_flags = -1;

static void bios_consoleRestore(void)
{
  if (original_flags != -1) {
    fcntl(STDIN_FILENO, F_SETFL, original_flags);
  }
  tcsetattr(STDIN_FILENO, TCSANOW, &original_termios);
}

static void cleanup_handler(void)
{
  bios_consoleRestore();
}

static void sigint_handler(int sig)
{
	 bios_consoleRestore();
  _exit(0);
}

static void bios_consoleInit( void )
{
  if (tcgetattr(STDIN_FILENO, &original_termios) != 0) {
     perror("tcgetattr failed");
     return;
   }

  struct termios newt = original_termios;
  newt.c_lflag &= ~(ICANON | ECHO | ISIG);
  newt.c_iflag &= ~(ICRNL);
  if (tcsetattr(STDIN_FILENO, TCSANOW, &newt) != 0) {
    perror("tcsetattr failed");
    return;
  }
  if ((original_flags = fcntl(STDIN_FILENO, F_GETFL, 0)) == -1) {
    perror("fcntl F_GETFL failed");
    return;
  }
  if (fcntl(STDIN_FILENO, F_SETFL, original_flags | O_NONBLOCK) == -1) {
    perror("fcntl F_SETFL O_NONBLOCK failed");
    return;
  }
  signal(SIGINT, sigint_handler);
  atexit(cleanup_handler);
}

#endif

//*************************************************
void bios_consolePutChar( char ch )
{
  putchar( ch );
}

//*************************************************
int16_t bios_consoleGetChar( void )
{
    bios_polling();
#ifdef _WIN32
  if (ctrl_c_flag) {
    ctrl_c_flag = 0;
    return 0x03;
  }

  if (!_kbhit()) {
    return -1;
  }
  int ch = _getch();

#else
  int ch = getchar();

  if (ch == EOF) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      errno = 0;
      return -1;
    }
    return -1;
  }
#endif

  if( ch == 0x04 ) {  // Ctrl-D
    exit( 0 );
  }
  return (int16_t)ch;
}

//*************************************************
//    Timing utilities
//*************************************************
static clock_t start_clock;
//*************************************************
static void bios_systemTickInit( void )
{
  start_clock = clock();
}

//*************************************************
int16_t bios_getSystemTick( void )
{
  clock_t now = clock();
  uint32_t ms = (uint32_t)((now - start_clock) * 1000 / CLOCKS_PER_SEC);
  return (int16_t)(ms & 0xFFFF);
}

//*************************************************
//    Random number
//*************************************************
//*************************************************
void bios_randomize( int16_t val )
{
  if (val == 0) {
      srand((unsigned int)time(NULL));
  } else {
      srand((unsigned int)val);
  }
}

//*************************************************
int16_t bios_rand( int16_t val )
{
  if (val <= 0) return 0;
  return (int16_t)(rand() % val);
}

//*************************************************
//    GPIO, ADC, PWM
//*************************************************
//*************************************************
int16_t bios_writeGpio( int16_t pin, int16_t value )
{
  if(pin < 0 || pin > 19) {
    return -1;
  }
  return 0;
}

//*************************************************
int16_t bios_readGpio( int16_t pin )
{
  if(pin < 0 || pin > 19) {
    return -1;
  }
  return 0;
}

//*************************************************
int16_t bios_readAdc( int16_t ch )
{
  // ch : 0〜5（A0〜A5）
  if(ch < 0 || ch > 5) {
    return -1;
  }
  return 0;
}

//*************************************************
int16_t bios_setPwm( int16_t pin, int16_t value )
{
  if( pin != 3 && pin != 5 && pin != 6 && pin != 9 && pin != 10 && pin != 11 ) {
    return -1;
  }
  return 0;
}

//*************************************************
//    Syetem reset
//*************************************************
#include <setjmp.h>
extern jmp_buf reset_env;
//*************************************************
void bios_systemReset( void )
{
  longjmp(reset_env, 1);
}

//*************************************************
//    EEPROM
//*************************************************
#define EEPROM_SIZE 1024
#define EEPROM_FILE "eeprom.bin"
static FILE* eep;

//*************************************************
static FILE* bios_eepOpenRW( void )
{
  if (!eep) {
    eep = fopen(EEPROM_FILE, "rb");
    if (!eep) {
      return eep;
    }
    eep = freopen(EEPROM_FILE, "r+b", eep);
  }
  return eep;
}

//*************************************************
void bios_eepEraseBlock( uint16_t addr, uint16_t len )
{
  if (addr >= EEPROM_SIZE) return;
  if (addr + len > EEPROM_SIZE) {
    len = EEPROM_SIZE - addr;
  }

  eep = bios_eepOpenRW();
  if (!eep) return;

  uint8_t ff = 0xFF;

  fseek(eep, addr, SEEK_SET);
  for (uint16_t i = 0; i < len; i++) {
    fwrite(&ff, 1, 1, eep);
  }
  fflush(eep);
}

//*************************************************
void bios_eepWriteBlock( uint16_t addr, const uint8_t* buf, uint16_t len )
{
  if (!buf) return;
  if (addr >= EEPROM_SIZE) return;
  if (addr + len > EEPROM_SIZE) {
    len = EEPROM_SIZE - addr;
  }

  eep = bios_eepOpenRW();
  if (!eep) {
    eep = fopen(EEPROM_FILE, "w+b");
  }
  if (!eep) return;

  fseek(eep, addr, SEEK_SET);
  fwrite(buf, 1, len, eep);
  fflush(eep);
}

//*************************************************
void bios_eepReadBlock(uint16_t addr, uint8_t* buf, uint16_t len)
{
  if (!buf) return;
  if (addr >= EEPROM_SIZE) return;
  if (addr + len > EEPROM_SIZE) {
    len = EEPROM_SIZE - addr;
  }

  eep = bios_eepOpenRW();
  if (!eep) {
    memset(buf, 0xFF, len);
  } else {
    fseek(eep, addr, SEEK_SET);
    size_t r = fread(buf, 1, len, eep);
    if (r < len) {
      memset(buf + r, 0xFF, len - r);
    }
  }
}

/**
 * @brief Periodic polling hook for platform-specific background tasks.
 *
 * This function is called regularly from the REPL input loop
 * and from the interpreter execution loop.
 *
 * Use this function to poll platform-dependent events
 * (e.g. communication, timers, background status checks).
 *
 * Do NOT put language logic or execution control here.
 */
static void bios_polling( void )
{
  // reserved for future use
}
