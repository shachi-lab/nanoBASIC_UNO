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

#ifndef	__NANO_BASIC_UNO_CONF_H
#define	__NANO_BASIC_UNO_CONF_H

#define	NAME_STR				    "NanoBASIC UNO"
#define	VERSION_STR			    "Ver 0.14"

#define	RAW_LINE_SIZE		    64
#define	CODE_LINE_SIZE	    64
#define STACK_NUM 			    8
#define	ARRAY_INDEX_NUM	    63

#define	CODE_DEBUG_ENABLE   0

#endif
