#include "ui.h"
#include "generator.h"
#include "player.h"
#include <U8g2lib.h>
#include <Wire.h>

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/-1);

DeviceState dev = {
  MODE_BOTH,
  { 1000.0, WAVE_SIN, SUBMODE_STATIC },
  { 1000.0, WAVE_SIN, SUBMODE_STATIC },
  0,
  false,
  -10,
  -10,
  false,
  0
};

static const double DIGIT_WEIGHTS[] = { 0.1, 1.0, 10.0, 100.0, 1000.0, 10000.0 };

static void sync_ui_to_generator() {
  gen_set_submode((int)dev.left.subMode, (int)dev.right.subMode);
  gen_set_channels_wave((int)dev.left.wave, (int)dev.right.wave);

  float target_L = (dev.left.subMode == SUBMODE_SWEEP) ? gen_get_current_frequency_L() : (float)dev.left.freq;
  float target_R = (dev.right.subMode == SUBMODE_SWEEP) ? gen_get_current_frequency_R() : (float)dev.right.freq;
  gen_set_channels_frequency(target_L, target_R);

  gen_set_phase((float)dev.phaseAngle);
  gen_set_volume(dev.volume);
  gen_set_mute(dev.isMute);
}

static void drawHeader(const char* chMode, const char* phaseStr, const char* statusStr) {
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.drawStr(0, 10, chMode);

  if (phaseStr && phaseStr[0] != '\0') {
    u8g2.drawStr(50, 10, phaseStr);
  }

  int strWidth = u8g2.getStrWidth(statusStr);
  u8g2.drawStr(128 - strWidth, 10, statusStr);
  u8g2.drawHLine(0, 12, 128);
}

static void formatFreq(double f, char* outStr) {
  dtostrf(f, 7, 1, outStr);
  for (int i = 0; i < 7; i++) {
    if (outStr[i] == ' ') outStr[i] = ' ';
  }
}

void switchScreen(AppScreen target);

class Screen {
protected:
  bool isDirty = true;
public:
  virtual ~Screen() {}
  virtual void tick() {}
  virtual void onEnter() {
    isDirty = true;
  }
  virtual void onExit() {}
  virtual void handleEvent(IOEvent ev) = 0;
  virtual void render() = 0;

  virtual bool needsRender() {
    return isDirty;
  }
  void clearDirty() {
    isDirty = false;
  }
  void setDirty() {
    isDirty = true;
  }
};

class MainMenuScreen : public Screen {
private:
  int menuIndex = 0;
  const char* menuItems[4] = {
    "Waves generator",
    "Noises",
    "MP3 Player",
    "Settings"
  };

public:
  void onEnter() override {
    menuIndex = 0;
    Screen::onEnter();
    //gen_set_submode(SUBMODE_STATIC, SUBMODE_STATIC);
  }

  void handleEvent(IOEvent ev) override {
    switch (ev) {
      case IO_ENC_ROT_RIGHT:
      case IO_BTN_RIGHT_CLICK:
        menuIndex = (menuIndex + 1) % 4;
        setDirty();
        break;

      case IO_ENC_ROT_LEFT:
      case IO_BTN_LEFT_CLICK:
        menuIndex = (menuIndex + 3) % 4;
        setDirty();
        break;

      case IO_ENC_BTN_CLICK:
      case IO_BTN_BOTH_CLICK:
        if (menuIndex == 0) switchScreen(SCREEN_WAVES);
        else if (menuIndex == 1) switchScreen(SCREEN_NOISES_MENU);
        else if (menuIndex == 2) switchScreen(SCREEN_MP3_PLAYER);
        else if (menuIndex == 3) switchScreen(SCREEN_SETTINGS);
        break;

      default:
        break;
    }
  }

  void render() override {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(36, 10, "MAIN MENU");
    u8g2.drawHLine(0, 12, 128);

    u8g2.setFont(u8g2_font_7x14_tf);
    for (int i = 0; i < 4; i++) {
      int yPos = 28 + (i * 12);
      if (i == menuIndex) {
        u8g2.setDrawColor(1);
        u8g2.drawBox(0, yPos - 11, 128, 13);
        u8g2.setDrawColor(0);
        u8g2.drawStr(6, yPos, menuItems[i]);
        u8g2.setDrawColor(1);
      } else {
        u8g2.drawStr(6, yPos, menuItems[i]);
      }
    }
    u8g2.sendBuffer();
  }
};

class WavesScreen : public Screen {
private:
  int activeCursorDigit = -1;
  bool isPhaseFocus = false;
  unsigned long focusTimer = 0;
  unsigned long lastAutoRender = 0;

  void adjustFrequency(ChannelSettings& chan, int direction) {
    if (activeCursorDigit < 0) return;
    double delta = DIGIT_WEIGHTS[activeCursorDigit] * direction;
    chan.freq += delta;
    if (chan.freq > FREQ_MAX) chan.freq = FREQ_MAX;
    if (chan.freq < FREQ_MIN) chan.freq = FREQ_MIN;
  }

public:
  void tick() override {
    gen_tick();
  }
  
