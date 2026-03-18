// Waveshare RP2350-Zero_EMUZ80: Z80 bus emulator for Waveshare RP2350-Zero
#include "RP2350-Zero.pio.h"
#include "hardware/clocks.h"
#include "hardware/pio.h"
#include "hardware/pwm.h"
#include "hardware/sync.h"
#include "hardware/vreg.h"
#include "pico/multicore.h"
#include "pico/stdlib.h"
#include "tusb.h" // TinyUSBのヘッダーを追加
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// GPIO Pin Definitions
#define DATA_BASE 0  // GP0..7: Data Bus
#define ADRS_BASE 8  // GP8..20,GP26..29: Address Bus A0-12,A13-15(16-bit)
#define RD_PIN 21    // GP21: RD
#define WR_PIN 22    // GP22: WR
#define MREQ_PIN 23  // GP23: MREQ
#define IORQ_PIN 24  // GP24: IORQ
#define RESET_PIN 25 // GP25: RESET
#define CLK_PIN 26   // GP26: CLK
#define A13_PIN 27   // GP27: A13
#define A14_PIN 28   // GP28: A14
#define A15_PIN 29   // GP29: A15

#define MEMORY_SIZE 65536 // 64KB

static uint8_t memory[MEMORY_SIZE];
volatile bool stop_flg = false;
volatile uint8_t uart_rxdata = 0;
volatile uint8_t uart_txdata = 0;
volatile uint8_t uart_stat = 2;

// Test Program (from Python testprg2)
const uint8_t testprg2[] = {0x21, 0x00, 0x00,  // LD HL, 0000
                            0x22, 0x00, 0x80,  // LD (8000), HL
                            0x23,              // INC HL
                            0xC3, 0x03, 0x00}; // JP 0003

// Test Program ROM-BASIC (EMUZ80のEMUBASIC)
// ROM-BASIC (Extracted to shared header)
#include "../../EMUBASIC.h"

// --- Helper: Manual Clock Pulse ---
static void clk_on_off(int n) {
  gpio_set_function(CLK_PIN, GPIO_FUNC_SIO);
  gpio_set_dir(CLK_PIN, GPIO_OUT);
  for (int i = 0; i < n; i++) {
    gpio_put(CLK_PIN, 1);
    sleep_ms(10); // 短いパルス
    gpio_put(CLK_PIN, 0);
    sleep_ms(10);
  }
}

// --- Helper: Delayed RESET OFF ---
static int64_t reset_off_callback(alarm_id_t id, void *user_data) {
  printf("RESET-OFF (Delayed 1s)\n");
  gpio_put(RESET_PIN, 1); // RESET-OFF (High)
  return 0;               // ONE_SHOT
}

// --- Helper: Set PWM Frequency in Hz ---
void set_pwm_freq(uint pin, uint32_t freq) {
  uint slice_num = pwm_gpio_to_slice_num(pin);
  uint32_t sys_clk = clock_get_hz(clk_sys);

  // 最適な clkdiv を探す (最大 255.9375)
  float clk_div = 1.0f;
  uint32_t wrap = (sys_clk / (freq * clk_div)) - 1;

  while (wrap > 65535 && clk_div < 256.0f) {
    clk_div += 1.0f;
    wrap = (sys_clk / (freq * clk_div)) - 1;
  }

  if (clk_div > 255.0f)
    clk_div = 255.0f;
  if (wrap > 65535)
    wrap = 65535;

  pwm_set_clkdiv(slice_num, clk_div);
  pwm_set_wrap(slice_num, wrap);
  pwm_set_gpio_level(pin, (wrap + 1) / 2);
}

// PIO0
uint sm_emu = 1;
uint sm_dirsL = 2;
uint sm_dirsH = 3;

// PIO1
// uint sm_trg2 = 0;
uint sm_trg_RD = 0;
uint sm_trg_WR = 1;

