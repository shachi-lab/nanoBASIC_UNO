/*
 * nanoBASIC UNO - Configuration / Build Settings
 * ------------------------------------------------
 * Configuration parameters and metadata for the
 * Arduino UNO (ATmega328P) port of nanoBASIC.
 *
 * This header defines:
 *   - Version string
 *   - Memory and buffer sizes
 *   - Platform-specific limits and adjustments
 *   - Optional compile-time feature switches
 *
 * These settings may vary by platform. Other ports
 * (ESP32, SAMD, etc.) should provide their own
 * nano_basic_<platform>_conf.h files.
 *
 * Core language definitions are located in
 * nano_basic_defs.h.
 *
 * GitHub: https://github.com/shachi-lab
 * Copyright (c) 2025 shachi-lab
 * License: MIT
 */

#ifndef __NANO_BASIC_UNO_CONF_H
#define __NANO_BASIC_UNO_CONF_H

#define NAME_STR            "nanoBASIC UNO"
#define VERSION_MAJOR       0
#define VERSION_MINOR       18

// NOTE: On Arduino UNO (2KB RAM), adjust the following parameters to fit your use case.

// --- Integer size for variables ---
#define NANOBASIC_INT32_EN  0    // Enable 32-bit integer support (uses more RAM)

// --- Memory / buffer settings ---
#define INPUT_BUFF_SIZE     80   // Input line buffer size (affects REPL history RAM usage)
#define CODE_BUFF_SIZE      80   // Buffer size for internal code generation
#define STACK_NUM           8    // Max nesting depth for FOR / WHILE / DO blocks
#define ARRAY_INDEX_NUM     64   // Maximum number of elements in @array
#define PROGRAM_AREA_SIZE   768  // BASIC program storage size in RAM
#define EXPR_DEPTH_MAX      16   // Maximum expression evaluation depth

// --- REPL features ---
#define REPL_EDIT_ENABLE    1    // Enable line editing in REPL (no extra RAM usage)
#define REPL_HISTORY_ENABLE 1    // Enable command history in REPL (up keys)

// --- Debug ---
#define CODE_DEBUG_ENABLE   0    // Enable internal code dump (for debugging)

// --- Startup behavior ---
#define AUTORUN_WAIT_TIME   3000 // Delay before AUTORUN at startup [ms]

// --- Output style ---
#define PRINT_HEX_STYLE     0    // 0:UPPER (FF), 1:lower (ff)
#define LIST_STYLE          0    // 0:UPPER, 1:lower, 2:Pascal

#define STR_(x)  #x
#define STR(x)   STR_(x)
#define VERSION_STR "Ver " STR(VERSION_MAJOR) "." STR(VERSION_MINOR)

#if NANOBASIC_INT32_EN
#define EXT_NAME_STR        "-32"
#else
#define EXT_NAME_STR        ""
#endif

#endif
