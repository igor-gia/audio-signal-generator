#include "Config.h"
#include "io.h"
#include "ui.h"
//#include "generator.h" // Подключаем обновленный генератор
//#include "player.h"

void setup() {
    Serial.begin(115200);
    delay(300);
    neopixelWrite(48, 0, 150, 0);
    delay(200);
    neopixelWrite(48, 150, 0, 0);
    delay(200);
    neopixelWrite(48, 0, 0, 150);
    delay(200);
    neopixelWrite(48, 0, 0, 0);
    pinMode(SDA_PIN, INPUT_PULLUP);
    pinMode(SCL_PIN, INPUT_PULLUP);
    delay(50);
    io_init();
    ui_init();
    Serial.println("All Systems Nominal. Core 0 and Core 1 active.");
}

void loop() {
    IOEvent currentEvent = io_poll();
    ui_update(currentEvent); // Здесь меняются экраны и обрабатываются кнопки
   
    ui_tick();
    
    vTaskDelay(pdMS_TO_TICKS(4));  
}