// --- PIO Helpers ---
void pio_init_bus() {
  PIO pio = pio0;
  PIO pio_1 = pio1;

  // GPIOをPIO用に初期化 GP0-7(D0-7 8bit), GP8-20(A0-12),GP27-29(A13-15)
  //     GP21(RD), GP22(WR), GP23(MREQ), GP24(IORQ), GP25(RESET), GP26(CLK)
  for (int i = 0; i <= 20; i++) {
    pio_gpio_init(pio, i);
  }

  pio_gpio_init(pio_1, RD_PIN); // GP21(RD)
  pio_gpio_init(pio_1, WR_PIN); // GP22(WR)
  pio_gpio_init(pio, A13_PIN);  // GP27(A13)
  pio_gpio_init(pio, A14_PIN);  // GP28(A14)
  pio_gpio_init(pio, A15_PIN);  // GP29(A15)

  // PIO1 - SM0/1: trg_pin (Detection of falling edge on RD/WR)
  uint offset_trg = pio_add_program(pio_1, &trg_pin_program);
  pio_sm_config c_trg_RD = trg_pin_program_get_default_config(offset_trg);
  pio_sm_config c_trg_WR = trg_pin_program_get_default_config(offset_trg);

  // PIO0 - SM0: m_emu (Address/Data handling)
  uint offset_emu = pio_add_program(pio, &m_emu_program);
  pio_sm_config c_emu = m_emu_program_get_default_config(offset_emu);

  // PIO0 - SM1/2: d_pindirs (Direction toggle)
  uint offset_dirs = pio_add_program(pio, &d_pindirs_program);
  pio_sm_config c_dirsL = d_pindirs_program_get_default_config(offset_dirs);
  pio_sm_config c_dirsH = d_pindirs_program_get_default_config(offset_dirs);

  // PIO1 SM0/1: trg_pin (Detection of falling edge on RD/WR)
  sm_config_set_in_pins(&c_trg_RD, RD_PIN); // base = GP21 (RD)
  pio_sm_init(pio_1, sm_trg_RD, offset_trg, &c_trg_RD);
  pio_sm_set_enabled(pio_1, sm_trg_RD, true);

  sm_config_set_in_pins(&c_trg_WR, WR_PIN); // base = GP22 (WR)
  pio_sm_init(pio_1, sm_trg_WR, offset_trg, &c_trg_WR);
  pio_sm_set_enabled(pio_1, sm_trg_WR, true);

  // PIO0 - SM0: m_emu (Address/Data handling)
  sm_config_set_in_pins(&c_emu, 0);      // base = GP0
  sm_config_set_out_pins(&c_emu, 0, 8);  // base = GP0, cnt = 8
  sm_config_set_jmp_pin(&c_emu, RD_PIN); // GP21 (RD)

  pio_sm_set_consecutive_pindirs(pio, sm_emu, 0, 8,
                                 false); // D0-7ピン初期化(入力)

  // シフトレジスタの設定 (Auto Push/Pull 有効化)
  sm_config_set_in_shift(&c_emu, false, true,
                         30); // ISRのシフト方向, auto_push=true, threshold=30
  sm_config_set_out_shift(&c_emu, true, true,
                          8); // OSRのシフト方向, auto_pull=true, threshold=8

  pio_sm_init(pio, sm_emu, offset_emu, &c_emu);
  pio_sm_set_enabled(pio, sm_emu, true);

  // PIO0 - SM1/2: d_pindirs (Direction toggle)
  sm_config_set_set_pins(&c_dirsL, DATA_BASE, 4); // GP0..3
  sm_config_set_jmp_pin(&c_dirsL, RD_PIN);        // GP21 (RD)
  pio_sm_init(pio, sm_dirsL, offset_dirs, &c_dirsL);
  pio_sm_set_enabled(pio, sm_dirsL, true);

  sm_config_set_set_pins(&c_dirsH, DATA_BASE + 4, 4); // GP4..7
  sm_config_set_jmp_pin(&c_dirsH, RD_PIN);            // GP21 (RD)
  pio_sm_init(pio, sm_dirsH, offset_dirs, &c_dirsH);
  pio_sm_set_enabled(pio, sm_dirsH, true);
}

// --- UART Task (Core 0) ---
void task1(void) {
  printf("task1 UART start..\n");
  while (true) {
    // 送信処理 (Z80 -> USB)
    if ((uart_stat & 0x02) == 0) {
      if (tud_cdc_connected() && tud_cdc_write_available() > 0) {
        putchar(uart_txdata);
        uart_stat |= 0x02; // TX Buffer Empty
      }
    }

    // 受信処理 (USB -> Z80)
    // RX Readyが0（空）の場合のみ入力をチェック
    if (!(uart_stat & 0x01)) {
      int c = getchar_timeout_us(0);
      if (c != PICO_ERROR_TIMEOUT) {
        if (c == 0x04) { // Ctrl-D: Stop emulation
          printf("\ntask1: Ctrl-D detected. Stopping..\n");
          stop_flg = true;
          break;
        }
        uart_rxdata = (uint8_t)c;
        uart_stat |= 0x01; // RX Data Available
      }
    }
    sleep_ms(1);
  }
}

