# 📘**nanoBASIC UNO Reference Manual**

English edition (Translated from the Japanese manual)
for Version 0.18

## Overview
nanoBASIC UNO is a BASIC interpreter running on Arduino UNO (ATmega328P).  
Each command entered from the serial terminal is translated  
into an intermediate bytecode format and executed by the interpreter.

* Line numbers are treated as labels (not related to execution order)
* Each line is translated into bytecode, limited to 63 bytes
* Hand-written expression parser without recursion
* Program area resides in RAM and is directly written using the PROG command
* Programs can be saved in EEPROM
* REPL / Run operating mode

---

## Operation Modes
nanoBASIC has two operation modes:

* **REPL Mode:**  
  Commands typed from the terminal are executed one line at a time.

* **Run Mode:**  
  Executes the program stored in the program area sequentially.  

After startup, the interpreter starts in REPL mode.

---

## REPL Mode

Commands are entered via the serial port.  
Statements are **case-insensitive**.  
However, the contents of **comments and string literals are preserved exactly as entered**.

Basic line-editing is available during input.  
The following keys can be used for editing:

* Left / Right arrow
* DEL, BS
* HOME, END

Command history can be recalled using the **Up Arrow** key.

In this mode, each line entered is handled as follows:

* **The line is executed immediately, but is not stored in the program area.**
* **Variable values are preserved.**

This mode is suitable for:

* Simple calculations
* Quick testing
* Trial-and-error experimentation

---

## Numbers
By default, numeric values in nanoBASIC are 16-bit signed integers  
(-32768 to 32767).

At build time, numeric values can be configured to use  
32-bit signed integers instead.

There is no floating-point support (such as float or double).  
Arithmetic overflow is not checked or handled.

Numbers are interpreted as decimal by default.  
If a number is prefixed with 0x, it is interpreted as hexadecimal.
```
A=10
B=0xBEEF
```

---

## Strings

Text enclosed in double quotation marks (`"`) is treated as a string.  

Strings can be used **only with the `PRINT` command (or `?`)**.  
They cannot be assigned to variables or used within expressions.

Strings support **C language–compatible escape sequences**.

### Supported Escape Sequences

#### Control Characters and Symbols (C-compatible)

| Escape | Meaning               |
| ------ | --------------------- |
| `\a`   | Bell                  |
| `\b`   | Backspace             |
| `\f`   | Form feed             |
| `\n`   | Newline               |
| `\r`   | Carriage return       |
| `\t`   | Tab                   |
| `\v`   | Vertical tab          |
| `\\`   | Backslash             |
| `\'`   | Single quote          |
| `\"`   | Double quotation mark |
| `\?`   | Question mark         |

#### Numeric Escape Sequences

* **`\ooo`**
  Specifies a character code in **octal** (0–377)

* **`\xhh`**
  Specifies a character code in **hexadecimal** (00–FF)

### Notes

* Numeric escape sequences are expanded as **a single character**
* Character codes are interpreted as **8-bit values (0–255)**
* UTF-8 characters may be mixed with escape sequences,  
  but care should be taken to maintain readability

---

## Variables

* **Scalar variables**  
  There are **26 variables**, named `A` through `Z`.  
  Each variable is a **signed 16-bit integer** by default.  

* **Array variable**  
  A single **one-dimensional array** is available, accessed using `@[index]`.  
  The array elements are **signed 16-bit integers**,  
  and the valid index range is **0 to 63**.

The **integer bit width** and **array size** can be changed  
using build-time configuration options.

### Notes

* All variables are **global**
* **Local variables are not supported**
* **String variables are not supported**

---

## Operators

### Binary Operators
| Operator | Meaning |
|:---:|---|
| + | Addition |
| - | Subtraction |
| * | Multiplication |
| / | Division |
| % | Modulo |

### Unary Operators
| Operator | Meaning |
|:---:|---|
| - | Negation |
| ~ | Bitwise NOT |
| ! | Logical NOT |

### Logical Operators
| Operator | Meaning |
|:---:|---|
| && | Logical AND |
| \|\| | Logical OR |

