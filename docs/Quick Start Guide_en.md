# 📘nanoBASIC UNO — Quick Start Guide (English)

This guide explains how to run nanoBASIC UNO **as quickly as possible**.  
Try launching it once to get a feel for how the BASIC interpreter works.

---

## 1. Requirements
- Arduino UNO (ATmega328P)
- Arduino IDE (latest version)
- USB cable
- Serial terminal (Arduino IDE's monitor is OK for basic use)

---

## 2. Uploading nanoBASIC UNO

1. Open `nanoBASIC_UNO.ino` in Arduino IDE  
2. Select **Arduino UNO** as the board  
3. Set the serial speed to **115200 bps**  
4. Click **Upload**

After uploading, open the Serial Monitor (115200 bps).
You should see:

```
nanoBASIC UNO Ver 0.15
OK
```
### **Important**
**Set "Line Ending" to `CR/LF`.**  
If this is not set, commands may not be interpreted correctly.

---

## 3. Try simple commands
Immediately after starting nanoBASIC UNO, it is in REPL (Interactive) mode

### Display a string
```
PRINT "HELLO"
```

Or shorthand:
```
? "HELLO"
```

### Use a variable
```
A = 10
PRINT A
```

### Arithmetic
```
PRINT 1 + 2 * 3
```
### Multiple statements on a single line
```
A=1:B=2:C=(A+B)<<3:PRINT "C="C
```
```
FOR I=0 TO 10:PRINT I*I:NEXT
```
---

## 4. Create your first program (blink the LED)

### Enter program command
```
PROG
```

### Enter the following lines exactly as written:
```
DO
OUTP 13, 1
DELAY 500
OUTP 13, 0
DELAY 500
LOOP
#
```
`#` ends PROG mode.

---

## 5. Run the program
The `RUN` command enters Run (program execution) mode.
```
RUN
```

The **LED on pin 13** should blink. ✨

---

## 6. Interrupt execution (Break)

nanoBASIC UNO uses **Ctrl-C (0x03)** as the only break signal.

### Important Note
Arduino IDE **cannot send Ctrl-C**.  
To use program interruption, you must use an external terminal such as:

- Tera Term (recommended)
- PuTTY
- CoolTerm
- minicom / screen

Press Ctrl-C to interrupt execution.

Resume with:
```
RESUME
```

---

## 7. View the stored program
```
LIST
```

---

## 8. Clear the stored program
```
NEW
```

---

## 9. Try more commands

### Read a number from the terminal
```
INPUT A
PRINT A
```

### Read analog input
```
PRINT ADC(0)
```

### Random numbers
```
RANDOMIZE 1
PRINT RND(100)
```

---

## You are now ready to explore nanoBASIC UNO!
For detailed language specifications, command reference, and complete behavior descriptions,  
see the **Reference Manual**.

---