  void onEnter() override {
    gen_init();
    gen_set_submode(SUBMODE_STATIC, SUBMODE_STATIC);
    Screen::onEnter();
    if (dev.chMode == MODE_BOTH) {
      dev.right.subMode = dev.left.subMode;
    }
    sync_ui_to_generator();
  }

  void onExit() override {
    activeCursorDigit = -1;
    isPhaseFocus = false;
    gen_deinit();
  }

  bool needsRender() override {
    if (isDirty) return true;

    bool isSweepActive = (dev.left.subMode == SUBMODE_SWEEP) || (dev.right.subMode == SUBMODE_SWEEP);
    bool isRotActive = (dev.right.subMode == SUBMODE_PHASE_ROT);

    if ((isSweepActive || isRotActive) && (millis() - lastAutoRender > 70)) {
      lastAutoRender = millis();
      return true;
    }
    return false;
  }

  void handleEvent(IOEvent ev) override {
    unsigned long now = millis();

    if (ev == IO_BTN_MODE_LONG) {
      switchScreen(SCREEN_MAIN_MENU);
      return;
    }

    if ((activeCursorDigit >= 0) && !isPhaseFocus && (now - focusTimer > FOCUS_TIMEOUT)) {
      activeCursorDigit = -1;
      setDirty();
    }

    if (ev == IO_NONE) return;
    focusTimer = now;

    switch (ev) {
      case IO_BTN_BOTH_LONG:
      case IO_BTN_LEFT_LONG:
      case IO_BTN_RIGHT_LONG:
        if (activeCursorDigit < 0) {
          if (dev.chMode == MODE_RIGHT && dev.right.subMode == SUBMODE_PHASE_SHIFT) {
            isPhaseFocus = true;
            activeCursorDigit = -1;
          } else {
            activeCursorDigit = 1;
            isPhaseFocus = false;
          }
          setDirty();
        }
        break;

      case IO_BTN_BOTH_CLICK:
        if (activeCursorDigit >= 0 || isPhaseFocus) {
          activeCursorDigit = -1;
          isPhaseFocus = false;
        } else {
          if (dev.chMode != MODE_BOTH) {
            dev.chMode = MODE_BOTH;
            dev.right.freq = dev.left.freq;
            dev.right.wave = dev.left.wave;
            if (dev.left.subMode == SUBMODE_PHASE_SHIFT || dev.left.subMode == SUBMODE_PHASE_ROT) {
              dev.left.subMode = SUBMODE_STATIC;
            }
            dev.right.subMode = dev.left.subMode;
          }
        }
        sync_ui_to_generator();
        setDirty();
        break;

      case IO_BTN_LEFT_CLICK:
        if (activeCursorDigit >= 0) {
          activeCursorDigit++;
          if (activeCursorDigit > 5) activeCursorDigit = 0;
        } else {
          if (dev.chMode != MODE_LEFT) dev.chMode = MODE_LEFT;
        }
        sync_ui_to_generator();
        setDirty();
        break;

      case IO_BTN_RIGHT_CLICK:
        if (activeCursorDigit >= 0) {
          activeCursorDigit--;
          if (activeCursorDigit < 0) activeCursorDigit = 5;
        } else {
          if (dev.chMode != MODE_RIGHT) dev.chMode = MODE_RIGHT;
        }
        sync_ui_to_generator();
        setDirty();
        break;

      case IO_BTN_MODE_CLICK:
        activeCursorDigit = -1;
        isPhaseFocus = false;

        if (dev.chMode == MODE_BOTH) {
          if (dev.left.subMode == SUBMODE_STATIC) dev.left.subMode = SUBMODE_SWEEP;
          else dev.left.subMode = SUBMODE_STATIC;
          dev.right.subMode = dev.left.subMode;
        } else if (dev.chMode == MODE_LEFT) {
          if (dev.left.subMode == SUBMODE_STATIC) dev.left.subMode = SUBMODE_SWEEP;
          else dev.left.subMode = SUBMODE_STATIC;
        } else if (dev.chMode == MODE_RIGHT) {
          if (dev.right.subMode == SUBMODE_STATIC) dev.right.subMode = SUBMODE_SWEEP;
          else if (dev.right.subMode == SUBMODE_SWEEP) {
            dev.right.subMode = SUBMODE_PHASE_SHIFT;
            isPhaseFocus = true;
          } else if (dev.right.subMode == SUBMODE_PHASE_SHIFT) dev.right.subMode = SUBMODE_PHASE_ROT;
          else dev.right.subMode = SUBMODE_STATIC;
        }
        sync_ui_to_generator();
        setDirty();
        break;

      case IO_BTN_WAVE_CLICK:
        if (dev.chMode == MODE_BOTH) {
          dev.left.wave = (dev.left.wave == WAVE_SIN) ? WAVE_SQR : WAVE_SIN;
          dev.right.wave = dev.left.wave;
        } else if (dev.chMode == MODE_LEFT) {
          dev.left.wave = (dev.left.wave == WAVE_SIN) ? WAVE_SQR : WAVE_SIN;
        } else if (dev.chMode == MODE_RIGHT) {
          dev.right.wave = (dev.right.wave == WAVE_SIN) ? WAVE_SQR : WAVE_SIN;
        }
        sync_ui_to_generator();
        setDirty();
        break;

      case IO_ENC_BTN_CLICK:
        dev.isMute = !dev.isMute;
        gen_set_mute(dev.isMute);
        setDirty();
        break;

      case IO_ENC_ROT_RIGHT:
        if (isPhaseFocus) {
          dev.phaseAngle += PHASE_STEP;
          if (dev.phaseAngle >= 360) dev.phaseAngle = 0;
          gen_set_phase((float)dev.phaseAngle);
        } else if (activeCursorDigit >= 0) {
          if (dev.chMode == MODE_BOTH || dev.chMode == MODE_LEFT) adjustFrequency(dev.left, 1);
          else if (dev.chMode == MODE_RIGHT) adjustFrequency(dev.right, 1);
          if (dev.chMode == MODE_BOTH) dev.right.freq = dev.left.freq;
          gen_set_channels_frequency((float)dev.left.freq, (float)dev.right.freq);
        } else {
          if (dev.volume < 0) {
            dev.volume += 1;
            gen_set_volume(dev.volume);
          }
        }
        setDirty();
        break;

      case IO_ENC_ROT_LEFT:
        if (isPhaseFocus) {
          dev.phaseAngle -= PHASE_STEP;
          if (dev.phaseAngle < 0) dev.phaseAngle = 360 - PHASE_STEP;
          gen_set_phase((float)dev.phaseAngle);
        } else if (activeCursorDigit >= 0) {
          if (dev.chMode == MODE_BOTH || dev.chMode == MODE_LEFT) adjustFrequency(dev.left, -1);
          else if (dev.chMode == MODE_RIGHT) adjustFrequency(dev.right, -1);
          if (dev.chMode == MODE_BOTH) dev.right.freq = dev.left.freq;
          gen_set_channels_frequency((float)dev.left.freq, (float)dev.right.freq);
        } else {
          if (dev.volume > -60) {
            dev.volume -= 1;
            gen_set_volume(dev.volume);
          }
        }
        setDirty();
        break;
      default:
        break;
    }
  }

