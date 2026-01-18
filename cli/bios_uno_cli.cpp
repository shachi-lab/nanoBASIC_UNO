/*
 * BIOS layer for NanoBASIC UNO
 * --------------------------------------------
 * Platform-specific backend for Arduino UNO
 * (ATmega328P)
 *
 * This module implements hardware-dependent
 * functions required by the NanoBASIC core:
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
 *   NanoBASIC core is platform-agnostic,
 *   and this BIOS layer provides the minimal
 *   hardware services needed for execution.
 *
 * Porting to another MCU only requires
 * creating a different bios_<platform>.cpp
 * with the same function interface.
 *
 * GitHub: https://github.com/shachi-lab
 * Copyright (c) 2025-2026 shachi-lab
 * License: MIT
 */

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <setjmp.h>
#include <cstring>
#include "nano_basic_defs.h"
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
#include <vector>
#include <queue>
#include <string>

static void bios_consoleInit(void)
{
  HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
  static bool initialized = false;
  if (!initialized) {
    SetConsoleCP(65001);
    SetConsoleOutputCP(65001);
    DWORD mode;
    GetConsoleMode(hInput, &mode);
    // ENABLE_VIRTUAL_TERMINAL_INPUT を有効にすると、OSが自動でCSIを出すこともありますが、
    // 自前で制御するためにあえてオフ（Rawモード）に近い設定にします。
    SetConsoleMode(hInput, mode & ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT | ENABLE_PROCESSED_INPUT));
    initialized = true;
  }
}

//*************************************************
/**
 * UTF-8バイトを1バイトずつ受け取り、文字が完成したタイミングで表示する
 * 戻り値: 正常に処理されたら渡されたバイト、エラー時は EOF
 */
void bios_consolePutChar( char c )
{
  static std::vector<unsigned char> s_buffer;
  unsigned char ch = (unsigned char)c;

  // 1. ASCII (0xxxxxxx) の場合
  if ((ch & 0x80) == 0) {
    s_buffer.clear(); // 念のためバッファをクリア
    putchar(c);
    return;
  }

  // 2. UTF-8 マルチバイトの開始 (11xxxxxx)
  if ((ch & 0xC0) == 0xC0) {
    s_buffer.clear();
    s_buffer.push_back(ch);
  }
  // 3. UTF-8 マルチバイトの続き (10xxxxxx)
  else if ((ch & 0xC0) == 0x80) {
    s_buffer.push_back(ch);
  }

  // 文字の長さを判定 (UTF-8の仕様に基づく)
  size_t expected_len = 0;
  if (!s_buffer.empty()) {
    unsigned char first = s_buffer[0];
    if ((first & 0xE0) == 0xC0) expected_len = 2;      // 2バイト文字
    else if ((first & 0xF0) == 0xE0) expected_len = 3; // 3バイト文字(日本語の多く)
    else if ((first & 0xF8) == 0xF0) expected_len = 4; // 4バイト文字
  }

  // バイトが揃ったらまとめて出力
  if (s_buffer.size() >= expected_len && expected_len > 0) {
    for (unsigned char b : s_buffer) {
      fwrite(&b, 1, 1, stdout);
    }
    fflush(stdout); // 即座に反映
    s_buffer.clear();
  }
}


static std::queue<unsigned char> utf8_queue;

// UTF-16文字をUTF-8バイト列に変換してキューに積む
static void push_utf16_as_utf8(wchar_t wch)
{
  char utf8[4] = { 0 };
  int len = WideCharToMultiByte(CP_UTF8, 0, &wch, 1, utf8, sizeof(utf8), NULL, NULL);
  for (int i = 0; i < len; i++) {
    utf8_queue.push((unsigned char)utf8[i]);
  }
}

// シーケンスをキューに積むヘルパー
static void push_sequence(const char* seq) {
  while (*seq) {
    utf8_queue.push((unsigned char)*seq++);
  }
}

