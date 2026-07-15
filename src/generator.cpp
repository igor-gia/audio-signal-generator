#include "generator.h"
#include "Config.h"
#include <driver/i2s_std.h>
#include <math.h>
#include <Arduino.h>
#include "driver/periph_ctrl.h"

// Очищаем дефайны под конкретную архитектуру чипа
#if !defined(PERIPH_I2S0_MODULE) && defined(PERIPH_I2S_MODULE)
  #define PERIPH_I2S0_MODULE PERIPH_I2S_MODULE
#endif

static const int SAMPLE_RATE = 192000;
static const int SINE_TABLE_SIZE = 4096;

static const float DB_TO_GAIN_LUT[] = {
  1.0000f, 0.8913f, 0.7943f, 0.7079f, 0.6310f, 0.5623f, 0.5012f, 0.4467f, 0.3981f, 0.3548f,
  0.3162f, 0.2818f, 0.2512f, 0.2239f, 0.1995f, 0.1778f, 0.1585f, 0.1413f, 0.1259f, 0.1122f,
  0.1000f, 0.0891f, 0.0794f, 0.0708f, 0.0631f, 0.0562f, 0.0501f, 0.0447f, 0.0398f, 0.0355f,
  0.0316f, 0.0282f, 0.0251f, 0.0224f, 0.0199f, 0.0178f, 0.0158f, 0.0141f, 0.0126f, 0.0112f,
  0.0100f, 0.0089f, 0.0079f, 0.0071f, 0.0063f, 0.0056f, 0.0050f, 0.0045f, 0.0040f, 0.0035f,
  0.0032f, 0.0028f, 0.0025f, 0.0022f, 0.0020f, 0.0018f, 0.0016f, 0.0014f, 0.0013f, 0.0011f,
  0.0010f
};

static volatile float gen_freq_L = 1000.0f;
static volatile float gen_freq_R = 1000.0f;
static volatile int gen_wave_L = 0;
static volatile int gen_wave_R = 0;
static volatile int gen_submode_L = 0;
static volatile int gen_submode_R = 0;

static volatile float gen_phase_deg = 0.0f;
static volatile int gen_vol_db = 0;
static volatile int gen_attn_db = 0;
static volatile bool gen_mute = false;

static unsigned long sweep_start_L = 0;
static unsigned long sweep_start_R = 0;
static unsigned long phase_rot_start_time = 0;

static int16_t sine_table[SINE_TABLE_SIZE];
static i2s_chan_handle_t tx_handle = NULL;
static TaskHandle_t ddsTaskHandle = NULL;
static volatile bool gen_running = false;

static float pink_b0 = 0.0f;
static float pink_b1 = 0.0f;
static float pink_b2 = 0.0f;
static uint32_t lcg_seed = 123456789U;

static inline int32_t fast_random_lcg() {
  lcg_seed = lcg_seed * 1664525U + 1013904223U;
  int32_t raw = (int32_t)(lcg_seed >> 16);
  return ((raw * 50000) / 65535) - 25000;
}

static void init_sine_table() {
  for (int i = 0; i < SINE_TABLE_SIZE; i++) {
    sine_table[i] = (int16_t)(25000.0 * sinf((2.0 * M_PI * i) / SINE_TABLE_SIZE));
  }
}

static int16_t generate_pink_noise_fast() {
  float white = ((float)fast_random_lcg()) / 25000.0f;
#if (SAMPLE_RATE == 192000)
  pink_b0 = 0.999738f * pink_b0 + white * 0.012845f;
  pink_b1 = 0.998464f * pink_b1 + white * 0.017372f;
  pink_b2 = 0.992851f * pink_b2 + white * 0.035616f;
#else
  pink_b0 = 0.99886f * pink_b0 + white * 0.0555179f;
  pink_b1 = 0.99332f * pink_b1 + white * 0.0750759f;
  pink_b2 = 0.96900f * pink_b2 + white * 0.1538520f;
#endif
  float pink = pink_b0 + pink_b1 + pink_b2 + white * 0.125f;
  pink *= 0.25f;
  return (int16_t)(pink * 25000.0f);
}

