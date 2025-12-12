# 📘**nanoBASIC UNO Reference Manual**

English edition (Translated from the Japanese manual)
for Version 0.15

## Overview
nanoBASIC UNO is a BASIC interpreter running on Arduino UNO (ATmega328P).  
Each command entered from the serial terminal is translated into an intermediate bytecode format and executed by the interpreter.

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
```
A=10
B=$BEEF
```

---

## Variables
* Scalar variables: `A` to `Z` (26 total, signed 16-bit integers)
* Array variable: One one-dimensional array `@[index]` (signed 16-bit), index range 0–63

All variables are global.  
There are no local variables.  
String variables do not exist.

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
| <>,!= | Not equal |
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
Calculations are performed according to basic priority.

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
A comment begins with `'` and continues to the end of the line.  
When writing after a command, place comments after a `:`.

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


### INPUT
```
INPUT variable
```

* Waits for user input and stores the value in the variable.  
* If input begins with `$`, it is parsed as hexadecimal.  
* Appending `$` to the variable name stores the ASCII code of the first character.

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

* `0` = false  
* Non-zero = true  
* A number following THEN/ELSE acts as GOTO

### FOR / TO / STEP / NEXT
```
FOR var = expr1 TO expr2 STEP expr3
...
NEXT
```
After assigning expr1 to the variable name, it loops between NEXT and NEXT until the variable becomes expr2.
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
Skips to the next iteration of a loop.

### END
```
END
```
Terminates program execution and returns to REPL mode.
When execute in REPL mode, it initializes the nested stack and the suspended state.

### STOP
```
STOP
```
Suspend program execution and returns to REPL mode.
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
Writes program lines into the program area.

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

### RUN
```
RUN
```
It will enter Run mode and the program in the program area will be executed from the beginning.
Prior to this, clears all variables to zero
When the program execution in the program area is completed or interrupted,
it will return to REPL mode.

### LIST
```
LIST
```
Outputs the stored program.

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

### RND(expression)
Returns a random number less than the expression.

### ABS(expression)
Returns the absolute value.

### INP(expression)
Reads a digital input.

| Pin | Port |
|-----|------|
| 0–7 | PORTD |
| 8–13 | PORTB |

Returns `0` (LOW) or `1` (HIGH).

### ADC(expression)
Reads an analog input.

| ADC | Pin |
|-----|------|
| 0–5 | A0–A5 |

Returns an integer 0–1023.

### CHR(expression)
PRINT-only.  
Outputs the character for the given ASCII code.

### String Formatting Functions
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
```
$(1234,4)   -> [ 4D2]
$(1234,-4)  -> [04D2]
0(1234,5)   -> [ 1234]
0(-1234,-6) -> [-01234]
0(-1234,0)  -> [-1234]
```

---

## Reserved Variables
nanoBASIC UNO has system reserved variables.  

### TICK
System tick counter (increments approximately every 1 ms).

### INKEY
ASCII code of the last received character.  
Returns `0` if no character is available.

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
  Index is negative or greater than 255.

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

* **PG empty error :**
  The program is empty on the SVAE or LOAD command.

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
