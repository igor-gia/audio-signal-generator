#include "Config.h"
#include <Preferences.h>

AppSettings cfg;

void config_load() {
    Preferences prefs;
    prefs.begin("app_cfg", true); // true = режим только для чтения (ReadOnly)
    cfg.sweepStartF      = prefs.getFloat("sw_start", 20.0f);
    cfg.sweepStopF       = prefs.getFloat("sw_stop", 150.0f);
    cfg.sweepDuration    = prefs.getFloat("sw_time", 10.0f);
    cfg.phaseRotDuration = prefs.getFloat("ph_time", 5.0f);
    cfg.bootFreq         = prefs.getDouble("boot_freq", 1000.0);
    cfg.bootLevelDb      = prefs.getInt("boot_level", -10);
    cfg.startScreen      = prefs.getInt("start_scr", 0);
    cfg.imdFreq1         = prefs.getFloat("imd_f1", 60.0f);
    cfg.imdFreq2         = prefs.getFloat("imd_f2", 7000.0f);
    cfg.imdRatio1        = prefs.getFloat("imd_r1", 0.8f); // 4/5 амплитуды
    cfg.imdRatio2        = prefs.getFloat("imd_r2", 0.2f); // 1/5 амплитуды
    cfg.playModeLoop     = prefs.getBool("pl_loop", false);
    prefs.end();
}

void config_save() {
    Preferences prefs;
    prefs.begin("app_cfg", false); // false = чтение/запись
    prefs.putFloat("sw_start", cfg.sweepStartF);
    prefs.putFloat("sw_stop", cfg.sweepStopF);
    prefs.putFloat("sw_time", cfg.sweepDuration);
    prefs.putFloat("ph_time", cfg.phaseRotDuration);
    prefs.putDouble("boot_freq", cfg.bootFreq);
    prefs.putInt("boot_level", cfg.bootLevelDb);
    prefs.putInt("start_scr", cfg.startScreen);
    prefs.putFloat("imd_f1", cfg.imdFreq1);
    prefs.putFloat("imd_f2", cfg.imdFreq2);
    prefs.putFloat("imd_r1", cfg.imdRatio1);
    prefs.putFloat("imd_r2", cfg.imdRatio2);
    prefs.putBool("pl_loop", cfg.playModeLoop);
    prefs.end();
}