//*************************************************
int16_t bios_consoleGetChar(void)
{
  bios_polling();

  while (true)
  {
    // キューにある場合は、1バイトずつ返す
    if (!utf8_queue.empty()) {
      unsigned char c = utf8_queue.front();
      utf8_queue.pop();
      return (int16_t)c;
    }

    HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
    DWORD events;
    if (!GetNumberOfConsoleInputEvents(hInput, &events) || events == 0) {
      return -1;
    }

    INPUT_RECORD record;
    DWORD read;
    if (ReadConsoleInputW(hInput, &record, 1, &read)) {
      if (record.EventType == KEY_EVENT && record.Event.KeyEvent.bKeyDown) {

        WORD vk = record.Event.KeyEvent.wVirtualKeyCode;
        DWORD ctrl = record.Event.KeyEvent.dwControlKeyState;
        bool isCtrl = (ctrl & (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED));

        // --- 制御キーの処理 ---
        if (isCtrl && vk == 'C') return 0x03; // Ctrl+C
        if (isCtrl && vk == 'D') exit(0);     // Ctrl+D

        // --- CSIシーケンスに変換 ---
        switch (vk) {
        case VK_UP:     push_sequence("\x1b[A"); break;
        case VK_DOWN:   push_sequence("\x1b[B"); break;
        case VK_RIGHT:  push_sequence("\x1b[C"); break;
        case VK_LEFT:   push_sequence("\x1b[D"); break;
        case VK_HOME:   push_sequence("\x1b[H"); break;
        case VK_END:    push_sequence("\x1b[F"); break;
        case VK_DELETE: return ASCII_DEL;

        default:
          // 通常文字（全角含む）の処理
          wchar_t wch = record.Event.KeyEvent.uChar.UnicodeChar;
          if (wch != 0) {
            push_utf16_as_utf8(wch);
          }
          break;
        }
      }
    }
  }
}
#else

#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>

//*************************************************
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

//*************************************************
static void bios_consoleInit(void)
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

//*************************************************
void bios_consolePutChar(char ch)
{
  putchar(ch);
}

//*************************************************
int16_t bios_consoleGetChar(void)
{
  bios_polling();

  int ch = getchar();
  if (ch == EOF) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      errno = 0;
      return -1;
    }
    return -1;
  }
  if (ch == ASCII_EOT) {  // Ctrl-D
    exit(0);
  }
  return (int16_t)ch;
}
#endif

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
nb_int_t bios_getSystemTick( void )
{
  clock_t now = clock();
  uint32_t ms = (uint32_t)((now - start_clock) * 1000 / CLOCKS_PER_SEC);
  return (nb_int_t)ms;
}

//*************************************************
//    Random number
//*************************************************
//*************************************************
void bios_randomize( nb_int_t val )
{
  if (val == 0) {
      srand((unsigned int)time(NULL));
  } else {
      srand((unsigned int)val);
  }
}

//*************************************************
nb_int_t bios_rand( nb_int_t val )
{
  if (val <= 0) return 0;
  return (nb_int_t)(rand() % val);
}

//*************************************************
//    GPIO, ADC, PWM
//*************************************************
//*************************************************
int8_t bios_writeGpio( nb_int_t pin, nb_int_t value )
{
  if(pin < 0 || pin > 19) {
    return -1;
  }
  return 0;
}

//*************************************************
int8_t bios_readGpio( nb_int_t pin )
{
  if(pin < 0 || pin > 19) {
    return -1;
  }
  return 0;
}

//*************************************************
int16_t bios_readAdc( nb_int_t ch )
{
  // ch : 0..5 (A0..A5)
  if(ch < 0 || ch > 5) {
    return -1;
  }
  return 0;
}

//*************************************************
int8_t bios_setPwm( nb_int_t pin, nb_int_t value )
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
void bios_eepReadBlock( uint16_t addr, uint8_t* buf, uint16_t len )
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
