# nanoBASIC Sample Programs

* This file contains a collection of sample programs for **nanoBASIC**.
* These samples are confirmed to work with **nanoBASIC Version 0.18**.  
  Some samples may produce errors on other versions.

---

## How to Use

* Copy and paste the samples into the nanoBASIC console  
  (such as **Tera Term** or the **Arduino Serial Monitor**) to run them.

#### Notes

* These samples are provided as **examples only**, without any warranty.  
* Execute them **at your own risk**.  
* Please verify LED, ADC, and other hardware connections on your own setup.

---

## One-liner samples (REPL) 

* Single-line samples are executed immediately  
  as soon as they are pasted into the REPL.

---

### Blink LED

Controls the LED blinking with a variable-based interval.  
Pasting it into the REPL will run it immediately.  
LED must be connected to pin 13.
```
T=500:DO:OUTP 13,1:DELAY T:OUTP 13,0:DELAY T:LOOP
```

---

### LED Random blinker

Randomizes the LED blink timing.  
You can easily try out irregular blinking behavior.  
LED must be connected to pin 13.
```
T=300:DO:OUTP 13,1:DELAY T+RND(T):OUTP 13,0:DELAY T+RND(T):LOOP
```

---

### PWM Random flicker

Randomizes the PWM output.  
You can easily try irregular changes in LED brightness.  
Connect the LED to pin 9 (PWM supported).
```
DO:PWM 9,RND(256):DELAY 100:LOOP
```

---

### FOR-NEXT Square Table

This is an example of a FOR–NEXT loop written on a single line.  
The calculation results are displayed directly.
```
FOR I=0 TO 10:? I" x "I" = "I*I:NEXT
```
---

### Cube Calculator

Calculates and displays the cube of the entered number.  
You can enter values repeatedly.
```
DO:? "Num ? ";:INPUT A:? A "^3 = "A*A*A:LOOP
```

---


## Multi-line samples (PROG)

* Multi-line samples should be copied from PROG to #.  
 After pasting them, run the program with RUN.

---

### Interactive Blink
Blinks the LED using the entered time interval (in milliseconds).  
This is an example of an interactive BLINK using INPUT.
LED must be connected to pin 13.
```
PROG
''------------------------------------
'' Blink built-in LED (D13) 
''------------------------------------
? "Input period(ms) = ";
INPUT T
DO
  OUTP 13,1
  DELAY T
  OUTP 13,0
  DELAY T
LOOP
#
```

---

### Candle-like PWM flicker using DATA

Evaluates the values and expressions defined in DATA sequentially  
and outputs them as LED brightness.  
This creates a flickering effect similar to a candle.   
Connect the LED to pin 9 (PWM supported).
```
PROG
''------------------------------------
'' Candle-like PWM flicker using DATA
''------------------------------------
DATA 10,30,80,20,50+RND(20),V-20,100,25,0
DO
  READ V
  IF !V THEN RESTORE:CONTINUE ENDIF
  PWM 9,V
  DELAY 150+RND(100)
LOOP
#
```

---

### PWM fade

Gradually increases and decreases the PWM output  
to fade the LED in and out.  
This is a simple example that switches the fade direction using a numeric value.  
Connect the LED to pin 9 (PWM supported).
```
PROG
''------------------------------------
'' PWM fade (pin 9)
''------------------------------------
A=0:B=5
DO
  PWM 9,A
  A+=B
  IF A>=255 THEN B=-5 ENDIF
  IF A<=0 THEN B=5 ENDIF
  DELAY 20
LOOP
#
```

---

### Triple nested FOR-NEXT loop

This is an example of triple-nested FOR–NEXT loops.  
It helps you understand loop nesting relationships and execution order.
```
PROG
''------------------------------------
'' Triple nested FOR-NEXT loop
''------------------------------------
FOR Z=1 TO 2
  FOR Y=1 TO 3
    FOR X=1 TO 4
      ? "Cell ("Z","Y","X")"
    NEXT
  NEXT
NEXT
? "Done!"
#
```

### While - Loop sample

Repeats processing as long as the condition is satisfied.
A basic usage example of WHILE–LOOP.
```
PROG
''------------------------------------
'' WHILE-LOOP sample
''------------------------------------
A=1
WHILE A<5
  ? A
  A++
LOOP
? "Done!"
#
```

---

### Key Code Monitor

Waits for a key input and displays the received key code.  
If no key is pressed, it continues waiting; press the Space key to exit.
```
PROG
''------------------------------------
'' Key Code Monitor
''------------------------------------
DO
  K=INKEY(0)
  IF K<0 THEN CONTINUE ENDIF
  ? HEX(K,-2) ",";
LOOP WHILE K!=0x20
? "\nDone!"
#
```

---

### LED Control by INKEY

Toggles the LED ON/OFF in response to key input.  
This example waits for key input using INKEY(0).  
Connect the LED to pin 13.
```
PROG
''------------------------------------
'' LED (pin 13) control by INKEY
''------------------------------------
? "1: LED ON"
? "2: LED OFF"
? "3: END"
? "Select (1-2)"
DO
  K=INKEY(0)
  IF K=49 THEN OUTP 13,1     '' '1'
  ELSEIF K=50 THEN OUTP 13,0 '' '2'
  ELSEIF K=51 THEN EXIT      '' '3'
  ELSE ? "invalid"
  ENDIF
LOOP
#
```

---

### Just Timing!

Press a key based on the displayed number of seconds.  
This measures human time perception in numeric form.
```
PROG
''------------------------------------
'' Just Timing!
'' (Random seconds challenge)
''------------------------------------
S=RND(10)+3
? S "sec challenge."
delay 1000
? "Ready ?"
delay 1000
? "Go !"
T=TICK
K=INKEY(0)
T=TICK-T-S*1000
IF T=0 THEN
  ? "Just !!"
ELSEIF T>0 THEN
  ? T " msec late."
ELSE
  ? ABS(T) " msec early."
ENDIF
#
```

---

### ADC to JSON

Reads analog values obtained via the ADC,  
converts them to voltage, and outputs the result in JSON format.  
This is an example where sensor data can be used directly  
for logging or communication.
```
PROG
''------------------------------------
'' ADC to JSON
''------------------------------------
F=5 ''AREF
DO
  S++
  ? "{\"dev\":\"UNO\",\"seq\":" S ",[";
  FOR N=0 TO 3
    GOSUB 100
    IF N<3 THEN PRINT ","; ENDIF
  NEXT
  ? "]}"
  DELAY 10000
LOOP
100 '
A=ADC(N)
V=(A*31/32)*F
? "{\"an\":" N ",\"adc\":\"0x" HEX(A,-4); "\",\"volt\":" DEC(V,300) "}";
RETURN
#
```

---

## License

* These sample programs are released under the MIT License.
* They may be freely used, modified, and redistributed.