### Bitwise Operators
| Operator | Meaning |
|:---:|---|
| & | Bitwise AND |
| \| | Bitwise OR |
| ^ | Bitwise XOR |

### Shift Operators
| Operator | Meaning |
|:---:|---|
| << | Left shift |
| >> | Right shift |

### Comparison Operators
| Operator | Meaning |
|:---:|---|
| =, == | Equal |
| <>, != | Not equal |
| < | Less than |
| <= | Less than or equal |
| > | Greater than |
| >= | Greater than or equal |

### Parentheses
| Operator | Meaning |
|:---:|---|
| (expression) | Evaluated with highest priority |

---

## Expressions in Commands
An "expression" is anything that uses numbers and operators.  
Expressions are written using numbers, variables, and operators.  
Expressions can be used as arguments to commands or in assignments.   
Calculations are performed according to standard operator precedence.

---

## Assignment Statement
```
variable = expression
```
Assigns the result of the expression to the variable.  
There is no `LET` statement.  
```
A=10
```

---

## Compound Assignment
```
variable operator = expression
```
Operators supported: `+ - * / % & | ^ << >>`

Example:
```
A += 10 + B
A <<= 1
```

---

## Increment / Decrement
Appending `++` or `--` increments or decrements a variable by 1.

*Cannot be used within expressions; must appear alone.*

Examples:
```
A++
@[10]--
```

---

## Statements
A statement consists of one or more commands or assignments.  
Multiple statements may be connected using `:`.  
A single line must be **63 characters or fewer**, or **63 bytes after bytecode translation**.

---

## Label Numbers
Labels are numeric identifiers placed at the beginning of a line for use with GOTO and GOSUB.

* Valid range: 1–32768  
* Duplicate labels are allowed; the earliest one in the program is used  
* Labels are optional  
* Label numbers do not affect execution order

---

## Line Numbers
Line numbers represent the physical order of program lines stored in RAM.  
Line numbers and labels are different concepts.  
Line numbers cannot be used as GOTO/GOSUB targets.

---

## Interrupting Execution
Sending **Ctrl-C (0x03)** from the terminal interrupts program execution.  
Execution may be resumed from the interruption point using `RESUME`.

Execution cannot be resumed after an error stop.

---

## Nesting Rules
The nesting stack for `GOSUB/RETURN`, `FOR/NEXT`, `DO/LOOP`, and `WHILE/LOOP` is shared.  
Maximum nesting depth is **8 levels**.  
Exceeding this results in a **stack overflow error**.

---

## Comments

Any text following a single quote (`'`) is treated as a comment  
until the end of the line.  
Comments do **not affect program execution**.

The handling of a comment depends on  
the **number of leading single quotes**, as described below.

* **`'` (one single quote)**  
  → The comment is **stored in the program area (PROG)**  
  → The comment **appears in `LIST` output**

* **`''` (two single quotes)**  
  → The comment is **not stored in the program area**  
  → The comment **does not appear in `LIST` output**

```
' This comment is stored (consumes memory)
'' This comment is not stored (does not consume memory)
```

This design allows both:

* Maintaining **readability** during editing
* **Reducing program memory usage** on memory-constrained systems

---

## Commands

### PRINT
```
PRINT expression
? expression
```

* `?` may be used as a shorthand for PRINT.  
* Multiple expressions may be separated with commas (tab) or semicolons (no newline).  
* Ending a PRINT statement with `;` suppresses the newline.  
* Strings, `CHR()`, and formatting functions may be used only inside PRINT.
* String literals support C-compatible escape sequences.  
  For a complete list, refer to [Supported Escape Sequences].

#### Numeric Output and Format Specification

The `PRINT` command allows you to specify numeric output formats as shown below.

##### Decimal Output

* `DEC(expression)`  
  → Equivalent to specifying a width of 0, or omitting the format specification.
* `DEC(expression, width)`  
  → Displays the value with the specified width.

##### Hexadecimal Output

* `HEX(expression)`  
  → Displays the value in hexadecimal (same as specifying width = 0).
