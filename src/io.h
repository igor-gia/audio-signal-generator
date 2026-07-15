#pragma once
#include "Config.h"

enum IOEvent {
    IO_NONE = 0,
    
    // Короткие клики кнопок
    IO_BTN_BOTH_CLICK,   
    IO_BTN_LEFT_CLICK,   
    IO_BTN_RIGHT_CLICK,  
    IO_BTN_MODE_CLICK,   
    IO_BTN_WAVE_CLICK,   
    IO_BTN_ATTN_CLICK,   
    IO_ENC_BTN_CLICK,    
    
    // ДЛИННЫЕ удержания кнопок (Добавлено для Варианта 1)
    IO_BTN_BOTH_LONG,
    IO_BTN_LEFT_LONG,
    IO_BTN_RIGHT_LONG,
    IO_BTN_MODE_LONG,
    
    // Энкодер
    IO_ENC_ROT_LEFT,     
    IO_ENC_ROT_RIGHT     
};

struct ButtonFilter {
    bool lastSteadyState;
    bool lastDebounceState;
    unsigned long lastDebounceTime;
    unsigned long pressStartTime;   // Время начала нажатия
    bool longPressReported;         // Флаг, что длинное нажатие уже отправлено
};

void io_init();
IOEvent io_poll();