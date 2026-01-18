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
 * Copyright (c) 2025-2026 shachi-lab
 * License: MIT
 */

#include <Arduino.h>
#include "nano_basic_defs.h"
#include "bios_uno.h"

#define BIOS_SELIAR_BAUDRATE        115200

static void bios_consoleInit(void);
static void bios_systemTickInit(void);
static void bios_eepInit(void);
static void bios_polling(void);

//*************************************************
void bios_init(void)
{
  bios_consoleInit();
  bios_systemTickInit();
  bios_randomize(0);
  bios_eepInit();
}

//*************************************************
//    Character input/output
//*************************************************
//*************************************************
static void bios_consoleInit(void)
{
  Serial.begin(BIOS_SELIAR_BAUDRATE);
}

//*************************************************
void bios_consolePutChar(char ch)
{
  Serial.write(ch);
}

//*************************************************
int16_t bios_consoleGetChar(void)
{
    bios_polling();
  return (int16_t)Serial.read();
}

//*************************************************
//    Timing utilities
//*************************************************
//*************************************************
static void bios_systemTickInit(void)
{
}

//*************************************************
nb_int_t bios_getSystemTick(void)
{
  return (nb_int_t)millis();
}

//*************************************************
//    Random number
//*************************************************
//*************************************************
void bios_randomize(nb_int_t val)
{
  if(val == 0) {
    randomSeed(analogRead(A0) ^ micros());
  } else {
    randomSeed(val);
  }
}

//*************************************************
nb_int_t bios_rand(nb_int_t val)
{
  return ((nb_int_t)random()) % val;
}

//*************************************************
//    GPIO, ADC, PWM
//*************************************************
//*************************************************
int8_t bios_writeGpio(nb_int_t pin, nb_int_t value)
{
  if(pin < 0 || pin > 19) {
    return -1;
  }
  // D0〜D7 : PORTD
  if (pin <= 7){
      DDRD |= (1 << pin);
      if(value) PORTD |= (1 << pin);
      else      PORTD &= ~(1 << pin);
  }
  // D8〜D13 : PORTB
  else if(pin <= 13){
      int b = pin - 8;
      DDRB |= (1 << b);
      if(value) PORTB |= (1 << b);
      else      PORTB &= ~(1 << b);
  }
  // A0〜A5 (14〜19) : PORTC
  else {
      int c = pin - 14;
      DDRC |= (1 << c);
      if(value) PORTC |= (1 << c);
      else      PORTC &= ~(1 << c);
  }
  return 0;
}

//*************************************************
int8_t bios_readGpio(nb_int_t pin)
{
  if (pin < 0 || pin > 19) {
    return -1;
  }
  if (pin >= 14 && pin <= 19) return -1; 

  if (pin <= 7) {
      return (PIND >> pin) & 1;
  } else if(pin <= 13) {
      return (PINB >> (pin - 8)) & 1;
  } else if(pin <= 19) { 
      return (PINC >> (pin - 14)) & 1;
  }
  return 0;
}

//*************************************************
int16_t bios_readAdc(nb_int_t ch)
{
  // ch : 0〜5（A0〜A5）
  if (ch < 0 || ch > 5) {
    return -1;
  }
  ADMUX = (1 << REFS0) | (ch & 0x07);
  ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
  ADCSRA |= (1 << ADSC);
  while (ADCSRA & (1 << ADSC));
  ADCSRA |= (1 << ADSC);
  while (ADCSRA & (1 << ADSC));
  return ADC;
}

//*************************************************
int8_t bios_setPwm(nb_int_t pin, nb_int_t value)
{
  if (pin != 3 && pin != 5 && pin != 6 && pin != 9 && pin != 10 && pin != 11) {
    return -1;
  }
  if (value < 0) value = 0;
  if (value > 255) value = 255;
  analogWrite(pin, value);
  return 0;
}

//*************************************************
//    Syetem reset
//*************************************************
#include <avr/wdt.h>
//*************************************************
void bios_systemReset(void)
{
  wdt_enable(WDTO_15MS);
  while (1) {}
}

//*************************************************
//    EEPROM
//*************************************************
#include <EEPROM.h>
//*************************************************
static void bios_eepInit(void)
{
}

//*************************************************
void bios_eepEraseBlock(uint16_t addr, uint16_t len)
{
  for (uint16_t i = 0; i < len; i++) {
    EEPROM.update(addr + i, 0xff);
  }
}

//*************************************************
void bios_eepWriteBlock(uint16_t addr, const uint8_t* buf, uint16_t len)
{
  for (uint16_t i = 0; i < len; i++) {
    EEPROM.update(addr + i, buf[i]);
  }
}

//*************************************************
void bios_eepReadBlock(uint16_t addr, uint8_t* buf, uint16_t len)
{
  for (uint16_t i = 0; i < len; i++) {
    buf[i] = EEPROM.read(addr + i);
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
static void bios_polling(void)
{
  // reserved for future use
}
