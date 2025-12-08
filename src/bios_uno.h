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
 *   - Digital I/O access
 *   - Analog input (ADC)
 *   - PWM output
 *   - Timing utilities
 *   - Character I/O for the console
 *   - Random number support
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

//UART
void bios_SerialInit( void );
void bios_SerialPutChar( uint8_t ch );
uint8_t bios_SerialGetChar( void );

// SystemTimer
void bios_systemTimerInit( void );
void bios_setWaitTick( int16_t val );
int16_t bios_getWaitTick( void );
int16_t bios_getSystemTick( void );

// Random
void bios_randomize( int16_t val );
int16_t bios_rand( int16_t val );

// GPIO, ADC, PWM
int16_t bios_writeGpio( int16_t pin, int16_t value );
int16_t bios_readGpio( int16_t pin );
int16_t bios_readAdc( int16_t ch );
int16_t bios_setPwm( int16_t pin, int16_t value );

#endif