static void dds_audio_task(void *pvParameters) {
  uint32_t phase_acc_L = 0;
  uint32_t phase_acc_R = 0;
  const size_t buffer_samples = 128;
  int16_t i2s_buffer[buffer_samples * 2];
  size_t bytes_written = 0;

  lcg_seed = analogRead(0) ^ 0xAAAAAAAA;

  while (gen_running) {
    float local_freq_L = gen_freq_L;
    float local_freq_R = gen_freq_R;
    int local_wave_L = gen_wave_L;
    int local_wave_R = gen_wave_R;
    int local_submode_L = gen_submode_L;
    int local_submode_R = gen_submode_R;
    float local_phase = gen_phase_deg;

    int vol_idx = abs(gen_vol_db);
    if (vol_idx > 60) vol_idx = 60;
    float vol_gain = DB_TO_GAIN_LUT[vol_idx];

    bool is_attn = (gen_attn_db == -20);
    bool is_mute = gen_mute;

    uint32_t phase_step_L = (uint32_t)((local_freq_L * SINE_TABLE_SIZE / SAMPLE_RATE) * 65536.0);
    uint32_t phase_step_R = (uint32_t)((local_freq_R * SINE_TABLE_SIZE / SAMPLE_RATE) * 65536.0);
    uint32_t phase_offset_R = (uint32_t)((local_phase * 4294967296.0) / 360.0);
    int32_t sqr_max_amplitude = (int32_t)(25000.0f * vol_gain);

    for (size_t i = 0; i < buffer_samples; i++) {
      int32_t sample_L = 0;
      int32_t sample_R = 0;

      if (!is_mute) {
        if (local_submode_L == SUBMODE_WHITE_NOISE) {
          sample_L = fast_random_lcg() * vol_gain;
          sample_R = sample_L;
          phase_acc_L += phase_step_L;
          phase_acc_R += phase_step_R;
        } else if (local_submode_L == SUBMODE_PINK_NOISE) {
          sample_L = generate_pink_noise_fast() * vol_gain;
          sample_R = sample_L;
          phase_acc_L += phase_step_L;
          phase_acc_R += phase_step_R;
        } else if (local_submode_L == SUBMODE_IMD) {
          uint32_t step_imd1 = (uint32_t)((cfg.imdFreq1 * SINE_TABLE_SIZE / SAMPLE_RATE) * 65536.0);
          uint32_t step_imd2 = (uint32_t)((cfg.imdFreq2 * SINE_TABLE_SIZE / SAMPLE_RATE) * 65536.0);
          static uint32_t phase_acc_imd2 = 0;

          uint16_t idx_imd1 = (phase_acc_L >> 16) % SINE_TABLE_SIZE;
          uint16_t idx_imd2 = (phase_acc_imd2 >> 16) % SINE_TABLE_SIZE;

          int32_t imd_signal = (sine_table[idx_imd1] * cfg.imdRatio1) + (sine_table[idx_imd2] * cfg.imdRatio2);
          sample_L = (int32_t)(imd_signal * vol_gain);
          sample_R = sample_L;

          phase_acc_L += step_imd1;
          phase_acc_imd2 += step_imd2;
        } else {
          uint16_t index_L = (phase_acc_L >> 16) % SINE_TABLE_SIZE;
          uint16_t index_R = ((phase_acc_R + phase_offset_R) >> 16) % SINE_TABLE_SIZE;

          if (local_wave_L == WAVE_SIN) {
            sample_L = (int32_t)(sine_table[index_L] * vol_gain);
          } else {
            sample_L = (index_L < SINE_TABLE_SIZE / 2) ? sqr_max_amplitude : -sqr_max_amplitude;
          }

          if (local_wave_R == WAVE_SIN) {
            sample_R = (int32_t)(sine_table[index_R] * vol_gain);
          } else {
            sample_R = (index_R < SINE_TABLE_SIZE / 2) ? sqr_max_amplitude : -sqr_max_amplitude;
          }

          phase_acc_L += phase_step_L;
          phase_acc_R += phase_step_R;
        }

        if (is_attn) {
          sample_L /= 10;
          sample_R /= 10;
        }
      } else {
        phase_acc_L += phase_step_L;
        phase_acc_R += phase_step_R;
      }

      i2s_buffer[i * 2] = (int16_t)sample_L;
      i2s_buffer[i * 2 + 1] = (int16_t)sample_R;
    }

    i2s_channel_write(tx_handle, i2s_buffer, sizeof(i2s_buffer), &bytes_written, portMAX_DELAY);
  }

  i2s_channel_disable(tx_handle);
  i2s_del_channel(tx_handle);
  tx_handle = NULL;
  ddsTaskHandle = NULL;
  vTaskDelete(NULL);
}

