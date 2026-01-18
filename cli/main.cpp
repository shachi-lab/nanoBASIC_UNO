/*
 * nanoBASIC UNO - CLI entry point
 *
 * This main function is intentionally minimal.
 * All platform-specific behavior is handled in the BIOS layer,
 * and all language logic is handled by the nanoBASIC_UNO core.
 *
 * GitHub: https://github.com/shachi-lab
 * Copyright (c) 2025-2026 shachi-lab
 * License: MIT
 */

#include <setjmp.h>
#include "nano_basic_uno.h"

// Jump buffer used for system reset
jmp_buf reset_env;

int main()
{
  // Save execution context for bios_systemReset()
  setjmp(reset_env);

  // Initialize nanoBASIC core and BIOS
  basicInit();

  // Main loop
  // One call to basicMain() processes a single REPL step.
  while (1) {
    basicMain();
  }
}