  void render() override {
    u8g2.clearBuffer();

    char dbBuf[16];
    if (dev.isMute) {
      sprintf(dbBuf, "MUTE");
    } else {
      sprintf(dbBuf, "%d dB", dev.volume);
    }

    char freqBuf[16];
    double currentFreqL = gen_get_current_frequency_L();
    double currentFreqR = gen_get_current_frequency_R();

    if (dev.chMode == MODE_BOTH) {
      drawHeader("BOTH CH", "", dbBuf);
      formatFreq(currentFreqL, freqBuf);
      u8g2.setFont(u8g2_font_10x20_mn);
      u8g2.drawStr(10, 35, freqBuf);
      u8g2.setFont(u8g2_font_6x10_tf);
      u8g2.drawStr(84, 35, "Hz");

      char modeBuf[16];
      const char* waveTxt = (dev.left.wave == WAVE_SIN) ? "SIN" : "SQR";
      const char* subTxt = (dev.left.subMode == SUBMODE_SWEEP) ? "SWP" : "FIX";
      sprintf(modeBuf, "%s  %s", waveTxt, subTxt);
      u8g2.drawStr(10, 48, modeBuf);

      if (activeCursorDigit >= 0) {
        int charPos = 6 - activeCursorDigit;
        if (charPos <= 5) charPos--;
        u8g2.setDrawColor(2);
        u8g2.drawBox(10 + (charPos * 10), 18, 10, 21);
        u8g2.setDrawColor(1);
      }

    } else {
      char phaseBuf[16];
      if (dev.right.subMode == SUBMODE_PHASE_ROT) sprintf(phaseBuf, "Ph:ROT");
      else sprintf(phaseBuf, "Ph:%d\xb0", dev.phaseAngle);
      drawHeader("SPL CH", phaseBuf, dbBuf);

      if (dev.chMode == MODE_LEFT) {
        u8g2.setFont(u8g2_font_9x15_tf);
        u8g2.drawStr(0, 29, ">L:");
        formatFreq(currentFreqL, freqBuf);
        u8g2.setFont(u8g2_font_9x15_mn);
        u8g2.drawStr(26, 29, freqBuf);
        u8g2.setFont(u8g2_font_6x10_tf);
        u8g2.drawStr(92, 29, "Hz");
        char subBufL[16];
        sprintf(subBufL, "%s  %s", (dev.left.wave == WAVE_SIN) ? "SIN" : "SQR", (dev.left.subMode == SUBMODE_SWEEP) ? "SWP" : "FIX");
        u8g2.drawStr(26, 40, subBufL);

        if (activeCursorDigit >= 0) {
          int charPos = 6 - activeCursorDigit;
          if (charPos <= 5) charPos--;
          u8g2.setDrawColor(2);
          u8g2.drawBox(26 + (charPos * 9), 15, 9, 16);
          u8g2.setDrawColor(1);
        }
        u8g2.setFont(u8g2_font_6x10_tf);
        u8g2.drawStr(0, 51, "R:");
        formatFreq(currentFreqR, freqBuf);
        u8g2.drawStr(26, 51, freqBuf);
        u8g2.drawStr(92, 51, "Hz");
        char subBufR[16];
        sprintf(subBufR, "%s  %s", (dev.right.wave == WAVE_SIN) ? "SIN" : "SQR", (dev.right.subMode == SUBMODE_SWEEP) ? "SWP" : "FIX");
        u8g2.drawStr(26, 62, subBufR);
      } else {
        u8g2.setFont(u8g2_font_6x10_tf);
        u8g2.drawStr(0, 25, "L:");
        formatFreq(currentFreqL, freqBuf);
        u8g2.drawStr(26, 25, freqBuf);
        u8g2.drawStr(92, 25, "Hz");
        char subBufL[16];
        sprintf(subBufL, "%s  %s", (dev.left.wave == WAVE_SIN) ? "SIN" : "SQR", (dev.left.subMode == SUBMODE_SWEEP) ? "SWP" : "FIX");
        u8g2.drawStr(26, 36, subBufL);

        u8g2.setFont(u8g2_font_9x15_tf);
        u8g2.drawStr(0, 52, ">R:");
        formatFreq(currentFreqR, freqBuf);
        u8g2.setFont(u8g2_font_9x15_mn);
        u8g2.drawStr(26, 52, freqBuf);
        u8g2.setFont(u8g2_font_6x10_tf);
        u8g2.drawStr(92, 52, "Hz");
        char subBufR[16];
        sprintf(subBufR, "%s  %s", (dev.right.wave == WAVE_SIN) ? "SIN" : "SQR", (dev.right.subMode == SUBMODE_SWEEP) ? "SWP" : "FIX");
        u8g2.drawStr(26, 63, subBufR);

        if (activeCursorDigit >= 0) {
          int charPos = 6 - activeCursorDigit;
          if (charPos <= 5) charPos--;
          u8g2.setDrawColor(2);
          u8g2.drawBox(26 + (charPos * 9), 38, 9, 16);
          u8g2.setDrawColor(1);
        }
      }
    }

    if (isPhaseFocus) {
      u8g2.setDrawColor(2);
      u8g2.drawBox(48, 1, 44, 11);
      u8g2.setDrawColor(1);
    }
    u8g2.sendBuffer();
  }
};

