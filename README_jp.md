# nanoBASIC UNO  （日本語版）
*A minimal BASIC interpreter for Arduino UNO (ATmega328P)*

![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)
![Platform: AVR](https://img.shields.io/badge/Platform-AVR-red)
![Arduino](https://img.shields.io/badge/Arduino-UNO-blue)

## 💾このプロジェクトについて

**nanoBASIC UNO** は、shachi-lab が開発した  
**8bit AVR（Arduino UNO）向けのミニマル BASIC インタプリタ**です。

2012年に STM8S 用として作成した最初期バージョンをベースに、  
現行アーキテクチャに合わせて再設計・再実装しています。

本プロジェクトは、**小型 MCU 上で「現代的に使える BASIC」** を  
最小限のメモリで実現することを目的としています。

For English readers: [README.md](README.md)

![nanoBASIC UNO screenshot](./images/screenshot_1.png)   
nanoBASIC UNO の実行例。起動メッセージ（Ver 0.14）に続き、  
単一行での複数文、FOR/NEXT、そして Arduino UNO の内蔵LEDを  
制御するシンプルなブリンクプログラムを実行しています。  
Ctrl-C によるブレーク動作も確認できます。

---

### CLI バージョン

nanoBASIC UNO のコマンドライン (CLI) バージョンが利用可能です。  

Arduino UNO ボードがなくても、nanoBASIC UNO を試せます。  
CLI 版は Windows と Linux でビルドすることができます。  
詳細は cli/ ディレクトリを参照してください。

---

## ✨機能

- **ミニマル構造の BASIC インタプリタ**
  - 行番号＝ラベル
  - 中間コード形式の仮想マシン
  - 再帰下降パーサによる式評価
- **Arduino UNO 上で動作（ATmega328P）**
- **REPL Mode と Run Mode** に対応
- **GPIO / ADC / PWM / TICK / INKEY 対応**
- **DATA / READ / RESTORE** などクラシック BASIC の基本機能を搭載
- **+= / -= / <<= / >>= / && / ||** など現代風の演算子にも対応  
- **SAVE / LOAD** でプログラムの保存と読み込み、自動実行に対応
- プラットフォーム依存部は **BIOS 層（bios_uno.cpp）** に完全分離
- 移植性を重視したヘッダ構造
- MIT License

---

## 🚀クイックスタートガイド

いますぐ nanoBASIC UNO を始めるために：

- **クイックスタートガイド（日本語版）**  
  → [docs/QuickStart_jp.md](docs/QuickStart_jp.md)

- **Quick Start (English)**  
  → [docs/QuickStart_en.md](docs/QuickStart_en.md)

---

## 📘ドキュメント

- **リファレンスマニュアル（日本語版）**  
  → [docs/nanoBASIC_UNO_Reference_Manual_ja.md](docs/nanoBASIC_UNO_Reference_Manual_ja.md)

- **Reference Manual (EN)**  
  → [docs/nanoBASIC_UNO_Reference_Manual_en.md](docs/nanoBASIC_UNO_Reference_Manual_en.md)

---

## 📦リポジトリの構成

```
/ (project root)
├── docs
|    ├── QuickStart_en.md
|    ├── QuickStart_jp.md
|    ├── nanoBASIC_UNO_Reference_Manual_en.md
|    └── nanoBASIC_UNO_Reference_Manual_ja.md
├── examples
|    └── ...
├── src
|    ├── nano_basic_defs.h      # 言語仕様（プラットフォーム非依存）
|    ├── nano_basic_uno_conf.h  # UNO版の設定・バージョン情報
|    ├── nano_basic_uno.h       # Arduino向けの最小限API（basicInit/basicMain）
|    ├── nano_basic_uno.cpp     # インタプリタ本体
|    ├── bios_uno.cpp           # Arduino UNO 依存のハードウェア層
|    └── bios_uno.h             # ハードウェア層のAPI
├── nanoBASIC_UNO.ino           # Arduino 用エントリーポイント
├── README.md
├── README_jp.md
└── LICENSE.md
```

---

## 🧠アーキテクチャの概要

### インタプリタ本体

- `convertInternalCode()`  
  BASIC ソース → 内部コード（tokenized）変換  
- `interpreterMain()`  
  内部コード実行ループ  
- `expr()` 系  
  再帰下降による演算子評価

### BIOS レイヤー
Arduino UNO 依存の処理はすべて **bios_uno.cpp** に隔離。

- Digital I/O  
- ADC  
- PWM  
- Random  
- System tick  
- Serial I/O  
- EEPROM

他 MCU へ移植したい場合は、  
bios_uno.cpp / bios_uno.h を置き換えるだけで対応可能です。

---

## ⚙️AVR（Arduino UNO）固有の実装について

nanoBASIC UNO は Arduino UNO（ATmega328P）のメモリ構造に最適化するため、
一部の文字列処理が **AVR 固有の PROGMEM / pgm_read_byte() 形式**で実装されています。

AVR は Harvard アーキテクチャであり、  
プログラムメモリ（Flash）と RAM が完全に分離しているため、

- 文字列は PROGMEM に格納  
- 読み出しには `pgm_read_byte()` を使用  
- `F("...")` マクロで RAM 使用量を抑制  
- `printStringFlash()` など AVR 専用の文字列処理関数を使用  

といった実装になっています。

### 📝他 MCU へ移植する場合

ESP32 / ARM / RP2040 など Flash と RAM が統合された MCU では  
上記の仕組みは不要のため、

以下のいずれかの方法で対応可能です：

- BIOS 層のように「文字列アクセス層」を抽象化する。  
- PROGMEM 使用部分を RAM 文字列に置き換える。  
- `pgm_read_byte()` を単純なメモリアクセスに変更する。  

この点を除けば nanoBASIC のコア部分はプラットフォーム非依存であり、  
移植は比較的容易です。

---

## 🚀Getting Started

### 1. Arduino IDE でプロジェクトを開く  
`nanoBASIC_UNO.ino` を開くだけでOK。

### 2. コンパイル & 書き込み  
ボードを **Arduino UNO** に設定して書き込む。

### 3. シリアルモニタ  
シリアルモニタを次のように設定してください。
* 115200 baud
* CR+LF

起動メッセージ：

```
nanoBASIC UNO Ver 0.15
OK
```

---

## 💡 REPL Mode と Run Mode

nanoBASIC UNO には **2つの動作モード**があります。

### 🔹 1. REPL（対話実行）モード
起動直後は入力した1行分のコマンドは **即実行されます。**  
このとき、プログラム領域には保存されません。
```
? 120+3
123
OK
```
→ すぐに `123` を表示して終了。  
このモードは簡単な計算や動作確認に便利です。  
変数やマルチステートメントも使えます。  
```
A=2:FOR I=1 TO 3:? I*A:NEXT
2
4
6
OK
```

### 🔹 2. Run（プログラム実行）モード
複数行のプログラムを実行したい場合は `PROG` コマンドを使用します。  
```
PROG
>DO
>OUTP 13,1
>DELAY 500
>OUTP 13,0
>DELAY 500
>LOOP
>#
RUN
```
`#` でPROGコマンドを終了します。  
入力した行は、入力順にプログラム領域に保存され、  
`RUN` で実行できます。

※プログラム領域はRAMのため、リセットで消去されます。

---

## 実行の中断（Ctrl-C）

プログラム実行中に **Ctrl-C** を送信すると、  
即座に実行を中断して `STOP` と同様の状態になります。

長いループや意図しない無限ループから抜けるときに利用できます。

---

## 🔧 サポートされているステートメント

### 大文字・小文字の区別について
コマンド名・ステートメント名・関数名の大文字・小文字は区別しません。  
例： `PRINT`, `print`, `PrInT` はどれも同じ意味となります。

### コマンド
| Statement                  | Meaning      |
|----------------------------|--------------|
| PRINT                      | 出力         |
| INPUT                      | 入力         |
| GOTO                       | ジャンプ      |
| GOSUB / RETURN             | サブルーチン  |
| FOR / NEXT                 | 回数ループ    |
| DO / LOOP / WHILE          | 条件ループ    |
| CONTINUE / EXIT            | ループ制御    | 
| IF / ELSEIF / ELSE / ENDIF | 条件分岐      |
| DATA / READ / RESTORE      | データ操作    |
| OUTP                       | デジタル出力  |
| PWM                        | PWM 出力      |
| DELAY / PAUSE              | 待機          |
| END / STOP                 | 実行終了、中断|
| RESUME                     | 実行再開      |
| RESET                      | システム制御   |
| RUN / NEW                  | プログラム制御 |
| PROG / LIST / SAVE / LOAD  | プログラム管理 |
| RANDOMIZE                  | 乱数          |

### 関数
| Function | Meaning |
|----------|---------|
| ABS()    | 絶対値      |
| INP()    | デジタル入力 |
| ADC()    | アナログ入力 |
| RND()    | 乱数       |

### 特殊変数
| Valiable | Meaning |
|----------|---------|
| TICK | システム時間 |
| INKEY | シリアル入力 |


### 演算子
| Operator                | Meaning |
|-------------------------|---------|
| +, -, *, /, %           | 算術演算子 |
| -, !, ~                 | 単項演算子 |
| &,  \|,  ^              | ビット演算子 |
| &&, \|\|                | 論理演算子 |
| <<, >>                  | シフト演算子
| =, ==, <>, <=, >=, <, != | 比較演算子 |

### カッコ  
カッコで囲まれた内側の演算が優先されます。

### 数値  
nanoBASIC の数値はすべて 16bit 整数（-32768〜32767）です。  
floatなどの浮動小数点はありません。
演算によるオーバーフローは考慮しません。  
指定が無い場合は10進数です。  
数値に前に '$' をつけると16進数となります。
```
A=10
B=$BEEF
```

### 変数
一般変数：A～Z までの1文字が変数となります。  
配列変数：@[0]～@[63] 
```
Z=10
@[1]=0
```

### 代入文  
「変数=式」のように記述します。
```
A=(B+10)*5
```

### 複合代入演算子
変数への代入には、以下の複合代入演算子が使用できます。  
+=、 =、 *=、 /=、 %=、 |=、 &=、 ^= 、 <<=、 >>=
```
A<<=2
```

### インクリメント、デクリメント
変数に続けて、"++" または、"--" を記述すると、  
変数の内容を インクリメント(+1) または、デクリメント(-1) します。  
```
A++
```  
※ 式の中には記述できません。  

### 文字列
PRINT（"?"で代用可）コマンド内でのみ、文字列が使用できます。
```
? "Hello!"
Hello!
OK
A=5:? "A="A*2
A=10
OK
```
文字列変数はありません

### 条件判断  
IF/WHILE における条件は、数値が0か0以外かで判定します。  
比較演算子は必須ではありません。  
```
IF !A THEN PRINT "OK" ENDIF
```

### コメント
シングルクォート **'** から行末まではコメントとして扱われます。  
また、命令区切りの **":"** の後に `'` を記述することで、行の途中にコメントを書くこともできます。  
```
OUTP 13,1 :' LED を ON
```

---

## 🤝 コントリビュートについて

Issue / Pull Request を歓迎しています。  
軽量な BASIC インタプリタの改善案や移植案もぜひ。

---

## 🌐 今後

* nanoBASIC UNO は、今後のリリースで機能追加や改善をしていくつもりです。
* 将来的には、他のプラットフォームへの移植も検討しています。

---

## 📄 ライセンス

MIT License  
Copyright (c) 2025 shachi-lab

詳しくは [LICENSE.md](./LICENSE.md) を参照してください。

---
