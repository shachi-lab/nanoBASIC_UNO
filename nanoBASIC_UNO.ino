// NanoBASIC UNO
// Entry point for Arduino sketch
// --------------------------------
// Target Board: Arduino UNO (ATmega328P)
//
// The BASIC interpreter core is implemented in nano_basic_uno.cpp.
// This file only provides setup() and loop() for the Arduino framework.
//
// GitHub: https://github.com/shachi-lab
// Copyright (c) 2025 shachi-lab
// License: MIT

#include "src/nano_basic_uno.h"

// the setup function runs once when you press reset or power the board
void setup() {
  basicInit();
}

// the loop function runs over and over again forever
void loop() {
	basicMain();
}