class NoisesScreen : public Screen {
private:
  int noiseIndex = 0;
  const char* noiseItems[3] = {
    "White Noise",
    "Pink Noise",
    "IMD Test"
  };

public:
  void tick() override {
    gen_tick();
  }

  void onEnter() override {
    gen_init();
    Screen::onEnter();
    dev.chMode = MODE_BOTH;
    updateNoiseSubmode();
  }

  void onExit() override {
    dev.left.subMode = SUBMODE_STATIC;
    dev.right.subMode = SUBMODE_STATIC;
    gen_deinit();
  }

  void updateNoiseSubmode() {
    if (noiseIndex == 0) dev.left.subMode = SUBMODE_WHITE_NOISE;
    else if (noiseIndex == 1) dev.left.subMode = SUBMODE_PINK_NOISE;
    else if (noiseIndex == 2) dev.left.subMode = SUBMODE_IMD;

    dev.right.subMode = dev.left.subMode;
    sync_ui_to_generator();
  }

  void handleEvent(IOEvent ev) override {
    switch (ev) {
      case IO_BTN_MODE_LONG:
        switchScreen(SCREEN_MAIN_MENU);
        return;

      case IO_BTN_RIGHT_CLICK:
        noiseIndex = (noiseIndex + 1) % 3;
        updateNoiseSubmode();
        setDirty();
        break;

      case IO_BTN_LEFT_CLICK:
        noiseIndex = (noiseIndex + 2) % 3;
        updateNoiseSubmode();
        setDirty();
        break;

      case IO_BTN_BOTH_CLICK:
      case IO_BTN_WAVE_CLICK:
        noiseIndex = (noiseIndex + 1) % 3;
        updateNoiseSubmode();
        setDirty();
        break;

      case IO_ENC_BTN_CLICK:
        dev.isMute = !dev.isMute;
        gen_set_mute(dev.isMute);
        setDirty();
        break;

      case IO_ENC_ROT_RIGHT:
        if (dev.volume < 0) {
          dev.volume += 1;
          gen_set_volume(dev.volume);
          setDirty();
        }
        break;

      case IO_ENC_ROT_LEFT:
        if (dev.volume > -60) {
          dev.volume -= 1;
          gen_set_volume(dev.volume);
          setDirty();
        }
        break;

      default:
        break;
    }
  }

