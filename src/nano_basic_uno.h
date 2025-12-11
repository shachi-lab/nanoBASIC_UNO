/*
 * NanoBASIC UNO - Public API for Arduino Sketch
 * ------------------------------------------------
 * Declares the minimal interface used by the
 * Arduino .ino file to start the NanoBASIC
 * interpreter on Arduino UNO.
 *
 * This header exposes:
 *   - basicInit()  : Initialize NanoBASIC engine
 *   - basicMain()  : Process input and execute BASIC
 *
 * Internal language definitions are in nano_basic_defs.h,
 * and hardware-dependent routines are implemented in
 * bios_uno.cpp/.h.
 *
 * This file is intentionally minimal — it provides only
 * the entry points required by the Arduino framework.
 *
 * GitHub: https://github.com/shachi-lab
 * Copyright (c) 2025 shachi-lab
 * License: MIT
 */

#ifndef __NANO_BASIC_UNO_H
#define __NANO_BASIC_UNO_H

void basicInit( void );
void basicMain( void );

#endif