// --- Main Emulation Loop パターン1 ---
//__attribute__((noinline)) void __time_critical_func(emu_loop1)(void) {
void __time_critical_func(emu_loop1)(void) {
  // レジスタアドレスをローカルにキャッシュ
  // rxfifo は読み取り専用、txfifo は書き込み専用の型を使用する
  io_ro_32 *const rxfifo = &pio0->rxf[sm_emu];
  io_wo_32 *const txfifo = &pio0->txf[sm_emu];
  const uint32_t mreq_mask = (1u << MREQ_PIN);
  const uint32_t wr_mask = (1u << WR_PIN);
  PIO pio = pio0;
  uint clk_cnt = 0;
  uint8_t data_byte = 0;

  //  while (!stop_flg) {
  while (true) {
    // バスライン取得
    // GP0-29(D0..8=GP0-7,A0..12=GP8-20,RD=GP21,WR=GP22,MREQ=GP23,IORQ=GP24,A13..15=GP27-29)
    // SDK関数を介さず直接FIFO待ち＆読み込み
    while (pio0->fstat & (1u << (PIO_FSTAT_RXEMPTY_LSB + sm_emu))) {
    }
    uint32_t agpio = *rxfifo;
    uint32_t adrs_word = ((agpio >> 8) & 0x1FFF) | ((agpio >> 14) & 0xE000);
    clk_cnt++;
    if (!(agpio & mreq_mask)) { // MREQ=0
      if (!(agpio & wr_mask)) { // MREQ=0 WR=0
        data_byte = (uint8_t)agpio;
        if (adrs_word == 0xE000) { // UART TX data
          uart_txdata = data_byte;
          uart_stat = uart_stat & 0xFD; // b2=0: TX busy
        } else if (adrs_word >= 0x8000) {
          memory[adrs_word] = data_byte;
        }
      } else { // MREQ=0, RD=0
        if (adrs_word == 0xE001) {
          data_byte = uart_stat;
        } else if (adrs_word == 0xE000) {
          data_byte = uart_rxdata;
          uart_stat &= 0xFE; // RX Data Empty (Clear bit 0)
        } else {
          data_byte = memory[adrs_word];
        }
        *txfifo = data_byte;
      }
    } else { // IORQ=0
      uint ioadrs = adrs_word & 0xFF;
      if (!(agpio & wr_mask)) { // IORQ=0, WR=0
        data_byte = (uint8_t)agpio;
      } else { // IORQ=0, RD=0
        data_byte = (uint8_t)agpio;
        *txfifo = data_byte;
      }
    }
    /*
    if (false) { // デバッグ用バスモニタ Z80_freq = 20　(20Hz) で使用する
      printf("%05d IORQ:%d MREQ:%d WR:%d RD:%d ADRS:%04X DATA:%02X\n", clk_cnt,
             (agpio >> IORQ_PIN) & 1, (agpio >> MREQ_PIN) & 1,
             (agpio >> WR_PIN) & 1, (agpio >> RD_PIN) & 1, adrs_word,
             (int)data_byte);
    }
    */
  }
}

// --- Main Emulation Loop パターン2 ---
__attribute__((noinline)) void __time_critical_func(emu_loop2)(void) {
  const uint32_t mreq_mask = (1u << MREQ_PIN);
  const uint32_t wr_mask = (1u << WR_PIN);
  PIO pio = pio0;
  uint clk_cnt = 0;
  uint8_t data_byte = 0;
  while (true) {
    // バスライン取得
    // GP0-29(D0..8=GP0-7,A0..12=GP8-20,RD=GP21,WR=GP22,MREQ=GP23,IORQ=GP24,A13..15=GP27-29)
    uint32_t agpio = pio_sm_get_blocking(pio, sm_emu);
    uint32_t adrs_word = ((agpio >> 8) & 0x1FFF) | ((agpio >> 14) & 0xE000);

    clk_cnt++;
    if (!(agpio & mreq_mask)) { // MREQ=0
      if (!(agpio & wr_mask)) { // MREQ=0, WR=0
        data_byte = (uint8_t)agpio;
        if (adrs_word == 0xE000) { // UART TX data
          uart_txdata = data_byte;
          uart_stat = uart_stat & 0xFD; // b2=0: TX busy
          //        } else if ((adrs_word >= 0x8000) && (adrs_word <= 0x8FFF)) {
        } else if (adrs_word >= 0x8000) {
          memory[adrs_word] = data_byte;
        }
      } else { // MREQ=0, RD=0
        if (adrs_word == 0xE001) {
          data_byte = uart_stat;
        } else if (adrs_word == 0xE000) {
          data_byte = uart_rxdata;
          uart_stat &= 0xFE; // RX Data Empty (Clear bit 0)
        } else {
          data_byte = memory[adrs_word];
        }
        pio_sm_put_blocking(pio, sm_emu, data_byte);
      }
    } else { // IORQ=0
      uint ioadrs = adrs_word & 0xFF;
      if (!(agpio & wr_mask)) { // IORQ=0, WR=0
        data_byte = (uint8_t)agpio;
      } else { // IORQ=0, RD=0
        data_byte = (uint8_t)agpio;
        pio_sm_put_blocking(pio, sm_emu, data_byte);
      }
    }
    if (false) { // デバッグ用バスモニタ Z80_freq = 20　(20Hz) で使用する
      printf("%05d IORQ:%d MREQ:%d WR:%d RD:%d ADRS:%04X DATA:%02X\n", clk_cnt,
             (agpio >> IORQ_PIN) & 1, (agpio >> MREQ_PIN) & 1,
             (agpio >> WR_PIN) & 1, (agpio >> RD_PIN) & 1, adrs_word,
             (int)data_byte);
    }
  }
}