  void render() override {
    u8g2.clearBuffer();

    char dbBuf[16];
    if (dev.isMute) {
      sprintf(dbBuf, "MUTE");
    } else {
      sprintf(dbBuf, "%d dB", dev.volume);
    }

    drawHeader("NOISES & TESTS", "", dbBuf);

    u8g2.setFont(u8g2_font_7x14_tf);
    for (int i = 0; i < 3; i++) {
      int yPos = 28 + (i * 14);
      if (i == noiseIndex) {
        u8g2.setDrawColor(1);
        u8g2.drawBox(0, yPos - 11, 128, 13);
        u8g2.setDrawColor(0);
        u8g2.drawStr(6, yPos, noiseItems[i]);
        u8g2.setDrawColor(1);
      } else {
        u8g2.drawStr(6, yPos, noiseItems[i]);
      }
    }

    u8g2.sendBuffer();
  }
};

class MP3PlayerScreen : public Screen {
private:
  int browserIndex = 0;          
  bool sdInitSuccess = false;
  bool leavingToMainMenu = false;

public:
  void onEnter() override {
    Screen::onEnter();
    browserIndex = 0;
    //gen_deinit();
    sdInitSuccess = player.sdInit();
    player.init();
    player.setMute(dev.isMute);
    player.setVolumeDb(dev.volume);
  }

  void onExit() override {
    if (leavingToMainMenu) {
      delay(200);
      player.stop();
      leavingToMainMenu = false;
    }
  }

  void handleEvent(IOEvent ev) override {
    if (ev == IO_BTN_MODE_LONG) {
      leavingToMainMenu = true;
      switchScreen(SCREEN_MAIN_MENU);
      return;
    }

    if (!sdInitSuccess) return;

    int itemsCount = player.getItemsCount();

    switch (ev) {
      case IO_BTN_LEFT_CLICK:
        if (itemsCount > 0) {
          browserIndex--;
          if (browserIndex < 0) browserIndex = itemsCount - 1;
          setDirty();
        }
        break;

      case IO_BTN_RIGHT_CLICK:
        if (itemsCount > 0) {
          browserIndex++;
          if (browserIndex >= itemsCount) browserIndex = 0;
          setDirty();
        }
        break;

      case IO_BTN_BOTH_CLICK: {
        if (itemsCount == 0) break;
        const FileBrowserItem* item = player.getItemSorted(browserIndex);
        if (!item) break;

        if (item->type == ITEM_UP_DIR) {
          player.exitToParent();
          browserIndex = 0;
          setDirty();
        } else if (item->type == ITEM_DIRECTORY) {
          player.enterDir(item->name);
          browserIndex = 0;
          setDirty();
        } else if (item->type == ITEM_FILE_AUDIO) {
          player.playSortedIndex(browserIndex);
          switchScreen(SCREEN_MP3_PLAYING);
        }
        break;
      }

      case IO_ENC_ROT_RIGHT:
        if (dev.volume < 0) {
          dev.volume += 1;
          player.setVolumeDb(dev.volume);
          setDirty();
        }
        break;

      case IO_ENC_ROT_LEFT:
        if (dev.volume > -60) {
          dev.volume -= 1;
          player.setVolumeDb(dev.volume);
          setDirty();
        }
        break;

      case IO_ENC_BTN_CLICK:
        dev.isMute = !dev.isMute;
        player.setMute(dev.isMute);
        setDirty();
        break;

      default:
        break;
    }
  }

  void render() override {
    u8g2.clearBuffer();
    
    char dbBuf[16];
    if (dev.isMute) {
      strcpy(dbBuf, "MUTE");
    } else {
      sprintf(dbBuf, "%d dB", dev.volume);
    }

    int itemsCount = player.getItemsCount();
    int totalPages = (itemsCount + 4) / 5;
    if (totalPages == 0) totalPages = 1;
    int currentPage = (browserIndex / 5) + 1;

    char pageBuf[16];
    sprintf(pageBuf, "Page %d/%d", currentPage, totalPages);

    drawHeader("SD List", pageBuf, dbBuf);

    if (!sdInitSuccess) {
      u8g2.setFont(u8g2_font_6x10_tf);
      u8g2.drawStr(10, 35, "SD Card Init Failed!");
      u8g2.sendBuffer();
      return;
    }

    u8g2.setFont(u8g2_font_6x10_tf);

    int startRenderIdx = (browserIndex / 5) * 5;

    for (int i = 0; i < 5; i++) {
      int currentItemIdx = startRenderIdx + i;
      if (currentItemIdx >= itemsCount) break;

      const FileBrowserItem* item = player.getItemSorted(currentItemIdx);
      if (!item) continue;

      int yPos = 23 + (i * 10);

      if (currentItemIdx == browserIndex) {
        u8g2.setDrawColor(1);
        u8g2.drawBox(0, yPos - 8, 128, 10);
        u8g2.setDrawColor(0); 
      } else {
        u8g2.setDrawColor(1);
      }

      if (item->type == ITEM_DIRECTORY) {
        u8g2.drawStr(2, yPos, "/");
        u8g2.drawStr(12, yPos, item->name);
      } else if (item->type == ITEM_UP_DIR) {
        u8g2.drawStr(2, yPos, "..");
      } else {
        u8g2.drawStr(12, yPos, item->name);
      }
    }
    u8g2.setDrawColor(1);
    u8g2.sendBuffer();
  }
};