* `HEX(expression, width)`  
  → Displays the value in hexadecimal with the specified width.

##### Width Specification Rules

* **Positive width**  
  → Left-padded with **spaces** (leading spaces).
* **Negative width**  
  → Left-padded with **zeros** (leading zeros).

```
? DEC(1234) "," DEC(1234,-5) "," DEC(5678,5)
1234,01234, 5678
? HEX(1234) "," HEX(1234,-5) "," HEX(5678,5)
4D2,004D2, 162E
```

##### Signed Decimal Numbers

* If the value is negative and **fits within the specified width including the sign (`-`)**,  
  it is displayed using the specified width.  
* If it does **not fit**, the output width is automatically extended to accommodate the sign.

```
A=-1234
? DEC(A,3) "," DEC(A,6) "," DEC(A,-6)
-234, -1234,-01234
```

##### Notes on Hexadecimal Output

* In hexadecimal output, **the sign is not displayed even if the value is negative**.  
  The internal two’s-complement representation is printed as-is.

```
? HEX(-1)
FFFF
```

##### Specifying Fractional Digits

* If the **hundreds digit of the width value is non-zero**,  
  a **decimal point is inserted at that position**.  
* If the value including the decimal point **fits within the specified width**,  
  it is displayed using that width.  
* If it does **not fit**, the width is automatically extended by one digit to include the decimal point.  

```
A=1234
? DEC(A,200) "," DEC(A,205) "," DEC(-A,-306) "," DEC(A,500)
12.34, 12.34,-01.234,0.01234
```

### INPUT
```
INPUT variable
```

* Waits for user input and stores the value in the variable.  
* If input begins with `0x`, it is parsed as hexadecimal.  

### GOTO
```
GOTO expression
```
Jumps to the label indicated by the expression.  
The expression must start with a numeric literal.

### GOSUB
```
GOSUB expression
RETURN
```
Jumps to a subroutine with the label number specified by the expression.  
A RETURN command within the subroutine will return to the statement following this command.  
The expression must start with a numeric literal.

### RETURN
```
RETURN
```
Return from the subroutine.

### IF / THEN / ELSEIF / ELSE / ENDIF
```
IF expr THEN statements
ELSEIF expr THEN statements
ELSE statements
ENDIF
```
Transfers control to an instruction depending on a condition.  
If the result of the expression is **0**, it is evaluated as **false**;  
any **non-zero** result is evaluated as **true**.  

* `ELSEIF` and `ELSE` are optional
* Multiple `ELSEIF–THEN` combinations can be written
* If a numeric expression is written immediately after `THEN` or `ELSE`,  
  it behaves the same as `GOTO`

**Notes:**

* If `ELSE` or `ELSEIF` is executed when no `IF` has been executed,  
  processing is skipped until the corresponding `ENDIF`.
* If `THEN` or `ENDIF` is executed when no `IF` has been executed,  
  nothing is done and execution continues with the next instruction.

### FOR / TO / STEP / NEXT
```
FOR var = expr1 TO expr2 STEP expr3
...
NEXT
```
After assigning expr1 to the variable, it loops between FOR and NEXT until the variable becomes expr2.  
Each time the program reaches NEXT, the value of expression 3 is added to the variable.  
STEP expr3 is optional, and if it is omitted, 1 will be specified.

---

### DO / LOOP
```
DO
...
LOOP
```
Repeat DO~LOOP

### DO / LOOP WHILE
```
DO
...
LOOP WHILE expr
```
When the result of the expr is "true", DO ~ LOOP WHILE will be repeated.  
After executing DO ~ LOOP, the expr is evaluated and if it is true, it will be repeated.

### WHILE / LOOP
```
WHILE expr
...
LOOP
```
If the result of the expr is "true", WHILE ~ LOOP will be repeated.  
After evaluating the expr, if it is true, WHILE ~ LOOP will be repeated.

### EXIT
```
EXIT
```
Exits the nearest FOR/NEXT or DO/WHILE/LOOP block.  
Execution continues after the corresponding NEXT or LOOP.

