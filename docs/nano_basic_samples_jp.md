# nanoBASIC サンプル集

- このファイルは nanoBASIC のサンプル集です。
- nanoBASIC Version 0.18 で動作します。  
  他のバージョンではエラーになるものがあります。

---

## 使い方

- nanoBASIC のコンソール（TeraTerm / シリアルモニタなど）に  
  コピー＆ペーストして使ってください。

#### 注意

- 本サンプルは動作例です。無保証で提供されます。
- 実行は自己責任で行ってください。
- LED や ADC などの接続は各自で確認してください。

---

## One-liner samples (REPL) 

- 1行のサンプルは、  
  REPL に貼り付けた時点でそのまま実行されます。

---

### Blink LED

変数で周期を指定して、LED を点滅させます。  
REPL に貼り付けると、そのまま動きます。  
※ LED は pin 13 に接続してください。
```
T=500:DO:OUTP 13,1:DELAY T:OUTP 13,0:DELAY T:LOOP
```

---

### LED Random blinker

LED の点灯タイミングを乱数で変化させます。  
不規則な変化を手軽に試せます。  
※ LED は pin 13 に接続してください。
```
T=300:DO:OUTP 13,1:DELAY T+RND(T):OUTP 13,0:DELAY T+RND(T):LOOP
```

---

### PWM Random flicker

PWM 出力を乱数で変化させます。  
不規則な明るさ変化を手軽に試せます。  
※ LED は PWM が使える pin 9 に接続してください。
```
DO:PWM 9,RND(256):DELAY 100:LOOP
```

---

### FOR-NEXT Square Table

FOR–NEXT を 1 行で書いた例です。  
計算結果をそのまま表示します。
```
FOR I=0 TO 10:? I" x "I" = "I*I:NEXT
```
---

### Cube Calculator

入力した数値の 3 乗を計算して表示します。  
繰り返し入力できます。
```
DO:? "Num ? ";:INPUT A:? A "^3 = "A*A*A:LOOP
```

---


## Multi-line samples (PROG)

- 複数行のサンプルは、PROG から # までをコピーし、  
  貼り付け後に RUN で実行します

---

### Interactive Blink

入力した時間（ms）で LED を点滅させます。  
INPUT を使った対話的な BLINK の例です。
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

DATA に書いた値と式を順番に評価し、  
LED の明るさとして出力します。  
ろうそくのような揺らぎを表現します。  
※ LED は PWM が使える pin 9 に接続してください。
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

PWM 出力を徐々に増減させて、  
LED をフェードイン／フェードアウトさせます。  
増減方向を数値で切り替える、シンプルな例です。  
※ LED は PWM が使える pin 9 に接続してください。
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

FOR–NEXT を三重に入れ子で使う例です。  
ループの対応関係と、処理順序を確認できます。
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

条件が成り立つ間、処理を繰り返します。  
WHILE–LOOP の基本的な使い方の例です。
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

キー入力を待ち、入力されたキーコードを表示します。  
何も入力がない場合は待ち続け、Spaceキーで終了します。
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

キー入力に応じて LED の ON / OFF を切り替えます。  
INKEY(0) を使って、キー入力を待ち受ける例です。  
※ LED は pin 13 に接続してください。
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

表示された秒数を目安にキーを押します。  
人間の時間感覚を数値で測定します。
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

ADC で取得したアナログ値を読み取り、  
電圧に換算して JSON 形式で出力します。  
センサーデータをそのままログや通信に使える例です。
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

## ライセンス

- 本サンプルコードは MIT License で公開されています。  
- 自由に使用・改変・再配布できます。


