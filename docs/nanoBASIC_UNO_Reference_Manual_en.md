# 📘**nanoBASIC UNO Reference Manual**

English edition (Translated from the Japanese manual)
for Version 0.14

## Overview
nanoBASIC UNO is a BASIC interpreter running on Arduino UNO (ATmega328P).  
Each command entered from the serial terminal is translated into an intermediate bytecode format and executed by the interpreter.

* Line numbers are treated as labels (not related to execution order)
* Each line is translated into bytecode, limited to 63 bytes
* Hand-written expression parser without recursion
* Program area resides in RAM and is directly written using the PROG command

---

## Operation Modes
nanoBASIC has two operation modes:

* **Interactive Mode:**  
  Commands typed from the terminal are executed one line at a time.

* **Program Mode:**  
  Executes the program stored in the program area sequentially.  
  After startup, the interpreter is in interactive mode.

---

## Input
Commands are entered through the serial port.  
Statements are case-insensitive.  
However, comments and text inside strings preserve the original character case.

---

## Numbers
All numeric values in nanoBASIC are 16-bit signed integers (−32768 to 32767).  
Floating-point types are not supported.  
Overflow is not handled.  
Decimal numbers are default; prefix with `$` to specify hexadecimal.

---

## Variables
* Scalar variables: `A` to `Z` (26 total, signed 16-bit integers)
* Array variable: One one-dimensional array `@[index]` (signed 16-bit), index range 0–63

All variables are global.  
There are no local variables.  
String variables do not exist.

---

## Expressions
An expression consists of numbers and operators.

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
| = | Equal |
| <> | Not equal |
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
Expressions can be used as arguments to commands or in assignments.  
They follow normal operator precedence.

---

## Assignment Statement
```
variable = expression
```
Assigns the result of the expression to the variable.  
There is no `LET` statement.

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
A comment begins with `'` and continues to the end of the line.  
When writing after a command, place comments after a `:`.

---

# Commands

## PRINT
```
PRINT expression
? expression
```

* `?` may be used as a shorthand for PRINT.  
* Multiple expressions may be separated with commas (tab) or semicolons (no newline).  
* Ending a PRINT statement with `;` suppresses the newline.  
* Strings, `CHR()`, and formatting functions may be used only inside PRINT.

---
## INPUT
```
INPUT variable
```

* Waits for user input and stores the value in the variable.  
* If input begins with `$`, it is parsed as hexadecimal.  
* Appending `$` to the variable name stores the ASCII code of the first character.

---

## GOTO
```
GOTO expression
```
Jumps to the label indicated by the expression.

---

## GOSUB / RETURN
```
GOSUB expression
RETURN
```

Jumps to a subroutine; RETURN returns execution to the following statement.

---

## IF / THEN / ELSEIF / ELSE / ENDIF
```
IF expr THEN statements
ELSEIF expr THEN statements
ELSE statements
ENDIF
```

* `0` = false  
* Non-zero = true  
* A number following THEN/ELSE acts as GOTO

---

## FOR / TO / STEP / NEXT
```
FOR var = expr1 TO expr2 STEP expr3
...
NEXT
```
STEP may be omitted (defaults to 1).
---

## DO / LOOP
```

DO
...
LOOP
```

### DO / LOOP WHILE
```

DO
...
LOOP WHILE expr
```

### WHILE / LOOP
```
WHILE expr
...
LOOP
```

---

## EXIT
Exits the nearest FOR/NEXT or DO/WHILE/LOOP block.  
Execution continues after the corresponding NEXT or LOOP.

---

## CONTINUE
Skips to the next iteration of a loop.

---

## END
Terminates program execution and returns to interactive mode.

---

## STOP / RESUME
```
STOP
RESUME
```

STOP halts execution.  
RESUME continues only if stopped normally (not by error).

---

## DELAY
```
DELAY expression
```
Waits for the specified number of milliseconds.

---

## PAUSE
Waits until one character is received from the serial port.

---

## DATA
```
DATA expr1, expr2, ...
```

Data values for READ. Expressions may be used.

---

## READ / RESTORE
```
READ variable
RESTORE
```

READ retrieves values from DATA statements in order.  
RESTORE resets the read position to the start.

---

## NEW
Clears the program area.

---

## PROG
Writes program lines into the RAM program area.

```
PROG
> DO
> OUTP 13, 1
> DELAY 200
> OUTP 13, 0
> DELAY 200
> LOOP
>
> #
OK
RUN
```

Notes:

* Each line is compiled as soon as it is entered.  
* When sending lines automatically, insert a delay of about **100 ms** between lines.  
* The program area resides in RAM and is cleared on reset or power-off.

---

## RUN
Clears all variables to zero and runs the stored program.

---

## LIST
Outputs the stored program.

---

## RANDOMIZE
```

RANDOMIZE expression

```
Sets the random seed.

---

## OUTP
```

OUTP pin, value

```
Outputs a digital signal.

| Pin | Function |
|-----|----------|
| 0–7 | PORTD |
| 8–13 | PORTB |

`0 = LOW`, non-zero = HIGH.

---

## PWM
```

PWM pin, value

```

Outputs PWM using Arduino’s `analogWrite()` (8-bit duty: 0–255).

| Pin | Function |
|-----|----------|
| 3,5,6,9,10,11 | PWM |

---

# Functions

## RND(expression)
Returns a random number less than the expression.

## ABS(expression)
Returns the absolute value.

## INP(expression)
Reads a digital input.

| Pin | Port |
|-----|------|
| 0–7 | PORTD |
| 8–13 | PORTB |

Returns `0` (LOW) or `1` (HIGH).

---

## ADC(expression)
Reads an analog input.

| ADC | Pin |
|-----|------|
| 0–5 | A0–A5 |

Returns an integer 0–1023.

---

## CHR(expression)
PRINT-only.  
Outputs the character for the given ASCII code.

---

## String Formatting Functions
PRINT-only.  

Decimal:
```
0(value, width)
```

Hexadecimal:
```
$(value, width)
```

Width rules:
* `=0` → automatic width  
* `>0` → space-padding  
* `<0` → zero-padding  

Examples:
```
$(1234,4)   -> [ 4D2]
$(1234,-4)  -> [04D2]
0(1234,5)   -> [ 1234]
0(-1234,-6) -> [-01234]
0(-1234,0)  -> [-1234]
```

---

# Reserved Variables

## TICK
System tick counter (increments approximately every 1 ms).

## INKEY
ASCII code of the last received character.  
Returns `0` if no character is available.

---

# Error Messages

* **Syntax error:**  
  Invalid syntax or unrecognized command.

* **Division by 0 error:**  
  Division by zero occurred.

* **Array index over error:**  
  Index is negative or greater than 255.

* **Parameter error:**  
  Required parameter is missing.

* **Stack overflow error:**  
  Nesting depth exceeded 8 for GOSUB/FOR/DO/WHILE.

* **Can't resume error:**  
  Cannot resume because execution stopped due to an error.

* **Label not found error:**  
  GOTO/GOSUB target label does not exist.

* **Unless from pg-mode error:**  
  Command cannot be executed in program mode.  
  (LOAD, NEW, LIST, RUN, RESUME)

* **Program area overflow error:**  
  Program writing exceeded available memory.

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

---