class MP3PlayingScreen : public Screen {
private:
  unsigned long lastTick = 0;

public:
  void tick() override {
    if (player.isPlaying() && !player.isPaused()) {
      player.tick();
    }
  }

 
  void onExit() override {
    delay(200);
    player.stop();
  }

  bool needsRender() override {
    if (isDirty) return true;
    if (player.isPlaying() && !player.isPaused() && (millis() - lastTick > 1000)) {
      lastTick = millis();
      return true;
    }
    return false;
  }

  void handleEvent(IOEvent ev) override {
    if (ev == IO_BTN_MODE_LONG) {
      switchScreen(SCREEN_MAIN_MENU);
      return;
    }

    switch (ev) {
      case IO_BTN_BOTH_CLICK: // Кнопка STOP
        //player.stop();
        switchScreen(SCREEN_MP3_PLAYER);
        break;

      case IO_BTN_WAVE_CLICK: // Кнопка PAUSE / PLAY
        if (player.isPlaying()) {
          player.togglePause();
          setDirty();
        }
        break;

      case IO_BTN_LEFT_CLICK: // Предыдущий трек
        player.prev();
        setDirty();
        break;

      case IO_BTN_RIGHT_CLICK: // Следующий трек
        player.next();
        setDirty();
        break;

      case IO_ENC_ROT_RIGHT:
        if (dev.volume < 0) {
          dev.volume += 1;
          player.setVolumeDb(dev.volume);
          setDirty();
        }
        break;

      case IO_ENC_ROT_LEFT:
        if (dev.volume > -60) {
          dev.volume -= 1;
          player.setVolumeDb(dev.volume);
          setDirty();
        }
        break;

      case IO_ENC_BTN_CLICK:
        dev.isMute = !dev.isMute;
        player.setMute(dev.isMute);
        setDirty();
        break;

      default:
        break;
    }
  }

void render() override {
    u8g2.clearBuffer();

    char dbBuf[16];
    if (dev.isMute) {
      strcpy(dbBuf, "MUTE");
    } else {
      sprintf(dbBuf, "%d dB", dev.volume);
    }

    const char* statusStr = player.isPaused() ? "PAUSED" : "PLAY";
    drawHeader("MP3", statusStr, dbBuf);

    TrackInfo info = {0};
    player.getCurrentTrackInfo(&info);

    // Строка 1: Имя файла (крупно)
    u8g2.setFont(u8g2_font_7x14_tf);
    u8g2.drawStr(0, 26, info.name);

    // Переключаем шрифт на 6x10 для служебных строк
    u8g2.setFont(u8g2_font_6x10_tf);
    
    // Строка 2: Кодек / Битрейт (например, FLAC / 6800 kbps)
    char formatBuf[48];
    sprintf(formatBuf, "%s / %d kbps", info.codec, info.bitrate / 1000);
    u8g2.drawStr(0, 38, formatBuf);

    // Строка 3: Частота дискретизации (Sample)
    char sampleBuf[32];
    sprintf(sampleBuf, "Sample:  %d Hz", info.sampleRate);
    u8g2.drawStr(0, 49, sampleBuf);

    // Строка 4: Текущее время / Общее время трека
    uint32_t curTime = player.getAudioCurrentTime();
    uint32_t totalTime = info.duration;
    
    char timeBuf[48]; 
    sprintf(timeBuf, "Time: %02d:%02d / %02d:%02d", 
            (int)(curTime / 60), (int)(curTime % 60), 
            (int)(totalTime / 60), (int)(totalTime % 60));
    u8g2.drawStr(0, 60, timeBuf);

    u8g2.sendBuffer();
  }
};

class SettingsScreen : public Screen {
private:
  int selectIndex = 0;
  bool isEditing = false;

public:
  void onEnter() override {
    isEditing = false;
    selectIndex = 0;
    Screen::onEnter();
  }

  void onExit() override {
    config_save();
  }

  void handleEvent(IOEvent ev) override {
    if (!isEditing) {
      switch (ev) {
        case IO_BTN_MODE_LONG:
          switchScreen(SCREEN_MAIN_MENU);
          return;

        case IO_ENC_ROT_RIGHT:
        case IO_BTN_RIGHT_CLICK:
          if (selectIndex < 11) {
            selectIndex++;
            setDirty();
          }
          break;

        case IO_ENC_ROT_LEFT:
        case IO_BTN_LEFT_CLICK:
          if (selectIndex > 0) {
            selectIndex--;
            setDirty();
          }
          break;

        case IO_ENC_BTN_CLICK:
        case IO_BTN_BOTH_CLICK:
          isEditing = true;
          setDirty();
          break;

        default: break;
      }
    } else {
      switch (ev) {
        case IO_ENC_BTN_CLICK:
        case IO_BTN_BOTH_CLICK:
          isEditing = false;
          setDirty();
          return;

        case IO_ENC_ROT_RIGHT:
          adjustValue(1);
          setDirty();
          break;

        case IO_ENC_ROT_LEFT:
          adjustValue(-1);
          setDirty();
          break;

        default: break;
      }
    }
  }

