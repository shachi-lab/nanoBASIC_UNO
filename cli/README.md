# nanoBASIC UNO - CLI version

This is the command-line (CLI) version of **nanoBASIC UNO**.

It uses **exactly the same core source code** as the Arduino UNO version.
Only the BIOS implementation and `main` differ.

---

## Build

Put the following files in the same directory and build them with a standard C++ compiler:

- `main.cpp`
- `nano_basic_uno.cpp`
- `bios_uno_cli.cpp`
- `nano_basic_uno.h`
- `bios_uno.h`

### Example (Linux / g++)

```
g++ -std=gnu++17 main.cpp nano_basic_uno.cpp bios_uno_cli.cpp -o nanoBASIC_UNO
```

---

### Windows

Windows and Linux use **exactly the same source files**.  
Just build them with your preferred compiler (MSVC, clang, MinGW, etc).

---

## Run

Run the generated executable and start typing BASIC commands.

The CLI version provides a REPL environment equivalent to the Arduino version,
including `RUN`, `DELAY`, and `Ctrl-C` handling.

---

## Exit

Press `Ctrl-D` to exit the CLI.

---

## Hardware-related commands

Hardware-related commands are accepted in the CLI environment,  
but they behave as follows:

- `OUTP` and `PWM`  
  → No effect.
- `INP()` and `ADC()`  
  → Always return `0`.

---

## SAVE / LOAD

In the CLI version, `SAVE` and `LOAD` use a file named `eeprom.bin`
in the current directory to emulate EEPROM storage.

The file is created automatically when `SAVE` is executed.

---

## Resource limits

The CLI version preserves the same program size and memory limits
as the Arduino UNO version, for compatibility and behavior consistency.

---

## Notes

- No IDE or project files are required
- The nanoBASIC UNO core source code is OS-independent.
- OS-specific and platform differences are isolated in the BIOS layer

---
