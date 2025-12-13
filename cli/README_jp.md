# nanoBASIC UNO - CLI 版

これは **nanoBASIC UNO** のコマンドライン（CLI）版です。

Arduino UNO 版と **完全に同一のコアソースコード** を使用しています。  
BIOS の実装と `main` のみが異なります。

---

## ビルド方法

以下のファイルを同じディレクトリに配置し、  
標準的な C++ コンパイラでビルドしてください。

- `main.cpp`
- `nano_basic_uno.cpp`
- `bios_uno_cli.cpp`
- `nano_basic_uno.h`
- `bios_uno.h`

### ビルド例（Linux / g++）

```
g++ -std=gnu++17 main.cpp nano_basic_uno.cpp bios_uno_cli.cpp -o nanoBASIC_UNO
```

---

### Windows

Windows と Linux は **まったく同じソースファイル** を使用します。  
お好みのコンパイラ（MSVC / clang / MinGW など）でビルドしてください。

---

## 実行方法

生成された実行ファイルを起動し、  
そのまま BASIC コマンドを入力してください。

CLI 版は Arduino 版と同等の REPL 環境を提供します。  
`RUN`、`DELAY`、`Ctrl-C` による中断も同様に動作します。

---

## 終了方法

`Ctrl-D` を押すと CLI を終了します。

---

## ハードウェア関連コマンドについて

CLI 環境では、ハードウェア関連のコマンドはエラーにはなりませんが、  
次のような挙動となります。

- `OUTP`、`PWM`  
  → 効果はありません
- `INP()`、`ADC()`  
  → 常に `0` を返します

---

## SAVE / LOAD

CLI 版では、`SAVE` / `LOAD` は  
カレントディレクトリ内の `eeprom.bin` ファイルを使用して  
EEPROM をエミュレートします。

`SAVE` を実行すると、`eeprom.bin` は自動的に作成されます。

---

## リソース制限について

CLI 版でも、Arduino UNO 版と同じ  
プログラム容量およびメモリ制限が適用されます。

互換性と動作の一貫性を保つために、このようになっています。

---

## 補足

- IDE やプロジェクトファイルは不要です
- nanoBASIC UNO コアのソースコードは OS 非依存です
- OS依存やプラットフォーム差分は BIOS レイヤに分離されています

---