#include "io.h"
#include "player.h"

const unsigned long DEBOUNCE_DELAY = 25; 
const unsigned long LONG_PRESS_TIME = 500; // Поллимона миллисекунд (0.5 сек) для активации курсора

const int BUTTON_PINS[] = { PIN_BTN_1, PIN_BTN_2, PIN_BTN_3, PIN_BTN_4, PIN_BTN_5, PIN_ENC_BTN };
const int NUM_BUTTONS = sizeof(BUTTON_PINS) / sizeof(BUTTON_PINS[0]);

const IOEvent BUTTON_EVENTS[] = {
    IO_BTN_BOTH_CLICK, IO_BTN_LEFT_CLICK, IO_BTN_RIGHT_CLICK,
    IO_BTN_MODE_CLICK, IO_BTN_WAVE_CLICK, IO_ENC_BTN_CLICK
};

// Соответствующие события для длинного нажатия 
const IOEvent BUTTON_LONG_EVENTS[] = {
    IO_BTN_BOTH_LONG, IO_BTN_LEFT_LONG, IO_BTN_RIGHT_LONG,
    IO_BTN_MODE_LONG, 
    IO_NONE, IO_NONE, IO_NONE
};

ButtonFilter buttons[NUM_BUTTONS];
static uint8_t encOldState = 0;

void io_init() {
    for (int i = 0; i < NUM_BUTTONS; i++) {
        pinMode(BUTTON_PINS[i], INPUT_PULLUP);
        buttons[i].lastSteadyState = true;
        buttons[i].lastDebounceState = true;
        buttons[i].lastDebounceTime = 0;
        buttons[i].pressStartTime = 0;
        buttons[i].longPressReported = false;
    }
    pinMode(PIN_ENC_A, INPUT_PULLUP);
    pinMode(PIN_ENC_B, INPUT_PULLUP);
    encOldState = (digitalRead(PIN_ENC_A) << 1) | digitalRead(PIN_ENC_B);
}

IOEvent io_poll() {
    unsigned long currentTime = millis();
    IOEvent triggeredEvent = IO_NONE;

    // 1. Опрос кнопок с поддержкой клика и удержания
    for (int i = 0; i < NUM_BUTTONS; i++) {
        bool rawState = digitalRead(BUTTON_PINS[i]);
        
        // Индивидуальный фильтр для кнопки энкодера (чтобы не чиркала Mute при вращении)
        unsigned long currentDelay = (BUTTON_PINS[i] == PIN_ENC_BTN) ? 90 : DEBOUNCE_DELAY;

        if (rawState != buttons[i].lastDebounceState) {
            buttons[i].lastDebounceTime = currentTime;
            buttons[i].lastDebounceState = rawState;
        }

        if ((currentTime - buttons[i].lastDebounceTime) > currentDelay) {
            if (rawState != buttons[i].lastSteadyState) {
                buttons[i].lastSteadyState = rawState;

                if (buttons[i].lastSteadyState == LOW) {
                    // Кнопка только что нажата
                    buttons[i].pressStartTime = currentTime;
                    buttons[i].longPressReported = false;
                } else {
                    // Кнопка отпущена. Если это была кнопка БЕЗ длинного нажатия 
                    // или длинное нажатие еще не успело сработать — выдаем обычный клик
                    if (!buttons[i].longPressReported) {
                        if (currentTime - buttons[i].pressStartTime > currentDelay) {
                            triggeredEvent = BUTTON_EVENTS[i];
                        }
                    }
                }
            }
            
            // Фиксация ДЛИННОГО нажатия в процессе удержания (не дожидаясь отпускания)
            if (buttons[i].lastSteadyState == LOW && !buttons[i].longPressReported) {
                if (currentTime - buttons[i].pressStartTime > LONG_PRESS_TIME) {
                    buttons[i].longPressReported = true;
                    if (BUTTON_LONG_EVENTS[i] != IO_NONE) {
                        triggeredEvent = BUTTON_LONG_EVENTS[i];
                    }
                }
            }
        }
    }

    if (triggeredEvent != IO_NONE) return triggeredEvent; 

    // 2. Полношаговый энкодер
    uint8_t currentEncState = (digitalRead(PIN_ENC_A) << 1) | digitalRead(PIN_ENC_B);
    if (currentEncState != encOldState) {
        uint8_t transition = (encOldState << 2) | currentEncState;
        static int8_t internalDelta = 0;

        switch (transition) {
            case 0b0001: case 0b0111: case 0b1110: case 0b1000: internalDelta++; break;
            case 0b0010: case 0b0100: case 0b1101: case 0b1011: internalDelta--; break;
        }
        encOldState = currentEncState;

        if (internalDelta >= 4) {
            internalDelta = 0;
            return IO_ENC_ROT_RIGHT;
        } else if (internalDelta <= -4) {
            internalDelta = 0;
            return IO_ENC_ROT_LEFT;
        }
    }

    return IO_NONE;
}