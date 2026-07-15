#pragma once
#include <Arduino.h>

// =============================================================================
// 1. АППАРАТНАЯ КОНФИГУРАЦИЯ (ПИНЫ GPIO)
// =============================================================================
// --- Интерфейс I2S (Звуковой ЦАП PCM5102A) ---
const int BCLK_PIN = 2;
const int LRCK_PIN = 41;
const int DIN_PIN  = 42;

// --- Интерфейс I2C (Дисплей SSD1306) ---
const int SDA_PIN  = 40;
const int SCL_PIN  = 39;

// --- Интерфейс SD-Card ---
const int SD_MMC_D0  = 48;
const int SD_MMC_CLK = 47;
const int SD_MMC_CMD = 21;

// --- Квадратурный энкодер ---
const int PIN_ENC_A    = 7;    // Enc+ (CLK)
const int PIN_ENC_B    = 15;   // Enc- (DT)
const int PIN_ENC_BTN  = 6;    // Enc Btn (Mute / Unmute)

// --- Линейка тактовых кнопок ---
const int PIN_BTN_1    = 16;  // BOTH CH
const int PIN_BTN_2    = 13;  // LEFT CH
const int PIN_BTN_3    = 5;   // RIGHT CH
const int PIN_BTN_4    = 14;  // MODE (STATIC -> SWEEP -> ...)
const int PIN_BTN_5    = 4;   // WAVE (SIN -> SQR)

// =============================================================================
// 2. ИНЖЕНЕРНЫЕ И КОНСТРУКТОРСКИЕ КОНСТАНТЫ
// =============================================================================
const int PHASE_STEP       = 10;    // Шаг изменения фазы в градусах
const double FREQ_MIN      = 0.1;   // Минимальная частота генерации, Гц
const double FREQ_MAX      = 22000.0; // Максимальная частота генерации, Гц
const unsigned long FOCUS_TIMEOUT = 5000; // Таймаут фокуса интерфейса (5 секунд)

// =============================================================================
// 3. ТИПЫ ДАННЫХ И СТРУКТУРЫ СИСТЕМЫ
// =============================================================================

// Глобальные режимы работы каналов
enum ChannelMode {
    MODE_BOTH,   // BOTH CH (Стерео-линк)
    MODE_LEFT,   // SPL CH (Левый в фокусе)
    MODE_RIGHT   // SPL CH (Правый в фокусе)
};

// Подрежимы генерации сигнала
enum SignalSubMode {
    SUBMODE_STATIC,      // FIX (Постоянная частота)
    SUBMODE_SWEEP,       // SWP (Качание частоты)
    SUBMODE_PHASE_SHIFT, // PHASE SHIFT (Ручной статический сдвиг)
    SUBMODE_PHASE_ROT,   // PHASE ROT (Динамическое вращение)
    SUBMODE_IMD,         // IMD TEST (Двухтональный тест искажений)
    SUBMODE_WHITE_NOISE, // Белый шум
    SUBMODE_PINK_NOISE   // Розовый шум
};

// Форма генерируемой волны
enum WaveForm {
    WAVE_SIN,            // Синусоида (SIN)
    WAVE_SQR             // Меандр (SQR)
};

// Структура индивидуальных настроек одного канала
struct ChannelSettings {
    double freq;           // Текущая частота канала в Гц
    WaveForm wave;         // Форма волны канала
    SignalSubMode subMode; // Подрежим генерации (STATIC или SWEEP)
};

// Главная структура состояния прибора
struct DeviceState {
    ChannelMode chMode;         // Текущий режим (BOTH, LEFT, RIGHT)
    ChannelSettings left;       // Настройки левого канала (Master)
    ChannelSettings right;      // Настройки правого канала (Slave)
    
    int phaseAngle;             // Угол сдвига фазы (0...360)
    bool isMute;                // Флаг глушения звука
    int volume;                 // Текущая общая громкость (0...100%)
    
    // Переменные состояния интерфейса (Ядро 0)
    int activeCursorDigit;      // Позиция курсора частоты (-1 — выкл, 0..5 — индекс разряда)
    bool isPhaseFocus;          // Флаг фокуса энкодера на изменении фазы в шапке
    unsigned long focusTimer;   // Время (millis()) последнего действия пользователя
};

struct AppSettings {
    float sweepStartF;
    float sweepStopF;
    float sweepDuration;
    float phaseRotDuration;
    double bootFreq;
    int bootLevelDb;
    int startScreen;            // 0=Main, 1=Waves, 2=Noises, 3=MP3
    float imdFreq1;             // Первая частота интермодуляции (например, 60.0f)
    float imdFreq2;             // Вторая частота интермодуляции  (например, 7000.0f)
    float imdRatio1;            // Коэффициент амплитуды первой частоты (например, 0.8f для 4/5)
    float imdRatio2;            // Коэффициент амплитуды второй частоты (например, 0.2f для 1/5)
    bool playModeLoop;
};

// =============================================================================
// 4. ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ (ОБЪЯВЛЕНИЕ)
// =============================================================================
// Использование extern гарантирует, что объект dev будет создан в одном месте (в .ino файле),
// но доступен для чтения и записи во всех модулях (.cpp).


extern DeviceState dev;
extern AppSettings cfg;

void config_load();
void config_save();
