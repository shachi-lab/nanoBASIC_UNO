/*
 * NanoBASIC UNO - Configuration / Build Settings
 * ------------------------------------------------
 * Configuration parameters and metadata for the
 * Arduino UNO (ATmega328P) port of NanoBASIC.
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
#define VERSION_STR         "Ver 0.16"

#define VERSION_MAJOR       0
#define VERSION_MINOR       16

#define RAW_LINE_SIZE       80
#define CODE_LINE_SIZE      80
#define STACK_NUM           8
#define ARRAY_INDEX_NUM     64
#define PROGRAM_AREA_SIZE   768
#define AUTORUN_WAIT_TIME   3000
#define EXPR_DEPTH_MAX      20

#define CODE_DEBUG_ENABLE   0

typedef struct {
    uint8_t magic1;       // 'n'
    uint8_t magic2;       // 'B'
    uint8_t verMajor;
    uint8_t verMinor;
    int16_t progLength;
    uint8_t autoRun;
    uint8_t reserved;
} EEP_Header_t;

#define EEP_MAGIC_1         'n'
#define EEP_MAGIC_2         'B'
#define EEP_HEADER_ADDR      0
#define EEP_HEADER_SIZE      sizeof(EEP_Header_t)
#define EEP_PROGRAM_ADDR     (EEP_HEADER_ADDR + EEP_HEADER_SIZE)

#endif