### CONTINUE
```
CONTINUE
```
CONTINUE skips the remaining statements of the currently executing loop  
(FOR–NEXT or DO / WHILE–LOOP) and proceeds to the next iteration of the same loop.  

The behavior depends on the type of loop:  
- DO / WHILE / LOOP loops  
  → Control jumps back to the beginning of the loop  
  (the condition check point).
- FOR / NEXT loops  
  → Control jumps to the corresponding NEXT statement,  
  updates the loop variable, and then starts the next iteration.

### END
```
END
```
Terminates program execution and returns to REPL mode.  
When executed in REPL mode, it initializes the nesting stack and the suspended state.

### STOP
```
STOP
```
Suspends program execution and returns to REPL mode.
It can be resumed with RESUME.

### RESUME
```
RESUME
```
Resumes processing that was suspended by STOP or by receiving Ctrl-C from the serial port.  
A suspension due to an error cannot be resumed.

### DELAY
```
DELAY expression
```
Waits for the specified number of milliseconds.

### PAUSE
```
PAUSE
```
Waits until one character is received from the serial port.

### DATA
```
DATA expr1, expr2, ...
```
Data values for READ.   
Expressions may be used.  
The DATA statement is evaluated independently of IF conditions and loop execution.

### READ
```
READ variable
```
READ retrieves values from DATA statements in order.  

### RESTORE
```
RESTORE
```
RESTORE resets the read position to the start.

### NEW
```
NEW
```
Clears the program area.

### PROG
```
PROG
```
Writes a program into the program area.

After the prompt `>`, enter the program **one line at a time**.  
Entering a line that starts with `#` ends the program input mode.  

Indentation and unnecessary spaces are ignored,  
and all characters **except comments and string literals** are converted to uppercase.  

If a syntax error or other error occurs during conversion to bytecode,  
that line is **not written to the program area** and an error message is displayed.  
However, program input mode is **not terminated**, and input continues with the next line.

Empty lines and lines containing only comments starting with two single quotes (`''`)  
are skipped.

If you want to intentionally keep a "blank line" in your program,  
use a line that contains only ":" (statement separator).

##### Notes

* Each input line is converted to bytecode immediately.  
  When entering multiple lines consecutively,  
  insert a delay of approximately **100 ms** between lines  
  to allow time for conversion.

* The program area resides in **RAM**,  
  and its contents are lost on reset or power-off.

* Programs can be saved to and loaded from **EEPROM**  
  using the `SAVE` and `LOAD` commands.

```
PROG
> '' This comment will be skipped
> DO
>  OUTP 13, 1
>  DELAY 200
>  OUTP 13, 0
>  DELAY 200
> LOOP
> #
OK
RUN
```

Notes:  
* Each line is compiled as soon as it is entered.  
* When sending lines automatically, insert a delay of about **100 ms** between lines.  
* The program area resides in RAM and is cleared on reset or power-off.

### RUN
```
RUN
```
It will enter Run mode and the program in the program area will be executed from the beginning.  
Before execution starts, all variables are cleared to zero.
When the program execution in the program area is completed or interrupted,  
it will return to REPL mode.

### LIST
```
LIST
```
Outputs the stored program.  

The display style (uppercase / lowercase) used by the LIST command can be configured at build time.

### SAVE
```
SAVE [Arguments]
```
Saves or erases the contents of the program area to EEPROM.  
Programs saved to EEPROM are not erased even by reset or power outage.  
You can also set them to run automatically.  
Programs saved to EEPROM can be loaded with the LOAD command.  

- **Normal Save** (No Arguments)  
Saves the contents of the program area to EEPROM.  
An error occurs if the program area is empty (to prevent accidental operation).  
AutoRun is disabled.  
- **AutoRun Save** (Argument: `!`)  
In addition to a normal save, this command enables AutoRun and saves.  
If AutoRun is enabled, the program will automatically run after LOAD when power is turned on.  
- **Erase** (Argument: `0`)  
Erase the program stored in EEPROM.  
The contents of the program area are not affected.  
AutoRun is disabled.  

