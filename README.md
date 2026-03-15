# AE-RP2040_EMUZ80 (English)

![img1](./img/img1.jpg)

A peripheral and bus emulator for a physical Z80 CPU, designed to run on RP2040/RP2350 based boards.

This project uses the RP2040/RP2350's PIO (Programmable I/O) subsystem to emulate the memory and I/O devices required to operate a real Z80 microprocessor, allowing the Z80 to run using various RP-series boards without additional peripheral ICs.

![img2](./img/img2.jpg)
![img3](./img/img3.jpg)

## Supported Boards
- AE-RP2040_EMUZ80 (Akizuki Denshi AE-RP2040)
- Waveshare RP2350-Zero_EMUZ80 (Waveshare RP2350-Zero)
- WeAct RP2350B CoreBoard_EMUZ80 (WeAct RP2350B CoreBoard)

## Project Structure
The source code and configuration for each board are organized within the `boards/` directory:
- `boards/AE-RP2040/`: For Akizuki AE-RP2040
- `boards/RP2350-Zero/`: For Waveshare RP2350-Zero
- `boards/WeAct-RP2350B/`: For WeAct RP2350B CoreBoard

### Pre-built UF2 Files
Ready-to-flash UF2 files are available in the project root:
- `AE-RP2040_EMUZ80.uf2` — For Akizuki AE-RP2040
- `RP2350-Zero_EMUZ80.uf2` — For Waveshare RP2350-Zero
- `WeAct-RP2350B_EMUZ80.uf2` — For WeAct RP2350B CoreBoard

## Clock Settings

| Board | RP Clock | Z80 Clock |
|---|---|---|
| AE-RP2040 | 200 MHz | 2.5 MHz |
| RP2350-Zero | 150 MHz (default) | 4 MHz |
| WeAct RP2350B CoreBoard | 150 MHz (default) | 4 MHz |

## Terminal Software Note

For RP2350-based boards, the Z80 runs at 4 MHz, which is relatively fast. When sending data via a terminal application (e.g., TeraTerm), you **must configure a transmit delay** (character or line delay), otherwise characters may be dropped or corrupted.

## Known Issues

- The `/flash` slash command may fail to write the UF2 file correctly in some environments.
  - **Workaround**: Put the board into BOOTSEL mode, then **drag and drop** the UF2 file directly onto the board's drive in Explorer.

## Build Requirements
- Antigravity IDE (Recommended Build Environment)
- Pico C/C++ SDK
- CMake

## How to Build

This project has currently only been built and tested using the **Antigravity IDE**.

1. Open the project in the Antigravity IDE.
2. Select the correct **Pico SDK Board** in the status bar at the bottom:
   - Select **pico** for RP2040 boards (e.g., AE-RP2040).
   - Select **waveshare_rp2350_zero** or **weact_studio_rp2350b_core** for RP2350 boards.
3. Select the target build executable (e.g., `[AE-RP2040_EMUZ80]`) by clicking the **Build** target name in the status bar.
4. If you have just switched boards, run the `CMake: Configure` command or `Developer: Reload Window` from the Command Palette to ensure the environment is correctly updated.
5. Run the `/build` slash command to compile.
6. Run the `/flash` slash command to write the compiled `.uf2` file to your board.
   - If `/flash` does not work, put the board into BOOTSEL mode and **drag and drop** the `.uf2` file onto the board's drive in Explorer.

*(Note: Standard CMake/Ninja build flows should theoretically work, but are not officially supported or tested.)*

## Acknowledgments

