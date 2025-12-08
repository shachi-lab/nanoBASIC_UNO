/*
 * BIOS layer for NanoBASIC UNO
 * --------------------------------------------
 * Platform-specific backend for Arduino UNO
 * (ATmega328P)
 *
 * This module implements hardware-dependent
 * functions required by the NanoBASIC core:
 *
 *   - GPIO (digital input/output)
 *   - PWM output
 *   - ADC (analog input)
 *   - Timing utilities (millis / delay)
 *   - Random number support
 *   - Character input/output
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
 * Copyright (c) 2025 shachi-lab
 * License: MIT
 */

#include <Arduino.h>
#include "bios_uno.h"

//*************************************************
void bios_init(void)
{
	bios_SerialInit();
	bios_systemTimerInit();
	bios_randomize( 0 );
}

//*************************************************
//    UART
//*************************************************

//*************************************************
void bios_SerialInit( void )
{
	Serial.begin( BIOS_SELIAR_BAUDRATE );
}

//*************************************************
void bios_SerialPutChar( uint8_t ch )
{
	Serial.write( ch );
}

//*************************************************
uint8_t bios_SerialGetChar( void )
{
	if ( Serial.available() ) {
		return Serial.read();
	}
	return 0;
}

//*************************************************
//    SystemTimer
//*************************************************
static uint32_t systemTick;
static uint32_t waitStart;
static int16_t waitTick;
//*************************************************
void bios_systemTimerInit( void )
{
	waitStart = 0;
	waitTick = 0;
}

//*************************************************
int16_t bios_getSystemTick( void )
{
	systemTick = millis();
	return (uint16_t)systemTick;
}

//*************************************************
void bios_setWaitTick( int16_t val )
{
	waitTick = val;
	waitStart = millis();
}

//*************************************************
int16_t bios_getWaitTick( void )
{
	uint16_t elps = (uint16_t)(millis() - waitStart); 
	if( elps >= waitTick )	return 0;
	return waitTick - elps;
}

//*************************************************
//    RANDOM
//*************************************************
//*************************************************
void bios_randomize( int16_t val )
{
	if( val == 0 ) {
		randomSeed( analogRead(A0) ^ micros() );
	} else {
		randomSeed( val );
	}
}

//*************************************************
int16_t bios_rand( int16_t val )
{
	return ((int16_t)random()) % val;
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
	if(pin <= 7){
			DDRD |= (1 << pin);
			if(value) PORTD |= (1 << pin);
			else      PORTD &= ~(1 << pin);
	}
	else if(pin <= 13){
			int b = pin - 8;
			DDRB |= (1 << b);
			if(value) PORTB |= (1 << b);
			else      PORTB &= ~(1 << b);
	}
	return 0;
}

//*************************************************
int16_t bios_readGpio( int16_t pin )
{
	if(pin < 0 || pin > 19) {
		return -1;
	}
	if(pin <= 7) {
			return (PIND >> pin) & 1;
	} else if(pin <= 13) {
			return (PINB >> (pin - 8)) & 1;
	} else if(pin <= 19) { 
			return (PINC >> (pin - 14)) & 1;
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
	// 1) チャンネル設定（REF=AVcc, ADLAR=0）
	ADMUX = (1 << REFS0) | (ch & 0x07);
	// 2) ADC有効化 + 分周128（16MHz → 125kHz）
	ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
	// 3) 1回だけ廃棄コンバージョン（精度向上）
	ADCSRA |= (1 << ADSC);
	while (ADCSRA & (1 << ADSC));
	// 4) 本番の1回
	ADCSRA |= (1 << ADSC);
	while (ADCSRA & (1 << ADSC));
	// 5) 10bit値を返す
	return ADC;
}

//*************************************************
int16_t bios_setPwm( int16_t pin, int16_t value )
{
	if( pin != 3 && pin != 5 && pin != 6 && pin != 9 &&	pin != 10 && pin != 11 ) {
		return -1;
	}
  if(value < 0) value = 0;
  if(value > 255) value = 255;
  analogWrite(pin, value);
	return 0;
}