### LOAD
```
LOAD
```
Loads a program stored in EEPROM into the program area.  
This will replace any existing programs in the program area.  
If there are no programs in the EEPROM, an error will occur.  

### RANDOMIZE
```
RANDOMIZE expression
```
Sets the random seed.

### OUTP
```
OUTP pin, value
```
Outputs a digital signal.

| Pin | Function |
|-----|----------|
| 0–7 | PORTD |
| 8–13 | PORTB |

`0 = LOW`, non-zero = HIGH.

### PWM
```
PWM pin, value
```
Outputs PWM using Arduino’s `analogWrite()` (8-bit duty: 0–255).

| Pin | Function |
|-----|----------|
| 3,5,6,9,10,11 | PWM |

---

## Functions

### RND
```
RND(expression)
```
Returns a random number less than the expression.

### ABS
```
ABS(expression)
```
Returns the absolute value.

### INP
```
INP(expression)
```
Reads a digital input.

| Pin | Port |
|-----|------|
| 0–7 | PORTD |
| 8–13 | PORTB |

Returns `0` (LOW) or `1` (HIGH).

### ADC
```
ADC(expression)
```
Reads an analog input.

| ADC | Pin |
|-----|------|
| 0–5 | A0–A5 |

Returns an integer 0–1023.

---

### INKEY

```
INKEY(expression)
```
Retrieves a **single character input** from the serial port.  
The argument specifies the **timeout duration in milliseconds (ms)**.

* If the expression is **0 or less**, the function waits **indefinitely** until a key is pressed.
* If no input is received within the specified time, **-1** is returned.
* If a key is received, its **ASCII code** is returned.

*Note:* **Ctrl-C** is always handled as an execution break and cannot be captured by `INKEY`.

---

## PRINT-Only Functions

nanoBASIC provides several functions that are **valid only within the `PRINT` command**.

These functions are used for output formatting and character display  
and cannot be used in expressions or assignments.

### CHR
```
CHR(expression)
```

Outputs the character corresponding to the ASCII code specified by the expression  
when used in a `PRINT` command.  

Only the **lower 8 bits** of the expression are used.

---

### DEC
```
DEC(expression)
DEC(expression, width)
```

Outputs the value of the expression in **decimal format**.  

By using this function, decimal numbers can be displayed  
with an explicitly specified output width.

For details on width specification and formatting behavior,  
refer to **[Numeric Output and Format Specification]** in the `PRINT` command section.

---

### HEX
```
HEX(expression)
HEX(expression, width)
```

Outputs the value of the expression in **hexadecimal format**.

By using this function, hexadecimal numbers can be displayed  
with an explicitly specified output width.

For details on width specification and formatting behavior,  
refer to **[Numeric Output and Format Specification]** in the `PRINT` command section.

---

## Reserved Variables
nanoBASIC UNO has system reserved variables.  

### TICK
System tick counter (increments approximately every 1 ms).

---

## AutoRun Function
nanoBASIC UNO provides an AutoRun feature that automatically executes a  
program stored in EEPROM when the device is powered on.

AutoRun is enabled when the program is saved using `SAVE !`.    
No additional configuration commands are required.  
A normal `SAVE` does not enable AutoRun.

When AutoRun is active, the interpreter automatically performs  
`LOAD → RUN` approximately 3 seconds after power-on or reset.

AutoRun can be cancelled by pressing Ctrl-C within this 3-second window.

---

## Error Messages

* **Syntax error:**  
  Invalid syntax or unrecognized command.

* **Division by 0 error:**  
  Division by zero occurred.

* **Array index over error:**  
  Index is negative or exceeds the configured maximum.

* **Parameter error:**  
  Required parameter is missing.  
  Or the parameter range is invalid.

* **Stack overflow error:**  
  Nesting depth exceeded 8 for GOSUB/FOR/DO/WHILE.

* **Can't resume error:**  
  Cannot resume because execution stopped due to an error.

* **Label not found error:**  
  GOTO/GOSUB target label does not exist.

* **Unless from run-mode error:**  
  Command cannot be executed in program mode.  
  (LOAD, NEW, LIST, RUN, RESUME)