  void render() override {
    u8g2.clearBuffer();

    int page = (selectIndex / 3) + 1;
    char pageBuf[16];
    sprintf(pageBuf, "Page %d/4", page);
    drawHeader("SETTINGS", "", pageBuf);

    int startIdx = (page - 1) * 3;
    u8g2.setFont(u8g2_font_7x14_tf);

    for (int i = 0; i < 3; i++) {
      int currentIdx = startIdx + i;
      if (currentIdx > 11) break;

      int yPos = 28 + (i * 16);
      bool isCurrent = (currentIdx == selectIndex);

      char nameBuf[16];
      char valBuf[16];
      getParamStrings(currentIdx, nameBuf, valBuf);

      if (isCurrent) {
        if (isEditing) {
          u8g2.drawStr(6, yPos, nameBuf);
          int valWidth = u8g2.getStrWidth(valBuf);

          u8g2.setDrawColor(1);
          u8g2.drawBox(124 - valWidth, yPos - 11, valWidth + 3, 13);
          u8g2.setDrawColor(0);
          u8g2.drawStr(125 - valWidth, yPos, valBuf);
          u8g2.setDrawColor(1);
        } else {
          u8g2.setDrawColor(1);
          u8g2.drawBox(0, yPos - 11, 128, 14);
          u8g2.setDrawColor(0);
          u8g2.drawStr(2, yPos, ">");
          u8g2.drawStr(10, yPos, nameBuf);
          int valWidth = u8g2.getStrWidth(valBuf);
          u8g2.drawStr(125 - valWidth, yPos, valBuf);
          u8g2.setDrawColor(1);
        }
      } else {
        u8g2.drawStr(10, yPos, nameBuf);
        int valWidth = u8g2.getStrWidth(valBuf);
        u8g2.drawStr(125 - valWidth, yPos, valBuf);
      }
    }
    u8g2.sendBuffer();
  }

private:
  void getParamStrings(int idx, char* name, char* val) {
    switch (idx) {
      case 0:
        strcpy(name, "Swp Start");
        sprintf(val, "%dHz", (int)cfg.sweepStartF);
        break;
      case 1:
        strcpy(name, "Swp Stop");
        sprintf(val, "%dHz", (int)cfg.sweepStopF);
        break;
      case 2:
        strcpy(name, "Swp Time");
        sprintf(val, "%ds", (int)cfg.sweepDuration);
        break;
      case 3:
        strcpy(name, "Rot Time");
        sprintf(val, "%ds", (int)cfg.phaseRotDuration);
        break;
      case 4:
        strcpy(name, "Boot Freq");
        sprintf(val, "%dHz", (int)cfg.bootFreq);
        break;
      case 5:
        strcpy(name, "Boot Lvl");
        sprintf(val, "%ddB", cfg.bootLevelDb);
        break;
      case 6:
        strcpy(name, "IMD Freq 1");
        sprintf(val, "%dHz", (int)cfg.imdFreq1);
        break;
      case 7:
        strcpy(name, "IMD Freq 2");
        sprintf(val, "%dHz", (int)cfg.imdFreq2);
        break;
      case 8:
        strcpy(name, "IMD Ratio1");
        dtostrf(cfg.imdRatio1, 4, 2, val);
        break;
      case 9:
        strcpy(name, "IMD Ratio2");
        dtostrf(cfg.imdRatio2, 4, 2, val);
        break;
      case 10:
        strcpy(name, "Play Mode");
        if (cfg.playModeLoop) strcpy(val, "Repeat");
        else strcpy(val, "Folder");
        break;
      case 11:
        strcpy(name, "Start Scr");
        if (cfg.startScreen == 0) strcpy(val, "Main");
        else if (cfg.startScreen == 1) strcpy(val, "Waves");
        else if (cfg.startScreen == 2) strcpy(val, "Noises");
        else if (cfg.startScreen == 3) strcpy(val, "MP3");
        break;
      default: break;
    }
  }

  float getFreqStep(float currentFreq) {
    if (currentFreq < 200.0f) return 10.0f;
    if (currentFreq < 1000.0f) return 50.0f;
    if (currentFreq < 5000.0f) return 200.0f;
    return 1000.0f;
  }