// QSPIクロックを調整する関数
void set_qspi_clock_divider(uint32_t sys_clock_khz, uint32_t qspi_max_khz) {
  uint32_t divider = (sys_clock_khz + qspi_max_khz - 1) / qspi_max_khz;
  clock_configure(clk_peri, 0, CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS,
                  sys_clock_khz * 1000, sys_clock_khz * 1000 / divider);
}

//
// main()関数
//
int main() {

  sleep_ms(100);
  uint32_t sysclk = clock_get_hz(clk_sys);
  if (false) {
    sysclk = 360 * 1000; // Pico2 システムクロック 360MHz
    //  sysclk = 400 * 1000;                    // Pico2 システムクロック 400MHz
    vreg_set_voltage(VREG_VOLTAGE_1_30);    // 電圧を1.3Vに設定
    sleep_ms(100);                          // 電圧安定のための待機
    set_sys_clock_khz(sysclk, true);        // 高速動作
    set_qspi_clock_divider(sysclk, 133000); // QSPIクロックを133MHz以下に
    sleep_ms(100);
  }
  stdio_init_all();
  setbuf(stdout, NULL); // 標準出力のバッファリングを無効化
  sleep_ms(100);

  // Z80用メモリー初期化
  memset(memory, 0xFF, MEMORY_SIZE);
  memcpy(memory, rom_basic, sizeof(rom_basic));

  // GPIO初期化
  gpio_init_mask(0x0FFFFFFF);
  for (int i = 0; i <= 24; i++) {
    gpio_init(i);
    gpio_set_dir(i, GPIO_IN);
    //   gpio_pull_up(i);
  }

  // 他の制御ピン  RESET:GP25 CLK:GP26
  gpio_init(RESET_PIN);
  gpio_set_dir(RESET_PIN, GPIO_OUT);
  gpio_put(RESET_PIN, 0); // RESET ON

  uint other_pins[] = {A13_PIN, A14_PIN, A15_PIN};
  for (int i = 0; i < 3; i++) {
    gpio_init(other_pins[i]);
    gpio_set_dir(other_pins[i], GPIO_IN);
    //  gpio_put(other_pins[i], 0);
  }
  sleep_ms(100);

  // Initial CLK pulses (Python: CLK_OnOff(10))
  clk_on_off(10);
  // PIO初期化
  pio_init_bus();

  sleep_ms(1000);
  printf("\n\n-hit [Enter] in terminal-\n");
  while (getchar_timeout_us(100) == PICO_ERROR_TIMEOUT)
    ;

  // printf("Waveshare RP2350-Zero Core:1.3V Clock:360MHz\n");
  // printf("Waveshare RP2350-Zero Core:1.3V Clock:400MHz\n");
  // printf("WeAct RP2350B-CoreBoard\n");
  printf("Waveshare RP2350-Zero\n");
  printf("Emulation パターン2 task(core1) Start..\n");
  //  エミュレーション開始(core1)
  multicore_launch_core1(emu_loop2);
  sleep_ms(1000);

  // CLK PWM Setup, RP2350-Zero 400MHz Z80 11MHz, 150MHz 5MHz
  // int Z80_freq = 12000000;
  // int Z80_freq = 11000000;
  // int Z80_freq = 10000000;
  // int Z80_freq = 8000000;
  // int Z80_freq = 6000000;
  // int Z80_freq = 5000000;
  int Z80_freq = 4000000;
  // int Z80_freq = 2500000;
  // int Z80_freq = 20;
  gpio_set_function(CLK_PIN, GPIO_FUNC_PWM);
  uint slice_num = pwm_gpio_to_slice_num(CLK_PIN);
  set_pwm_freq(CLK_PIN, Z80_freq);
  printf("Z80 CLK ON %fMHz\n", Z80_freq / 1000000.0);
  pwm_set_enabled(slice_num, true);

  // 1秒後にRESETを解除 (Python 439行目相当)
  add_alarm_in_ms(1000, reset_off_callback, NULL, false);
  printf("main task(Core0) start..\n");
  task1();

  // Cleanup (Python lines 442-445)
  gpio_put(RESET_PIN, 0);
  printf("RESET-ON\n");
  sleep_ms(100);
  pwm_set_enabled(slice_num, false);
  clk_on_off(10);
  printf("Exited.\n");

  return 0;
}
