/*
 * BIOS interface for NanoBASIC UNO
 * --------------------------------------------
 * Declares the hardware-dependent functions
 * implemented in bios_uno.cpp.
 *
 * The NanoBASIC core is platform-independent.
 * Each port (UNO, Mega, ESP32, etc.) provides
 * its own BIOS layer by implementing the same
 * function prototypes defined here.
 *
 * This header defines:
 *   - Character I/O for the console
 *   - GPIO (digital input/output)
 *   - Analog input (ADC)
 *   - PWM output
 *   - Timing utilities
 *   - Random number support
 *   - System reset
 *   - EEPROM access
 *
 * Porting NanoBASIC to another MCU involves
 * creating a bios_<platform>.cpp/.h pair
 * that matches this interface.
 *
 * GitHub: https://github.com/shachi-lab
 * Copyright (c) 2025 shachi-lab
 * License: MIT
 */

#ifndef __BIOS_UNO_H
#define __BIOS_UNO_H

#define BIOS_SELIAR_BAUDRATE  115200

// Initialize
void bios_init(void);

// Character I/O
void bios_serialInit( void );
void bios_serialPutChar( char ch );
int16_t bios_serialGetChar( void );

// Timing utilities
void bios_systemTimerInit( void );
void bios_setWaitTick( int16_t val );
int16_t bios_getWaitTick( void );
int16_t bios_getSystemTick( void );

// Random number
void bios_randomize( int16_t val );
int16_t bios_rand( int16_t val );

// GPIO, ADC, PWM
int16_t bios_writeGpio( int16_t pin, int16_t value );
int16_t bios_readGpio( int16_t pin );
int16_t bios_readAdc( int16_t ch );
int16_t bios_setPwm( int16_t pin, int16_t value );

// System reset
void bios_systemReset( void );

// EEPROM
void eepEraseBlock( uint16_t addr, uint16_t len );
void eepWriteBlock( uint16_t addr, const uint8_t* buf, uint16_t len );
void eepReadBlock( uint16_t addr, uint8_t* buf, uint16_t len );

#endif
