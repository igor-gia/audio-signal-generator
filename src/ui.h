#pragma once
#include "Config.h"
#include "io.h"

enum AppScreen {
    SCREEN_MAIN_MENU,
    SCREEN_WAVES,
    SCREEN_NOISES_MENU,
    SCREEN_WHITE_NOISE,
    SCREEN_PINK_NOISE,
    SCREEN_MP3_PLAYER,
    SCREEN_MP3_PLAYING,
    SCREEN_SETTINGS
};

// Инициализация графического движка дисплея
void ui_init();

// Главный метод конечного автомата. Принимает событие от железного модуля IO,
// изменяет глобальное состояние dev и перерисовывает экран.
void ui_update(IOEvent ev);
void ui_tick();