* **PG area overflow error:**  
  Program writing exceeded available memory.

* **PG empty error:**
  The program is empty on the SAVE or LOAD command.

* **Loop nothing error:**  
  Missing LOOP corresponding to DO/WHILE.

* **Endif not found error:**  
  Missing ENDIF.

* **Unexpected Next error:**  
  NEXT without matching FOR.

* **Unexpected Return error:**  
  RETURN without matching GOSUB.

* **Unexpected Loop error:**  
  LOOP without matching DO/WHILE.

* **Unexpected Exit error:**  
  EXIT without matching loop.

* **Unexpected Continue error:**  
  CONTINUE without matching loop.

* **Unexpected Read error :**	  
  READ without matching DATA.

---

## 🆕 Version Updates (Additional Section)

### 🔄 Major Changes

In **Version 0.18**, the following feature enhancements and internal improvements were introduced:

* **32-bit integer support (configurable at build time)**  
  In addition to the default 16-bit integers,  
  numeric values can now be configured to use **32-bit integers** at build time.

* **REPL variable retention and command history**  
  The REPL mode now preserves variable values  
  and supports command history navigation using the up/down arrow keys.

* **Support for odd (unaligned) memory access**  
  Internal implementation has been adjusted to allow operation on MCUs  
  that do not permit unaligned memory access,  
  improving portability to other architectures.

* **C-style escape sequences for string literals**  
  String literals now support C-compatible escape sequences, such as  
  `\n`, `\r`, `\t`, `\\`, and `\"`.

* **UTF-8 support for strings and comments**  
  UTF-8 characters can now be used in strings printed by `PRINT`  
  and within comments.

* **Hexadecimal notation changed from `$` to `0x`**  
  Hexadecimal numeric literals now use the `0x` prefix,  
  consistent with C language conventions.

---

## ⚠️ Memory Usage Notes (Important)

The amount of RAM used by nanoBASIC UNO varies significantly  
depending on build-time configuration options.

Arduino UNO (ATmega328P) has only **2 KB of RAM**,  
and enabling the following options at the same time can cause  
**severe memory pressure**:

* **32-bit integer support enabled**  
* **REPL command history enabled**  

When built with this configuration,  
the remaining free RAM on the UNO is approximately **220 bytes**.

While the system can still start and run in this state,  
runtime conditions such as:  

* Temporary buffers during execution
* Stack usage during expression evaluation
* Recursive processing or deeply nested control structures

may result in:

* Unpredictable behavior
* Program termination
* System hangs

---

### 🔧 Recommended Mitigations

To increase available RAM and improve stability,  
consider applying one or more of the following adjustments.

Edit `nano_basic_uno_conf.h` and rebuild the firmware as needed.

* **Reduce the program area size**

  ```
  #define PROGRAM_AREA_SIZE 768 → 512
  ```

* **Disable REPL history**

  ```
  #define REPL_HISTORY_ENABLE 1 → 0
  ```

* **Build with 16-bit integers**

  ```
  #define NANOBASIC_INT32_EN  1 → 0
  ```

* **Reduce array size**

  ```
  #define ARRAY_INDEX_NUM   64 → 16
  ```

Adjusting the memory balance through build-time configuration  
is **strongly recommended**, depending on your use case.

---

## 📌 Additional Notes (Design Philosophy)

nanoBASIC UNO is designed to balance the following goals:  

* A configuration that **barely fits and runs on the UNO**  
* A configuration that runs **comfortably on other MCUs**  

On the UNO, nanoBASIC UNO is **not intended to be built with all features enabled**.  

Instead, selecting an appropriate build configuration  
based on the intended use case is part of the design philosophy.

---

## 📄 License

nanoBASIC UNO is released under the **MIT License**.

This software may be freely used, modified, and redistributed  
for both personal and commercial purposes,  
provided that the copyright notice and license text  
are included in the source code or distributed materials.  

For the full license text,  
please refer to `LICENSE.md` in the repository.  

Copyright (c) 2025–2026 shachi-lab  
License: MIT

---