void gen_init() {
  if (gen_running) return;

  init_sine_table();

  i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
  
  esp_err_t err = i2s_new_channel(&chan_cfg, &tx_handle, NULL);
  if (err == ESP_OK) {
    i2s_std_config_t std_cfg = {
      .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(SAMPLE_RATE),
      .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO),
      .gpio_cfg = {
        .mclk = I2S_GPIO_UNUSED,
        .bclk = (gpio_num_t)BCLK_PIN,
        .ws = (gpio_num_t)LRCK_PIN,
        .dout = (gpio_num_t)DIN_PIN,
        .din = I2S_GPIO_UNUSED,
        .invert_flags = { .mclk_inv = false, .bclk_inv = false, .ws_inv = false } }
    };
    i2s_channel_init_std_mode(tx_handle, &std_cfg);
    i2s_channel_enable(tx_handle);
    
    gen_running = true;
    xTaskCreatePinnedToCore(dds_audio_task, "DDS_Task", 4096, NULL, 5, &ddsTaskHandle, 1);
  } else {
    Serial.printf("Generator I2S Init Error: %d\n", err);
  }
}

void gen_deinit() {
  if (!gen_running) return;
  gen_running = false;
  
  while (ddsTaskHandle != NULL) {
    delay(2);
  }
}

void gen_set_channels_frequency(float hz_L, float hz_R) { gen_freq_L = hz_L; gen_freq_R = hz_R; }
void gen_set_channels_wave(int wave_L, int wave_R) { gen_wave_L = wave_L; gen_wave_R = wave_R; }
void gen_set_submode(int mode_L, int mode_R) { gen_submode_L = mode_L; gen_submode_R = mode_R; }
void gen_set_phase(float degrees) { gen_phase_deg = degrees; }
void gen_set_volume(int vol_db) { gen_vol_db = vol_db; }
void gen_set_attenuation(int attn_db) { gen_attn_db = attn_db; }
void gen_set_mute(bool mute) { gen_mute = mute; }
float gen_get_current_frequency_L() { return gen_freq_L; }
float gen_get_current_frequency_R() { return gen_freq_R; }
float gen_get_phase() { return gen_phase_deg; }

void gen_tick() {
  static unsigned long last_update = 0;
  if (millis() - last_update < 20) return;
  last_update = millis();

  unsigned long duration_ms = cfg.sweepDuration * 1000;

  if (gen_submode_L == SUBMODE_SWEEP) {
    if (sweep_start_L == 0) sweep_start_L = millis();
    unsigned long elapsed = millis() - sweep_start_L;
    if (elapsed >= duration_ms) { sweep_start_L = millis(); elapsed = 0; }
    float t = (float)elapsed / duration_ms;
    gen_freq_L = cfg.sweepStartF * powf((cfg.sweepStopF / cfg.sweepStartF), t);
  } else { sweep_start_L = 0; }

  if (gen_submode_R == SUBMODE_SWEEP) {
    if (sweep_start_R == 0) sweep_start_R = millis();
    unsigned long elapsed = millis() - sweep_start_R;
    if (elapsed >= duration_ms) { sweep_start_R = millis(); elapsed = 0; }
    float t = (float)elapsed / duration_ms;
    gen_freq_R = cfg.sweepStartF * powf((cfg.sweepStopF / cfg.sweepStartF), t);
  } else { sweep_start_R = 0; }

  if (gen_submode_R == SUBMODE_PHASE_ROT) {
    if (phase_rot_start_time == 0) phase_rot_start_time = millis();
    unsigned long elapsed = millis() - phase_rot_start_time;
    unsigned long rot_duration_ms = cfg.phaseRotDuration * 1000;
    if (elapsed >= rot_duration_ms) { phase_rot_start_time = millis(); elapsed = 0; }
    gen_phase_deg = 360.0f * ((float)elapsed / rot_duration_ms);
    if (gen_phase_deg >= 360.0f) gen_phase_deg = 0.0f;
    dev.phaseAngle = (int)gen_phase_deg;
  } else { phase_rot_start_time = 0; }
}