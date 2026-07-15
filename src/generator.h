#pragma once
#include <Arduino.h>

void gen_init();
void gen_deinit(); 
void gen_tick();

void gen_set_channels_frequency(float hz_L, float hz_R);
void gen_set_channels_wave(int wave_L, int wave_R);
void gen_set_submode(int mode_L, int mode_R);
void gen_set_phase(float degrees);
void gen_set_volume(int vol_db);
void gen_set_attenuation(int attn_db);
void gen_set_mute(bool mute);

float gen_get_current_frequency_L();
float gen_get_current_frequency_R();
float gen_get_phase();