
#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "Application.h"
#include <pgmspace.h>
#include "extras/www/template.h"

#include "Animation.h"
#include "Background.h"


struct Config {
  uint16_t alert_sweep_speed;
  uint16_t alert_fill_speed;
  uint16_t alert_flash_speed;
  uint32_t alert_intro_color;
  bool alert_intro_glitchy;
  
  uint16_t bg_rainbow_speed;
  uint16_t bg_cycle_speed;
  uint32_t bg_color_color;
  uint16_t bg_beat_speed;
  uint32_t bg_beat_color;
  
  char bg_default[16];
  uint8_t bg_default_len = 16;
};

class Controller {
  public:
    Controller(ESP8266WebServer* server, Animation** pCurrAnim, Background** pCurrBg);

    // LEDs
    bool getLedsOn();
    void setLedsOn(bool on);
    void initBg();

    // Wifi
    void handleRoot();
    void handleAlert(String name = "");
    void handleBg(String name = "");
    void handleSettings();
    void handleSettingsAlert();
    void saveSettingsAlert();
    void handleSettingsBg();
    void saveSettingsBg();
    void handleOff();
    void handleOn();

    // Settings
    bool loadConfig();
    bool saveConfig();
    
    uint16_t getAlertSweepSpeed();
    uint16_t getAlertFillSpeed();
    uint16_t getAlertFlashSpeed();
    uint32_t getAlertIntroColor();
    bool getAlertIntroGlitchy();
    uint16_t getBgRainbowSpeed();
    uint16_t getBgCycleSpeed();
    uint32_t getBgColorColor();
    uint16_t getBgBeatSpeed();
    uint32_t getBgBeatColor();
    char* getBgDefault();
  
  private:
    //LEDs
    bool ledsOn = true;
    Animation** pCurrAnim;
    Background** pCurrBg;
    void startAnimation(Animation* newAnim);
    void startBackground(Background* newBg);

    // Wifi
    ESP8266WebServer* server;
    void setupHandlers();

    // Settings
    Config cfg;
};

uint32_t parseColorCode(String c);
uint32_t parseNumber(String c);
String colorCodeToHex(uint32_t c);
void printConfig(Config &cfg);

#endif