  void adjustValue(int direction) {
    switch (selectIndex) {
      case 0:
        {
          float step = getFreqStep(cfg.sweepStartF);
          cfg.sweepStartF += direction * step;
          if (cfg.sweepStartF < 10) cfg.sweepStartF = 10;
          if (cfg.sweepStartF > 20000) cfg.sweepStartF = 20000;
          break;
        }
      case 1:
        {
          float step = getFreqStep(cfg.sweepStopF);
          cfg.sweepStopF += direction * step;
          if (cfg.sweepStopF < 20) cfg.sweepStopF = 20;
          if (cfg.sweepStopF > 20000) cfg.sweepStopF = 20000;
          break;
        }
      case 2:
        cfg.sweepDuration += direction * 1;
        if (cfg.sweepDuration < 1) cfg.sweepDuration = 1;
        if (cfg.sweepDuration > 60) cfg.sweepDuration = 60;
        break;

      case 3:
        cfg.phaseRotDuration += direction * 1;
        if (cfg.phaseRotDuration < 1) cfg.phaseRotDuration = 1;
        if (cfg.phaseRotDuration > 60) cfg.phaseRotDuration = 60;
        break;

      case 4:
        {
          float step = getFreqStep(cfg.bootFreq);
          cfg.bootFreq += direction * step;
          if (cfg.bootFreq < 10) cfg.bootFreq = 10;
          if (cfg.bootFreq > 20000) cfg.bootFreq = 20000;
          break;
        }
      case 5:
        cfg.bootLevelDb += direction * 1;
        if (cfg.bootLevelDb < -60) cfg.bootLevelDb = -60;
        if (cfg.bootLevelDb > 0) cfg.bootLevelDb = 0;
        break;

      case 6:
        {
          float step = getFreqStep(cfg.imdFreq1);
          cfg.imdFreq1 += direction * step;
          if (cfg.imdFreq1 < 10) cfg.imdFreq1 = 10;
          if (cfg.imdFreq1 > 22000) cfg.imdFreq1 = 22000;
          break;
        }
      case 7:
        {
          float step = getFreqStep(cfg.imdFreq2);
          cfg.imdFreq2 += direction * step;
          if (cfg.imdFreq2 < 10) cfg.imdFreq2 = 10;
          if (cfg.imdFreq2 > 22000) cfg.imdFreq2 = 22000;
          break;
        }
      case 8:
        cfg.imdRatio1 += direction * 0.05f;
        if (cfg.imdRatio1 < 0.0f) cfg.imdRatio1 = 0.0f;
        if (cfg.imdRatio1 > 1.0f) cfg.imdRatio1 = 1.0f;
        break;

      case 9:
        cfg.imdRatio2 += direction * 0.05f;
        if (cfg.imdRatio2 < 0.0f) cfg.imdRatio2 = 0.0f;
        if (cfg.imdRatio2 > 1.0f) cfg.imdRatio2 = 1.0f;
        break;

      case 10:
        cfg.playModeLoop = !cfg.playModeLoop;
        break;

      case 11:
        cfg.startScreen += direction;
        if (cfg.startScreen < 0) cfg.startScreen = 3;
        if (cfg.startScreen > 3) cfg.startScreen = 0;
        break;
    }
  }
};

static MainMenuScreen screenMainMenu;
static WavesScreen screenWaves;
static NoisesScreen screenNoises;
static MP3PlayerScreen screenMP3Player;
static MP3PlayingScreen screenMP3Playing;
static SettingsScreen screenSettings;

static Screen* currentScreen = nullptr;

void switchScreen(AppScreen target) {
  if (currentScreen) currentScreen->onExit();

  switch (target) {
    case SCREEN_MAIN_MENU: currentScreen = &screenMainMenu; break;
    case SCREEN_WAVES: currentScreen = &screenWaves; break;
    case SCREEN_NOISES_MENU: currentScreen = &screenNoises; break;
    case SCREEN_MP3_PLAYER: currentScreen = &screenMP3Player; break;
    case SCREEN_MP3_PLAYING: currentScreen = &screenMP3Playing; break;
    case SCREEN_SETTINGS: currentScreen = &screenSettings; break;
    default: break;
  }

  if (currentScreen) currentScreen->onEnter();
}

void ui_init() {
  Wire.begin(SDA_PIN, SCL_PIN, 400000);
  u8g2.begin();

  config_load();

  dev.volume = cfg.bootLevelDb;
  dev.left.freq = cfg.bootFreq;
  dev.right.freq = cfg.bootFreq;

  sync_ui_to_generator();

  switch (cfg.startScreen) {
    case 0: switchScreen(SCREEN_MAIN_MENU); break;
    case 1: switchScreen(SCREEN_WAVES); break;
    case 2: switchScreen(SCREEN_NOISES_MENU); break;
    case 3: switchScreen(SCREEN_MP3_PLAYER); break;
    default: switchScreen(SCREEN_MAIN_MENU); break;
  }

  if (currentScreen) currentScreen->onEnter();
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tf); // любой стандартный шрифт, который у тебя подключен
  u8g2.drawStr(0, 12, "BOOT OK!");
  u8g2.sendBuffer();
}

void ui_update(IOEvent ev) {
  if (currentScreen) {
    currentScreen->handleEvent(ev);
  }

  if (currentScreen && currentScreen->needsRender()) {
    currentScreen->render();
    currentScreen->clearDirty();
  }
}

void ui_tick() {
  if (currentScreen) {
    currentScreen->tick();
  }
}