# AGP-S3: Dual-Channel Audio Generator & SD Player

[English](#English) | [Українська](#Ukrainian) | [русский](#russian)

---

## English

**AGP-S3** is a compact dual-channel low-frequency DDS signal generator and a high-quality audio player (Hi-Res Audio). The device is built on the powerful dual-core **ESP32-S3** microcontroller and a dedicated stereo DAC **PCM5102A** (192 kHz / 24-bit).

The instrument is designed for testing, tuning, and repairing various audio equipment: power amplifiers, acoustic systems, crossovers, equalizers, and other stages of the audio signal path.

---

## 📝 Author's Preface and Project Backstory

The idea of creating this device was born out of a real practical challenge. While building a custom home media center (using an ESP32 and TPA3255) and assembling a custom 10-inch subwoofer based on an Alpine driver powered by a TPA3255 monoblock running in parallel bridge mode (PBTL), I faced an issue with tuning the audio system.

I had a portable FNIRSI DSO-TC4 device on my bench, which combines both an oscilloscope and a signal generator. However, they share a common circuit ground. When tuning amplifiers operating in bridge mode (BTL/PBTL), it is physically impossible to simultaneously apply a test signal from the generator to the input and measure the output with the oscilloscope — doing so short-circuits one of the output phases to ground through the test equipment.

Initially, I planned to build a simple, standalone sine wave generator out of whatever spare parts I had lying around. But since I had a powerful **ESP32-S3 (N16R8)** microcontroller on hand, the project quickly grew beyond a simple "tone generator".

The result is a complete laboratory instrument that addresses two key scenarios:
1. **Precise Tuning of Audio Equipment:** a dual-channel pure tone generator, phase shifter, and test noise source that is fully galvanically isolated from the oscilloscope.
2. **High-Quality Audio Source (Hi-Res Player):** a standalone lossless audio player that can be used as a reference source when evaluating audio signal paths.

Most DIY generators rely on the microcontroller's internal 8-bit DACs or PWM, which introduce immense distortion. The **AGP-S3** utilizes a dedicated **PCM5102A** stereo DAC connected via the I2S bus, ensuring a pristine analog output signal, while the processing power of the ESP32-S3 allows for instantaneous, real-time phase and frequency calculations.

---

## 🚀 Key Features

* **Dual-Core Architecture:** Tasks are strictly split between the ESP32-S3 cores. Core 1 is fully dedicated to hardware sound generation over the I2S bus (ensuring zero jitter and playback stutters), while Core 0 handles the graphical user interface, button inputs, and SD card data reading.
* **High-Quality Audio Output:** The dedicated PCM5102A DAC provides a clean analog signal with sampling rates up to 192 kHz and 24-bit resolution.
* **Dual-Channel DDS Generator:** Frequency range from **1 Hz to 24,000 Hz** with adjustment steps down to **0.1 Hz**. Supported waveforms: *Sine* and *Square*.
* **Flexible Phase Control:** Manual phase shift adjustment between channels from $0^{\circ}$ to $360^{\circ}$ (in $10^{\circ}$ steps), as well as an automatic continuous phase rotation mode at a user-defined speed.
* **Test Noises & Special Signals:** Built-in White and Pink noise generators, plus a two-tone Intermodulation Distortion (IMD) test mode.
* **Hi-Res Audio Player:** Read and play back **WAV, MP3, OGG, and FLAC** audio files directly from a MicroSD card (FAT32).
* **Built-in Attenuator:** Features two pairs of analog RCA outputs — a direct output (0 dB) and an attenuated output (-20 dB) via a precision resistor divider for interfacing with sensitive, low-level inputs.
* **Non-Volatile Memory:** Automatically saves all current settings and parameters to the ESP32-S3 flash memory upon powering off.

---

## 🛠 Technical Specifications

| Parameter | Value |
| :--- | :--- |
| **Microcontroller** | ESP32-S3-WROOM-1 (N16R8) |
| **DAC** | Texas Instruments PCM5102A (I2S) |
| **Resolution / Sample Rate** | 24-bit / up to 192 kHz |
| **Generator Frequency Range** | 1 Hz – 24,000 Hz |
| **Frequency Adjustment Step** | 0.1 Hz / 1 Hz / 10 Hz / 100 Hz / 1000 Hz |
| **Waveforms** | Sine, Square (Meander) |
| **Phase Adjustment** | $0^{\circ} - 360^{\circ}$ ($10^{\circ}$ step) + auto-rotation mode |
| **Noise Modes** | White Noise, Pink Noise, IMD (two-tone test) |
| **Supported Card Formats** | MicroSD, FAT32 (up to 32 GB) |
| **Supported Audio Formats** | MP3, WAV, FLAC, OGG |
| **Display** | OLED 0.96" (SSD1306, I2C, 128x64) |
| **Power Supply** | USB Type-C (5V) |

---

## 📦 Bill of Materials (BOM)

To build this device, you will need the following main components:

| Component | Description / Module | Qty |
| :--- | :--- | :--- |
| **Microcontroller** | ESP32-S3-DevKitC-1-N16R8V (or compatible board with 16MB Flash and 8MB PSRAM) | 1 pc. |
| **DAC** | PCM5102A DAC Module (popular blue board with I2S interface) | 1 pc. |
| **Display** | OLED 0.96" 128x64 Module (SSD1306 driver, I2C interface) | 1 pc. |
| **Encoder** | EC11 incremental encoder with integrated push-button | 1 pc. |
| **Buttons** | Tactile push-buttons (12x12x7.5 mm) | 5 pcs. |
| **Card Reader** | MicroSD slot (TF-Card) wired for SD_MMC mode | 1 pc. |
| **Output Connectors** | Chassis-mount RCA jacks (for audio outputs) | 4 pcs. |
| **Power Connector** | Chassis-mount USB Type-C female breakout (for 5V power supply) | 1 pc. |
| **Passives** | Resistors and capacitors for filtering and attenuation (per schematic) | 1 set |

---

## 📂 Repository Structure

```text
├── hardware/                 # Schematics and PCB layout
│    ├── kicad_project/       # Original project files in KiCad
│    ├── scheme_v2.pdf        # Complete schematic diagram in vector PDF format
│    └── scheme.png           # Schematic image for quick preview
├── src/                      # Source code for Arduino IDE (.ino)
├── docs/                     # Documentation and manuals
│    └── User manual.pdf      # Complete User Manual (in Russian)
├── images/                   # Photos and project media files
├── LICENSE                   # MIT License
└── README.md                 # Project overview (this file)
```
---

## 🔌 Hardware & Pinout

The complete schematic diagram of the device is available in the [`hardware/scheme_v2.pdf`](hardware/scheme_v2.pdf) file.

![Schematic Diagram](hardware/scheme.png)

All controls (buttons and encoder) are wired as follows: **button pin to GPIO, second pin to GND** (the firmware utilizes the internal pull-up mode: `INPUT_PULLUP`).

### ESP32-S3 Pin Mapping Table:

| GPIO | Function | Constant in `Config.h` / Settings |
| :--- | :--- | :--- |
| **GPIO 1** | 5V Power Detector (5V detected) | |
| **GPIO 2** | BCLK (PCM5102A DAC) | `BCLK_PIN` |
| **GPIO 4** | Tactile button "WAVE" | `PIN_BTN_5` |
| **GPIO 5** | Tactile button "RIGHT CH" | `PIN_BTN_3` |
| **GPIO 6** | Encoder Button (Enc Btn) | `PIN_ENC_BTN` |
| **GPIO 7** | Encoder Direction A (Enc+) | `PIN_ENC_A` |
| **GPIO 13**| Tactile button "LEFT CH" | `PIN_BTN_2` |
| **GPIO 14**| Tactile button "MODE" | `PIN_BTN_4` |
| **GPIO 15**| Encoder Direction B (Enc-) | `PIN_ENC_B` |
| **GPIO 16**| Tactile button "BOTH CH" | `PIN_BTN_1` |
| **GPIO 21**| TF-Card CMD (SD_MMC class) | `SD_MMC_CMD` |
| **GPIO 39**| SCL (SSD1306 Display) | `SCL_PIN` |
| **GPIO 40**| SDA (SSD1306 Display) | `SDA_PIN` |
| **GPIO 41**| LRCK / WS (PCM5102A DAC) | `LRCK_PIN` |
| **GPIO 42**| DIN / DOUT (PCM5102A DAC) | `DIN_PIN` |
| **GPIO 47**| TF-Card CLK (SD_MMC class) | `SD_MMC_CLK` |
| **GPIO 48**| TF-Card D0 (SD_MMC class) | `SD_MMC_D0` |

---

## 💻 Environment Setup & Flashing

To compile and upload the firmware to the ESP32-S3 via the development board's USB port, configure the following settings in the **Arduino IDE**:

* **Board:** `ESP32S3 Dev Module`
* **Port:** Select the COM port of your connected device
* **USB CDC On Boot:** `Enabled` (for debugging output in the Serial Monitor)
* **CPU Frequency:** `240MHz (WiFi/BT)`
* **Core Debug Level:** `None`
* **USB DFU On Boot:** `Disabled`
* **Erase All Flash Before Sketch Upload:** `Disabled`
* **Arduino Runs On:** `Core 0`
* **Events Run On:** `Core 1`
* **Flash Frequency:** `80MHz`
* **Flash Mode:** `QIO 80MHz`
* **Flash Size:** `16MB (128Mb)`
* **JTAG Adapter:** `Integrated USB JTAG`
* **Partition Scheme:** `16M Flash (3MB APP/9.9MB FATFS)`
* **PSRAM:** `OPI PSRAM`
* **Upload Mode:** `UART0 / Hardware CDC`
* **Upload Speed:** `921600`

---

## 🎮 Device Control (Quick Start)

![AGP-S3 Controls](images/main%20view.png)

### 1. Main Menu Navigation (Mode Selection)

When powering on the device (or when returning to the Main Menu), the screen displays a list of available modes.

* **Mode Selection (Moving the Highlight):** Done using the **[LEFT]** and **[RIGHT]** buttons or by **turning the encoder knob**. The selected mode will be highlighted on the screen.
* **Entering the Selected Mode:** Press the **[BOTH]** button to confirm and enter the highlighted mode.
* **Quick Return to the Main Menu:** You can return to the Main Menu from any active operating mode by **holding down the [MODE] button**.

---

### 2. Setting the Frequency (in Generator Modes):

The process for precise frequency tuning is designed as follows:
1. **Enter Frequency Edit Mode:** Press and **hold** any of the channel selection buttons — **[LEFT]**, **[BOTH]**, or **[RIGHT]**.
2. **Shift Between Digits:** To select which digit to change (indicated by the underline cursor on the screen), press the **[LEFT]** button (shifts left / increases the digit weight) or the **[RIGHT]** button (shifts right / decreases the digit weight).
3. **Change the Digit Value:** Rotate the encoder knob to increase or decrease the value of the selected digit.
4. **Exit Frequency Edit Mode:** Press the **[BOTH]** button to apply the changes and return to the normal display state.

---

### 3. Operating Modes:

#### 📺 SCREEN 1. Waves Generator
Allows independent or combined configuration of the left and right channels:
* The **[WAVE]** button cyclically changes the waveform: **Sine** or **Square**.
* In the right channel view (RIGHT), you can manually set a phase shift relative to the left channel from 0° to 360° (in 10° steps), or activate the automatic phase rotation mode with speeds ranging from 1°/sec to 360°/sec.

#### 📺 SCREEN 2. Noises & Tests
Generates uncorrelated test signals and noise. Toggle through the active signal type with a short press of any button: [Left], [Both], [Right], or [Wave]:
* **White Noise:** Uniform spectral power density across the entire frequency range.
* **Pink Noise:** Spectral power density rolls off at 3 dB per octave.
* **IMD (Intermodulation Distortion Test):** A two-tone signal used to evaluate intermodulation distortion ($f_1$ and $f_2$ mixed in user-defined amplitude proportions).

#### 📺 SCREEN 3. MP3 Player
Plays Hi-Res audio files (WAV, MP3, OGG, FLAC) from a MicroSD card (FAT32).
* Supports repeat options: repeat all files in the current folder (`Folder`) or loop the active track (`Repeat`).
* Player controls:
  * **[LEFT]** — Previous track / rewind.
  * **[RIGHT]** — Next track / fast forward.
  * **[BOTH]** — Start / Stop playback.
  * **[Wave]** — Pause playback.

#### 📺 SCREEN 4. Settings
Navigate through the settings list by turning the encoder or by pressing the [LEFT] / [RIGHT] buttons.
* To modify a setting, press the encoder knob **[Encoder]** (or the **[BOTH]** button). The selection cursor will highlight the parameter value.
* Change the value by rotating the encoder knob.
* Press the encoder knob or the **[BOTH]** button again to save the setting to the non-volatile memory.

**Some of the menu parameters:**
* `Phase Rot` (Phase Auto-rotation): Speed of rotation from 1°/sec to 360°/sec.
* `IMD f1`, `IMD f2`: Target frequencies for the two-tone intermodulation test.
* `IMD Amplit f1`, `IMD Amplit f2`: Amplitude mixing ratios (must sum to 1.0, e.g., 0.8 and 0.2).
* `Player Mode`: Player repeat configuration (`Folder` / `Repeat`).
* `Start Scr` (Startup Screen): Active screen upon booting the device (`Main`, `Waves`, `Noises`, `MP3`).

---

⚠️ **Important Note:** This "Quick Start" section covers only the basic controls and provides brief descriptions of the main features. The device offers a much wider range of capabilities. For a detailed guide on all operating modes, system configurations, and setup procedures, please refer to the official User Manual: [`docs/User manual.pdf`](docs/User%20manual.pdf) (currently available in Russian).

---

### Support
If you find this project useful and want to support the author, you can make a donation using any of the following methods:

- **PayPal**: [gia@gia.org.ua] [Donate via PayPal](https://www.paypal.me)  
- **Ko-fi**: [Donate on Ko-fi](https://ko-fi.com/igorgimelfarb)  
- **Monobank**: Scan the QR code or use the link: [Support via Monobank](https://send.monobank.ua/jar/8HG6A3VPaW)

![Support via Monobank](images/monobank_DIY_QR.png)

**Your support is highly appreciated!**

---

## ⚖️ License

This project is licensed under the **MIT License**. You are free to use, modify, and distribute this project for both commercial and non-commercial purposes, provided that the original copyright notice and author attribution are retained. See the [LICENSE](LICENSE) file for details.

---
*Project Author: Igor Gimelfarb (2026)*

---

###### Tags / Keywords: 
`esp32s3 audio generator`, `DDS signal generator DIY`, `PCM5102A I2S DAC`, `hi-res lossless player ESP32`, `audio testing tool`, `phase rotation generator`, `pink noise generator DIY`, `IMD test signal`, `BTL amplifier tuning`, `KiCad audio circuit`

---

[English](#English) | [Українська](#Ukrainian) | [русский](#russian)

---

## Ukrainian

# AGP-S3: Dual-Channel Audio Generator & SD Player

**AGP-S3** — це компактний двоканальний низькочастотний DDS-генератор сигналів та високоякісний аудіоплеєр (Hi-Res Audio). Пристрій побудовано на базі потужного двоядерного мікроконтролера **ESP32-S3** та виділеного стерео-ЦАП **PCM5102A** (192 кГц / 24 біт).

Прилад призначений для тестування, налаштування та ремонту різноманітної аудіоапаратури: підсилювачів потужності, акустичних систем, кросоверів, еквалайзерів та інших ланок звукового тракту.

---

## 📝 Передмова автора та передісторія проєкту

Ідея створення цього приладу народилася з реальної практичної задачі. У процесі побудови домашнього медіацентру (на зв'язці ESP32 та TPA3255) та збирання кастомного 10-дюймового сабвуфера на динаміці Alpine з моноблоком TPA3255 у режимі паралельного моста (PBTL) я зіткнувся з проблемою налаштування аудіосистеми.

У моєму розпорядженні був портативний прилад FNIRSI DSO-TC4, що поєднує в собі функції осцилографа та генератора сигналів. Однак вони мають спільну схемну землю. При налаштуванні підсилювачів, що працюють у мостовому режимі (BTL/PBTL), фізично неможливо одночасно подавати тестовий сигнал з генератора на вхід і знімати показання осцилографом з виходу — це призводить до короткого замикання одного з плечей виходу на землю через вимірювальний прилад. 

Спочатку планувалося зібрати найпростіший автономний генератор синусу з того, що було під рукою. Але оскільки в наявності був потужний мікроконтролер **ESP32-S3 (N16R8)**, проєкт швидко переріс рамки «простої пищалки». 

В результаті вийшов закінчений лабораторний прилад, який закриває відразу два ключові сценарії:
1. **Точне налаштування аудіоапаратури:** повністю гальванічно розв'язаний від осцилографа двоканальний генератор чистих тонів, фазообертач та джерело тестових шумів.
2. **Високоякісне джерело звуку (Hi-Res Player):** автономний програвач аудіофайлів без втрат (Lossless), який можна використовувати як еталонний плеєр при прослуховуванні звукових трактів.

Більшість аматорських генераторів використовують вбудовані 8-бітні ЦАП мікроконтролерів або ШІМ, що дає колосальні спотворення. В **AGP-S3** застосовано виділений стерео-ЦАП **PCM5102A** з шиною I2S, що забезпечує найчистіший аналоговий сигнал, а обчислювальна потужність ESP32-S3 дозволяє миттєво розраховувати фазу та частоту на льоту.

---

## 🚀 Ключові особливості

* **Двоядерна архітектура:** Задачі жорстко розділені між ядрами ESP32-S3. Одне ядро повністю виділене під апаратну генерацію звуку по шині I2S (що гарантує відсутність джиттера та затримок), а друге обслуговує графічний інтерфейс, обробку натискань та читання даних з SD-карти.
* **Висока якість звуку:** Виділений ЦАП PCM5102A забезпечує чистий аналоговий сигнал із частотою дискретизації до 192 кГц та розрядністю 24 біт.
* **Двоканальний DDS-генератор:** Діапазон частот від **1 Гц до 24 000 Гц** із кроком регулювання до **0.1 Гц**. Підтримувані форми сигналів: *Синус* та *Меандр*.
* **Гнучке управління фазою:** Можливість ручного встановлення зсуву фази між каналами від $0^{\circ}$ до $360^{\circ}$ (із кроком $10^{\circ}$), а також режим автоматичного безперервного обертання фази із заданою швидкістю.
* **Тестові шуми та спецсигнали:** Вбудований генератор Білого (White) та Рожевого (Pink) шуму, а також режим двотонального інтермодуляційного тесту (IMD).
* **Hi-Res Аудіоплеєр:** Читання та відтворення аудіофайлів форматів **WAV, MP3, OGG, FLAC** безпосередньо з карти пам'яті MicroSD (FAT32).
* **Вбудований атенюатор:** Наявність двох пар аналогових виходів RCA — прямий вихід (0 дБ) та ослаблений на -20 дБ через прецизійний резистивний дільник для роботи зі слабкострумовими входами.
* **Енергонезалежна пам'ять:** Автоматичне збереження всіх налаштувань та параметрів у пам'ять ESP32-S3 при вимкненні приладу.

---

## 🛠 Технічні характеристики

| Параметр | Значення |
| :--- | :--- |
| **Мікроконтролер** | ESP32-S3-WROOM-1 (N16R8) |
| **ЦАП (DAC)** | Texas Instruments PCM5102A (I2S) |
| **Розрядність / Частота** | 24 біт / до 192 кГц |
| **Діапазон частот генератора** | 1 Гц – 24 000 Гц |
| **Крок перебудови частоти** | 0.1 Гц / 1 Гц / 10 Гц / 100 Гц / 1000 Гц |
| **Форми сигналів** | Синусоїда (Sine), Меандр (Square) |
| **Регулювання фази** | $0^{\circ} - 360^{\circ}$ (крок $10^{\circ}$) + режим авторотації |
| **Режими шумів** | Білий шум, Рожевий шум, IMD (двотональний тест) |
| **Підтримувані формати карт пам'яті** | MicroSD, FAT32 (до 32 ГБ) |
| **Підтримувані аудіоформати** | MP3, WAV, FLAC, OGG |
| **Дисплей** | OLED 0.96" (SSD1306, I2C, 128x64) |
| **Живлення** | USB Type-C (5В) |

---

## 📦 Список компонентів (BOM)

Для збирання пристрою вам знадобляться такі основні компоненти:

| Компонент | Опис / Модуль | Кількість |
| :--- | :--- | :--- |
| **Мікроконтролер** | ESP32-S3-DevKitC-1-N16R8V (або аналогічна плата з 16MB Flash та 8MB PSRAM) | 1 шт. |
| **ЦАП** | Модуль ЦАП PCM5102A (популярна синя плата з інтерфейсом I2S) | 1 шт. |
| **Дисплей** | Модуль OLED 0.96" 128x64 (драйвер SSD1306, інтерфейс I2C) | 1 шт. |
| **Енкодер** | Інкрементальний енкодер EC11 із вбудованою кнопкою | 1 шт. |
| **Кнопки** | Тактові кнопки 12x12x7.5 мм | 5 шт. |
| **Кардридер** | Слот MicroSD (TF-Card) для роботи в режимі SD_MMC | 1 шт. |
| **Роз'єми виходу** | Гнізда RCA на панель (для аудіовиходів) | 4 шт. |
| **Роз'єм живлення** | Гніздо USB Type-C на панель (для подачі живлення 5В) | 1 шт. |
| **Пасивні елементи** | Резистори та конденсатори обв'язки (відповідно до схеми) | 1 компл. |

---

## 📂 Структура репозиторію

```text
├── hardware/                 # Схемотехніка та друкована плата
│    ├── kicad_project/       # Вихідний проєкт пристрою у середовищі KiCad
│    ├── scheme_v2.pdf        # Повна принципова схема у векторному форматі PDF
│    └── scheme.png           # Зображення принципової схеми
├── src/                      # Вихідний код проєкту для Arduino IDE (.ino)
├── docs/                     # Документація та посібники
│    └── User manual.pdf      # Повний посібник користувача (російською мовою)
├── images/                   # Фотографії та медіафайли проєкту
├── LICENSE                   # Ліцензія MIT
└── README.md                 # Опис проєкту (цей файл)
```
---
## 🔌 Схемотехніка та розпиновка пристрою

![Принципова схема](hardware/scheme.png)
Повна принципова електрична схема пристрою у форматі PDF знаходиться у файлі [`hardware/scheme_v2.pdf`](hardware/scheme_v2.pdf). 

Усі органи управління (кнопки та енкодер) підключені за схемою: **вивід кнопки — GPIO, другий вивід — GND** (у прошивці використовується внутрішній режим підтяжки `INPUT_PULLUP`).

### Таблица підключення периферії до ESP32-S3:

| GPIO | Призначення | Константа в `Config.h` / Налаштування |
| :--- | :--- | :--- |
| **GPIO 1** | Детектор живлення 5В (5V detected) | |
| **GPIO 2** | BCLK (ЦАП PCM5102A) | `BCLK_PIN` |
| **GPIO 4** | Тактова кнопка «WAVE» | `PIN_BTN_5` |
| **GPIO 5** | Тактова кнопка «RIGHT CH» | `PIN_BTN_3` |
| **GPIO 6** | Кнопка енкодера (Enc Btn) | `PIN_ENC_BTN` |
| **GPIO 7** | Напрямок енкодера (Enc+) | `PIN_ENC_A` |
| **GPIO 13**| Тактова кнопка «LEFT CH» | `PIN_BTN_2` |
| **GPIO 14**| Тактова кнопка «MODE» | `PIN_BTN_4` |
| **GPIO 15**| Напрямок енкодера (Enc-) | `PIN_ENC_B` |
| **GPIO 16**| Тактова кнопка «BOTH CH» | `PIN_BTN_1` |
| **GPIO 21**| TF-Card CMD (клас SD_MMC) | `SD_MMC_CMD` |
| **GPIO 39**| SCL (Дисплей SSD1306) | `SCL_PIN` |
| **GPIO 40**| SDA (Дисплей SSD1306) | `SDA_PIN` |
| **GPIO 41**| LRCK / WS (ЦАП PCM5102A) | `LRCK_PIN` |
| **GPIO 42**| DIN / DOUT (ЦАП PCM5102A) | `DIN_PIN` |
| **GPIO 47**| TF-Card CLK (клас SD_MMC) | `SD_MMC_CLK` |
| **GPIO 48**| TF-Card D0 (клас SD_MMC) | `SD_MMC_D0` |

---

## 💻 Налаштування середовища та прошивка

Для компіляції та завантаження прошивки в ESP32-S3 через USB-роз'єм плати розробника встановіть такі параметри в **Arduino IDE**:

* **Board (Плата):** `ESP32S3 Dev Module`
* **Port (Порт):** Виберіть COM-порт вашого пристрою
* **USB CDC On Boot:** `Enabled` (активує виведення логів відлагодження в консоль)
* **CPU Frequency:** `240MHz (WiFi/BT)`
* **Core Debug Level:** `None`
* **USB DFU On Boot:** `Disabled`
* **Erase All Flash Before Sketch Upload:** `Disabled`
* **Arduino Runs On:** `Core 0`
* **Events Run On:** `Core 1`
* **Flash Frequency:** `80MHz`
* **Flash Mode:** `QIO 80MHz`
* **Flash Size:** `16MB (128Mb)`
* **JTAG Adapter:** `Integrated USB JTAG`
* **Partition Scheme:** `16M Flash (3MB APP/9.9MB FATFS)`
* **PSRAM:** `OPI PSRAM`
* **Upload Mode:** `UART0 / Hardware CDC`
* **Upload Speed:** `921600`

---

## 🎮 Керування пристроєм (Швидкий старт)

![Органи керування AGP-S3](images/main%20view.png)

### 1. Навігація на Головному екрані (Вибір режиму)

При увімкненні приладу (або при виході на Головний екран) перед вами відображається список доступних режимів. 

* **Вибір режиму (Переміщення підсвічування):** Здійснюється кнопками **[LEFT]**, **[RIGHT]** або **обертанням енкодера**. Відповідний режим буде підсвічено на дисплеї.
* **Вхід у вибраний режим:** Натисніть кнопку **[BOTH]** для підтвердження та входу в підсвічений режим.
* **Швидке повернення на Головний екран:** З будь-якого режиму роботи повернутися назад на Головний екран можна за допомогою **тривалого утримання кнопки [MODE]**.

---

### 2. Налаштування частоти (у режимах генераторів):

Процес точного налаштування частоти організований таким чином:
1. **Вхід у режим зміни частоти:** Натисніть та **утримуйте** одну з кнопок вибору каналів — **[LEFT]**, **[BOTH]** або **[RIGHT]**.
2. **Перемикання між розрядами частоти:** Для вибору змінного розряду (курсор на екрані підкреслює поточний крок) натискайте кнопки **[LEFT]** (крок вліво / збільшення розряду) та **[RIGHT]** (крок вправо / зменшення розряду).
3. **Зміна значення розряду:** Обертайте ручку енкодера для збільшення або зменшення значення вибраного розряду частоты.
4. **Вихід із режиму встановлення частоти:** Натисніть тактову кнопку **[BOTH]** для фіксації змін та повернення до звичайного відображення.

---

### 3. Режими роботи приладу:

#### ЕКРАН 1. Генератор сигналов (Waves)
Дозволяє незалежно або спільно налаштовувати параметри лівого та правого каналів:
* Кнопка **[WAVE]** циклічно змінює форму сигналу: **Синусоїда (Sine)** або **Меандр (Square)**.
* На вкладці правого каналу (RIGHT) доступне ручне встановлення зсуву фази відносно лівого каналу від 0° до 360° із кроком 10°, або активація режиму автоматичного безперервного обертання фази зі швидкістю від 1°/сек до 360°/сек.

#### ЕКРАН 2. Генератор тестових шумів (Noises)
Генерація некорельованих спецсигналів та шумів. Вибір активного типу сигналу здійснюється коротким натисканням будь-якої з кнопок: [Left], [Both], [Right] або [Wave]:
* **White Noise (Білий шум):** Спектральна щільність постійна по всьому діапазону частот.
* **Pink Noise (Розовый шум):** Спектральна щільність спадає на 3 дБ на октаву.
* **IMD (Intermodulation Distortion Test):** Двотональний сигнал для оцінки інтермодуляційних спотворень ($f_1$ та $f_2$ у задаваних пропорціях амплітуди).

#### ЕКРАН 3. MP3 Аудіопрогравач (Player)
Відтворює Hi-Res аудіофайли (WAV, MP3, OGG, FLAC) з карти пам'яті MicroSD (FAT32).
* Підтримується циклічне програвання файлів у поточній папці (`Folder`) або повтор одного треку (`Repeat`).
* Кнопки керування плеєром:
  * **[LEFT]** — Попередній трек / перемотування назад.
  * **[RIGHT]** — Наступний трек / перемотування вперед.
  * **[BOTH]** — Старт / Cтоп відтворення.
  * **[Wave]** — Пауза відтворення.

#### ЕКРАН 4. Системні налаштування (Settings)
Навігація за списком параметрів здійснюється обертанням енкодера або натисканням кнопок [LEFT] / [RIGHT]. 
* Для зміни параметра натисніть кнопку енкодера **[Encoder]** (або кнопку **[BOTH]**). Курсор переключиться на значення параметра.
* Змініть значення обертанням енкодера.
* Натисніть кнопку енкодера або **[BOTH]** повторно для збереження налаштування в енергонезависиму пам'ять мікроконтролера.

**Деякі параметри меню:**
* `Phase Rot` (Авторотація фази): Швидкість авторотації від 1°/сек до 360°/сек.
* `IMD f1`, `IMD f2`: Задання опорних частот для інтермодуляційного тесту.
* `IMD Amplit f1`, `IMD Amplit f2`: Задання співвідношення амплітуд двох частот (у сумі має бути 1.0, наприклад 0.8 і 0.2).
* `Player Mode`: Режим повтору плеєра (`Folder` / `Repeat`).
* `Start Scr` (Стартовий екран): Екран, що активується при увімкненні приладу (`Main`, `Waves`, `Noises`, `MP3`).

---

⚠️ **Важлива примітка:** У цьому розділі «Швидкий старт» наведено лише базові принципи керування та короткий опис основних функций. Пристрій має значно ширші можливості. Детальний опис з усіма режимами роботи, налаштуваннями та тонкощами експлуатації наведено в посібнику користувача: [`docs/User manual.pdf`](docs/User%20manual.pdf).

---

### Підтримка
Якщо Ви вважаєте цей проєкт корисним і хочете підтримати автора, Ви можете зробити пожертву будь-яким із таких способів:

- **PayPal**: [gia@gia.org.ua] [Donate via PayPal](https://www.paypal.me)  
- **Ko-fi**: [Donate on Ko-fi](https://ko-fi.com/igorgimelfarb)  
- **Monobank**: Відсканируйте QR-код або скористайтеся посиланням: [Підтримати через Monobank](https://send.monobank.ua/jar/8HG6A3VPaW)

![Підтримати через Monobank](images/monobank_DIY_QR.png)

**Ваша підтримка буде дуже цінною!**

---

## ⚖️ Ліцензія

Цей проєкт розповсюджується під вільною ліцензією **MIT**. Ви можете вільно використовувати, модифікувати та розповсюджувати цей проєкт як у некомерційних, так і в комерційних цілях, за умови збереження згадки авторства. Подробиці див. у файлі [LICENSE](LICENSE).

---
*Автор проєкту: Гімельфарб Ігор (2026 р.)*

---

###### Теги / Ключові слова:
`генератор звукових частот ESP32`, `DDS генератор синусу`, `ЦАП PCM5102A схема`, `аудіоплеєр на ESP32-S3`, `вимірювання спотворень підсилювача`, `генератор рожевого шуму`, `тестування мостових підсилювачів`, `радіоаматорські прилади DIY`

---

[English](#English) | [Українська](#Ukrainian) | [русский](#russian)

---

## russian

# AGP-S3: Dual-Channel Audio Generator & SD Player

**AGP-S3** — это компактный двухканальный низкочастотный DDS-генератор сигналов и высококачественный аудиопроигрыватель (Hi-Res Audio). Устройство построено на базе мощного двухъядерного микроконтроллера **ESP32-S3** и выделенного стерео-ЦАП **PCM5102A** (192 кГц / 24 бит).

Прибор предназначен для тестирования, настройки и ремонта различной аудиоаппаратуры: усилителей мощности, акустических систем, кроссоверов, эквалайзеров и других звеньев звукового тракта.

---

## 📝 Предисловие автора и предыстория проекта

Идея создания этого прибора родилась из реальной практической задачи. В процессе постройки домашнего медиацентра (на связке ESP32 и TPA3255) и сборки кастомного 10-дюймового сабвуфера на динамике Alpine с моноблоком TPA3255 в режиме параллельного моста (PBTL) я столкнулся с проблемой настройки аудиосистемы.

В моем распоряжении был портативный прибор FNIRSI DSO-TC4, совмещающий в себе функции осциллографа и генератора сигналов. Однако они имеют общую схемную землю. При настройке усилителей, работающих в мостовом режиме (BTL/PBTL), физически невозможно одновременно подавать тестовый сигнал с генератора на вход и снимать показания осциллографом с выхода — это приводит к короткому замыканию одного из плеч выхода на землю через прибор. 

Изначально планировалось собрать простейший автономный генератор синуса из того, что было под рукой. Но поскольку в наличии оказался мощный микроконтроллер **ESP32-S3 (N16R8)**, проект быстро перерос рамки «простой пищалки». 

В результате получился законченный лабораторный прибор, закрывающий сразу два ключевых сценария:
1. **Точная настройка аудиоаппаратуры:** полностью гальванически развязанный от осциллографа двухканальный генератор чистых тонов, фазовращатель и источник тестовых шумов.
2. **Высококачественный источник звука (Hi-Res Player):** автономный проигрыватель аудиофайлов без потерь (Lossless), который можно использовать как эталонный плеер при прослушивании звуковых трактов.

Большинство любительских генераторов используют встроенные 8-битные ЦАП микроконтроллеров или ШИМ, что дает колоссальные искажения. В **AGP-S3** применен выделенный стерео-ЦАП **PCM5102A** с шиной I2S, обеспечивающий чистейший аналоговый сигнал, а вычислительная мощность ESP32-S3 позволяет мгновенно рассчитывать фазу и частоту на лету.

---

## 🚀 Ключевые особенности

* **Двухъядерная архитектура:** Задачи жестко разделены между ядрами ESP32-S3. Одно ядро полностью выделено под аппаратную генерацию звука по шине I2S (что гарантирует отсутствие джиттера и задержек), а второе обслуживает графический интерфейс, обработку нажатий и чтение данных с SD-карты.
* **Высокое качество звука:** Выделенный ЦАП PCM5102A обеспечивает чистый аналоговый сигнал с частотой дискретизации до 192 кГц и разрядностью 24 бит.
* **Двухканальный DDS-генератор:** Диапазон частот от **1 Гц до 24 000 Гц** с шагом регулировки до **0.1 Гц**. Поддерживаемые формы сигналов: *Синус* и *Меандр*.
* **Гибкое управление фазой:** Возможность ручной установки сдвига фазы между каналами от $0^{\circ}$ до $360^{\circ}$ (с шагом $10^{\circ}$), а также режим автоматического непрерывного вращения фазы с заданной скоростью.
* **Тестовые шумы и спецсигналы:** Встроенный генератор Белого (White) и Розового (Pink) шума, а также режим двухтонального интермодуляционного теста (IMD).
* **Hi-Res Аудиопроигрыватель:** Чтение и воспроизведение аудиофайлов форматов **WAV, MP3, OGG, FLAC** напрямую с карты памяти MicroSD (FAT32).
* **Встроенный аттенюатор:** Наличие двух пар аналоговых выходов RCA — прямой выход (0 дБ) и ослабленный на -20 дБ через прецизионный резистивный делитель для работы со слаботочными входами.
* **Энергонезависимая память:** Автоматическое сохранение всех настроек и параметров в память ESP32-S3 при выключении прибора.

---

## 🛠 Технические характеристики

| Параметр | Значение |
| :--- | :--- |
| **Микроконтроллер** | ESP32-S3-WROOM-1 (N16R8) |
| **ЦАП (DAC)** | Texas Instruments PCM5102A (I2S) |
| **Разрядность / Частота** | 24 бит / до 192 кГц |
| **Диапазон частот генератора** | 1 Гц – 24 000 Гц |
| **Шаг перестройки частоты** | 0.1 Гц / 1 Гц / 10 Гц / 100 Гц / 1000 Гц |
| **Формы сигналов** | Синусоида (Sine), Меандр (Square) |
| **Регулировка фазы** | $0^{\circ} - 360^{\circ}$ (шаг $10^{\circ}$) + режим авторотации |
| **Режимы шумов** | Белый шум, Розовый шум, IMD (двухтональный тест) |
| **Поддерживаемые форматы карт памяти** | MicroSD, FAT32 (до 32 ГБ) |
| **Поддерживаемые аудиоформаты** | MP3, WAV, FLAC, OGG |
| **Дисплей** | OLED 0.96" (SSD1306, I2C, 128x64) |
| **Питание** | USB Type-C (5В) |

---

## 📦 Список компонентов (BOM)

Для сборки устройства вам потребуются следующие основные компоненты:

| Компонент | Описание / Модуль | Кол-во |
| :--- | :--- | :--- |
| **Микроконтроллер** | ESP32-S3-DevKitC-1-N16R8V (или аналогичная плата с 16MB Flash и 8MB PSRAM) | 1 шт. |
| **ЦАП** | Модуль ЦАП PCM5102A (популярная синяя плата с I2S интерфейсом) | 1 шт. |
| **Дисплей** | Модуль OLED 0.96" 128x64 (драйвер SSD1306, интерфейс I2C) | 1 шт. |
| **Энкодер** | Инкрементальный энкодер EC11 со встроенной кнопкой | 1 шт. |
| **Кнопки** | Тактовые кнопки 12x12x7.5 мм | 5 шт. |
| **Кардридер** | Слот MicroSD (TF-Card) для работы в режиме SD_MMC | 1 шт. |
| **Разъемы выхода** | Гнезда RCA на панель (для аудиовыходов) | 4 шт. |
| **Разъем питания** | Гнездо USB Type-C на панель (для подачи питания 5В) | 1 шт. |
| **Пассивные элементы** | Резисторы и конденсаторы обвязки (согласно схеме) | 1 компл. |

---

## 📂 Структура репозитория

```text
├── hardware/                 # Схемотехника и печатная плата
│    ├── kicad_project/       # Исходный проект устройства в среде KiCad
│    ├── scheme_v2.pdf        # Полная принципиальная схема в векторном формате PDF
│    └── scheme.png           # Изображение принципиальной схемы
├── src/                      # Исходный код проекта для Arduino IDE (.ino)
├── docs/                     # Документация и руководства
│    └── User manual.pdf      # Полное руководство пользователя на русском языке
├── images/                   # Фотографии и медиафайлы проекта
├── LICENSE                   # Лицензия MIT
└── README.md                 # Описание проекта (этот файл)
```
---

## 🔌 Схемотехника и распиновка устройства

![Принципиальная схема](hardware/scheme.png)
Принципиальная электрическая схема устройства в формате PDF находится в файле [`hardware/scheme_v2.pdf`](hardware/scheme_v2.pdf). 

Все органы управления (кнопки и энкодер) подключены по схеме: **вывод кнопки — GPIO, второй вывод — GND** (в прошивке используется внутренний режим подтяжки `INPUT_PULLUP`).

### Таблица подключения периферии к ESP32-S3:

| GPIO | Назначение | Константа в `Config.h` / Настройки |
| :--- | :--- | :--- |
| **GPIO 1** | Детектор питания 5В (5V detected) | |
| **GPIO 2** | BCLK (ЦАП PCM5102A) | `BCLK_PIN` |
| **GPIO 4** | Тактовая кнопка «WAVE» | `PIN_BTN_5` |
| **GPIO 5** | Тактовая кнопка «RIGHT CH» | `PIN_BTN_3` |
| **GPIO 6** | Кнопка энкодера (Enc Btn) | `PIN_ENC_BTN` |
| **GPIO 7** | Направление энкодера (Enc+) | `PIN_ENC_A` |
| **GPIO 13**| Тактовая кнопка «LEFT CH» | `PIN_BTN_2` |
| **GPIO 14**| Тактовая кнопка «MODE» | `PIN_BTN_4` |
| **GPIO 15**| Направление энкодера (Enc-) | `PIN_ENC_B` |
| **GPIO 16**| Тактовая кнопка «BOTH CH» | `PIN_BTN_1` |
| **GPIO 21**| TF-Card CMD (класс SD_MMC) | `SD_MMC_CMD` |
| **GPIO 39**| SCL (Дисплей SSD1306) | `SCL_PIN` |
| **GPIO 40**| SDA (Дисплей SSD1306) | `SDA_PIN` |
| **GPIO 41**| LRCK / WS (ЦАП PCM5102A) | `LRCK_PIN` |
| **GPIO 42**| DIN / DOUT (ЦАП PCM5102A) | `DIN_PIN` |
| **GPIO 47**| TF-Card CLK (класс SD_MMC) | `SD_MMC_CLK` |
| **GPIO 48**| TF-Card D0 (класс SD_MMC) | `SD_MMC_D0` |

---

## 💻 Настройка среды и прошивка

Для компиляции и загрузки прошивки в ESP32-S3 через USB-разъем платы разработчика установите следующие параметры в **Arduino IDE**:

* **Board (Плата):** `ESP32S3 Dev Module`
* **Port (Порт):** Выберите COM-порт вашего устройства
* **USB CDC On Boot:** `Enabled` (активирует вывод логов отладки в консоль)
* **CPU Frequency:** `240MHz (WiFi/BT)`
* **Core Debug Level:** `None`
* **USB DFU On Boot:** `Disabled`
* **Erase All Flash Before Sketch Upload:** `Disabled`
* **Arduino Runs On:** `Core 0`
* **Events Run On:** `Core 1`
* **Flash Frequency:** `80MHz`
* **Flash Mode:** `QIO 80MHz`
* **Flash Size:** `16MB (128Mb)`
* **JTAG Adapter:** `Integrated USB JTAG`
* **Partition Scheme:** `16M Flash (3MB APP/9.9MB FATFS)`
* **PSRAM:** `OPI PSRAM`
* **Upload Mode:** `UART0 / Hardware CDC`
* **Upload Speed:** `921600`

---

## 🎮 Управление устройством (Быстрый старт)

![Органы управления AGP-S3](images/main%20view.png)

### 1. Навигация на Главном экране (Выбор режима)

При включении прибора (или при выходе на Главный экран) перед вами отображается список доступных режимов. 

* **Выбор режима (Перемещение подсветки):** Осуществляется кнопками **[LEFT]**, **[RIGHT]** или **вращением энкодера**. Соответствующий режим будет подсвечен на дисплее.
* **Вход в выбранный режим:** Нажмите кнопку **[BOTH]** для подтверждения и входа в подсвеченный режим.
* **Быстрый возврат на Главный экран:** Из любого режима работы вернуться обратно на Главный экран можно с помощью **длительного удержания кнопки [MODE]**.

---

### 2. Настройка частоты (в режимах генераторов):

Процесс точной настройки частоты организован следующим образом:
1. **Вход в режим изменения частоты:** Нажмите и **удерживайте** одну из кнопок выбора каналов — **[LEFT]**, **[BOTH]** или **[RIGHT]**.
2. **Переключение между разрядами частоты:** Для выбора изменяемого разряда (курсор на экране подчеркивает текущий шаг) нажимайте кнопки **[LEFT]** (шаг влево / увеличение разряда) и **[RIGHT]** (шаг вправо / уменьшение разряда).
3. **Изменение значения разряда:** Вращайте ручку энкодера для увеличения или уменьшения значения выбранного разряда частоты.
4. **Выход из режима установки частоты:** Нажмите тактовую кнопку **[BOTH]** для фиксации изменений и возврата к обычному отображению.

---

### 2. Режимы работы прибора:

#### ЭКРАН 1. Генератор сигналов (Waves)
Позволяет независимо или совместно настраивать параметры левого и правого каналов:
* Кнопка **[WAVE]** циклически меняет форму сигнала: **Синусоида (Sine)** или **Меандр (Square)**.
* На вкладке правого канала (RIGHT) доступна ручная установка сдвига фазы относительно левого канала от 0° до 360° с шагом 10°, либо активация режима автоматического непрерывного вращения фазы со скоростью от 1°/сек до 360°/сек.

#### ЭКРАН 2. Генератор тестовых шумов (Noises)
Генерация некоррелированных спецсигналов и шумов. Выбор активного типа сигнала осуществляется коротким нажатием любой из кнопок: [Left], [Both], [Right] или [Wave]:
* **White Noise (Белый шум):** Спектральная плотность постоянна по всему диапазону частот.
* **Pink Noise (Розовый шум):** Спектральная плотность спадает на 3 дБ на октаву.
* **IMD (Intermodulation Distortion Test):** Двухтональный сигнал для оценки интермодуляционных искажений ($f_1$ и $f_2$ в задаваемых пропорциях амплитуды).

#### ЭКРАН 3. MP3 Аудиопроигрыватель (Player)
Воспроизводит Hi-Res аудиофайлы (WAV, MP3, OGG, FLAC) с карты памяти MicroSD (FAT32).
* Поддерживается циклическое проигрывание файлов в текущей папке (`Folder`) или повтор одного трека (`Repeat`).
* Кнопки управления плеером:
  * **[LEFT]** — Предыдущий трек / перемотка назад.
  * **[RIGHT]** — Следующий трек / перемотка вперед.
  * **[BOTH]** — Старт / Cтоп воспроизведения.
  * **[Wave]** — Пауза воспроизведения.

#### ЭКРАН 4. Системные настройки (Settings)
Навигация по списку параметров осуществляется вращением энкодера или нажатием кнопок [LEFT] / [RIGHT]. 
* Для изменения параметра нажмите кнопку энкодера **[Encoder]** (или кнопку **[BOTH]**). Курсор переключится на значение параметра.
* Измените значение вращением энкодера.
* Нажмите кнопку энкодера или **[BOTH]** повторно для сохранения настройки в энергонезависимую память микроконтроллера.

**Некоторые параметры меню:**
* `Phase Rot` (Авторотация фазы): Скорость авторотации от 1°/сек до 360°/сек.
* `IMD f1`, `IMD f2`: Задание опорных частот для интермодуляционного теста.
* `IMD Amplit f1`, `IMD Amplit f2`: Задание соотношения амплитуд двух частот (в сумме должно быть 1.0, например 0.8 и 0.2).
* `Player Mode`: Режим повтора плеера (`Folder` / `Repeat`).
* `Start Scr` (Стартовый экран): Экран, активирующийся при включении прибора (`Main`, `Waves`, `Noises`, `MP3`).

---

⚠️ **Важное примечание:** В данном разделе «Быстрый старт» приведены лишь базовые принципы управления и краткое описание основных функций. Устройство обладает гораздо более широкими возможностями. Детальное описание со всеми режимами работы, настройками и тонкостями эксплуатации приведено в руководстве пользователя: [`docs/User manual.pdf`](docs/User%20manual.pdf).

---

### Поддержка
Если Вы считаете этот проект полезным и хотите поддержать автора, Вы можете сделать пожертвование любым из следующих способов:

- **PayPal**: [gia@gia.org.ua] [Donate via PayPal](https://www.paypal.me)  
- **Ko-fi**: [Donate on Ko-fi](https://ko-fi.com/igorgimelfarb)  
- **Monobank**: Отсканируйте QR-код или воспользуйтесь ссылкой: [Поддержать через Monobank](https://send.monobank.ua/jar/8HG6A3VPaW)

![Поддержать через Monobank](images/monobank_DIY_QR.png)

**Ваша поддержка будет очень ценной!**

---

## ⚖️ Лицензия

Этот проект распространяется под свободной лицензией **MIT**. Вы можете свободно использовать, модифицировать и распространять данный проект как в некоммерческих, так и в коммерческих целях, при условии сохранения упоминания авторства. Подробности см. в файле [LICENSE](LICENSE).

---
*Автор проекта: Гимельфарб Игорь (2026 г.)*

---

###### Теги / Ключевые слова:
`генератор звуковых частот ESP32`, `DDS генератор синуса`, `ЦАП PCM5102A схема`, `аудиоплеер на ESP32-S3`, `измерение искажений усилителя`, `генератор розового шума`, `тестирование мостовых усилителей`, `радиолюбительские приборы DIY`