This project utilizes the **ROM-BASIC (EMUBASIC)** from the [EMUZ80 Project](https://vintagechips.wordpress.com/2022/03/05/emuz80_reference/).

The included EMUBASIC is based on **Grant Searle's NASCOM BASIC**. We are deeply grateful to Grant Searle (http://searle.x10host.com/) for making this classic software accessible, and to vintagechips for their work on the EMUZ80 project and making EMUBASIC available.

## License

This project is released under the **MIT License**. See the `LICENSE` file for details.

**Important Exception (ROM-BASIC):**
The included ROM-BASIC (`EMUBASIC`) is based on the work of Grant Searle and the EMUZ80 project. 
- Grant Searle's adaptations are for **NON-COMMERCIAL USE ONLY**.
- The EMUZ80 materials are generally provided under **CC BY-NC-SA 3.0** or similar Non-Commercial terms.
Therefore, the ROM-BASIC code embedded within this project (`AE-RP2040_EMUZ80.c`) is strictly bound by these Non-Commercial restrictions and is **NOT** covered by the MIT License.

---

# AE-RP2040_EMUZ80 (日本語)

実物の Z80 CPU を RP2040 または RP2350 搭載ボードで動かすための周辺回路・バスエミュレータです。

ソフトウェアでCPU自体をエミュレーションするのではなく、RP2040/RP2350 の PIO (プログラマブル I/O) サブシステムを利用してメモリや I/O デバイスをエミュレートし、本物の Z80 マイクロプロセッサを動作させます。

## 対応ボード
- AE-RP2040_EMUZ80 (秋月電子 AE-RP2040)
- Waveshare RP2350-Zero_EMUZ80 (Waveshare RP2350-Zero)
- WeAct RP2350B CoreBoard_EMUZ80 (WeAct RP2350B CoreBoard)

## プロジェクト構成
各対応ボードのソースコードと設定ファイルは `boards/` ディレクトリ配下に分かれています。
- `boards/AE-RP2040/`: 秋月電子 AE-RP2040 用
- `boards/RP2350-Zero/`: Waveshare RP2350-Zero 用
- `boards/WeAct-RP2350B/`: WeAct RP2350B CoreBoard 用

### ビルド済み UF2 ファイル
すぐに書き込んで試せるUF2ファイルをプロジェクトルートに用意しています。
- `AE-RP2040_EMUZ80.uf2` — 秋月電子 AE-RP2040 用
- `RP2350-Zero_EMUZ80.uf2` — Waveshare RP2350-Zero 用
- `WeAct-RP2350B_EMUZ80.uf2` — WeAct RP2350B CoreBoard 用

## クロック設定

| ボード | RPクロック | Z80クロック |
|---|---|---|
| AE-RP2040 | 200 MHz | 2.5 MHz |
| RP2350-Zero | 150 MHz（標準） | 4 MHz |
| WeAct RP2350B CoreBoard | 150 MHz（標準） | 4 MHz |

## 通信端末ソフトの注意

RP2350 系ボードは Z80 が 4MHz で動作するため、TeraTerm 等の通信端末ソフトから文字を送る際は **送信遅延（文字遅延・行遅延）を設定しないと文字の取りこぼしや誤動作が発生する場合があります**。

## 既知の問題

- `/flash` スラッシュコマンドが正常にUF2ファイルを書き込めない場合があります。
  - **回避策**: ボードをBOOTSELモードにして、エクスプローラーからUF2ファイルを書き込み先ドライブへ**ドラッグ＆ドロップ**してください。

## ビルド要件
- Antigravity IDE (推奨ビルド環境)
- Raspberry Pi Pico C/C++ SDK
- CMake

## ビルド方法

現在、本プロジェクトのビルドおよび動作確認は **Antigravity IDE** 上でのみ行われています。

1. Antigravity IDE で本プロジェクトを開きます。
2. ステータスバーの右下にある **Pico SDK Board** をクリックして、使用するチップに合わせたボード構成を選択します。
   - RP2040 ボード（AE-RP2040 など）の場合は **pico** を選択。
   - RP2350 ボードの場合は **waveshare_rp2350_zero** または **weact_studio_rp2350b_core** を選択。
3. ステータスバーのターゲット名（例: `[AE-RP2040_EMUZ80]`）をクリックして、ビルド対象の実行ファイルを選択します。
4. ボード設定やターゲットを切り替えた直後は、必要に応じてコマンドパレットから `CMake: Configure` を実行、または `Developer: Reload Window` を行って環境を最新の状態に更新してください。
5. `/build` スラッシュコマンドを実行してコンパイルします。
6. `/flash` スラッシュコマンドを実行して、生成された `.uf2` ファイルをボードに書き込みます。
   - `/flash` が正常に動作しない場合は、BOOTSELモードでボードを接続し、エクスプローラーから `.uf2` ファイルを**ドラッグ＆ドロップ**してください。

*(注: 通常の CMake/Ninja を用いたビルドも理論上は可能ですが、公式にはサポート（テスト）していません)*

## 謝辞 (Acknowledgments)

本プロジェクトでは、[EMUZ80 プロジェクト](https://vintagechips.wordpress.com/2022/03/05/emuz80_reference/) の **ROM-BASIC (EMUBASIC)** を利用させていただいています。

含まれている EMUBASIC は、**Grant Searle 氏の NASCOM BASIC** をベースにしています。この歴史的な素晴らしいソフトウェアを利用可能にしてくださった Grant Searle 氏 (http://searle.x10host.com/) と、EMUZ80 プロジェクトを通じて EMUBASIC を提供・公開してくださった vintagechips 氏に深く感謝いたします。

## ライセンス (License)

このプロジェクト自体は **MIT ライセンス**の下で公開されています。詳細は `LICENSE` ファイルを確認してください。

**重要な例外 (ROM-BASIC について):**
ソースコード (`AE-RP2040_EMUZ80.c`) に含まれる ROM-BASIC (`EMUBASIC`) は、Grant Searle 氏および EMUZ80 プロジェクトの著作物に基づいています。
- Grant Searle 氏のコードは **「非商用利用 (NON-COMMERCIAL USE ONLY)」** に限定されています。
- EMUZ80 の関連資料は通常 **CC BY-NC-SA 3.0** 等の非商用ライセンスで提供されています。
従って、組み込まれている ROM-BASIC 部分のコードには上記の非商用制限が適用され、この部分については **MIT ライセンスの適用外** となりますのでご注意ください。
