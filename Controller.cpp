
#include "Controller.h"

Controller::Controller(ESP8266WebServer* server,  Animation** pCurrAnim, Background** pCurrBg) : server(server), pCurrAnim(pCurrAnim), pCurrBg(pCurrBg) {
  this->setupHandlers();
}

// LEDs
bool Controller::getLedsOn() {
  return this->ledsOn;
}

void Controller::setLedsOn(bool on) {
  this->ledsOn = on;
}

void Controller::initBg() {
  Background* bg = NULL;
  String bg_default = String(this->getBgDefault());
  
  if(bg_default == "rainbow") {
    bg = new RainbowBackground(scale16by8(this->getBgRainbowSpeed(), 38), false);  // Convert from 100% speed to 15bpm (38/256 ~ 15/100)
  }
  else if(bg_default == "cycle") {
    bg = new CycleBackground(scale16by8(this->getBgCycleSpeed(), 38));  // Convert from 100% speed to 15bpm (38/256 ~ 15/100)
  }
  else if(bg_default == "off") {
    bg = new OffBackground();
  }
  else if(bg_default == "color") {
    bg = new ColorBackground(CRGB(this->getBgColorColor()));
  }
  else if(bg_default == "beat") {
    bg = new BeatBackground(CRGB(this->getBgBeatColor()), scale16by8(this->getBgBeatSpeed(), 38));  // Convert from 100% speed to 15bpm (38/256 ~ 15/100)
  }
  
  this->startBackground(bg);
}

void Controller::startAnimation(Animation* newAnim) {
  if(*(this->pCurrAnim) != NULL) {  // Interrupt currently running animation
    delete *(this->pCurrAnim);
    *(this->pCurrAnim) = NULL;
  }
  
  *(this->pCurrAnim) = newAnim;
}

void Controller::startBackground(Background* newBg) {
  if(*(this->pCurrBg) != NULL) {  // Interrupt currently running background
    delete *(this->pCurrBg);
    *(this->pCurrBg) = NULL;
  }
  
  *(this->pCurrBg) = newBg;
}

// Wifi
void Controller::setupHandlers() {
  this->server->on("/",               [this](){ this->handleRoot(); });
  this->server->on("/alert/sweep",    [this](){ this->handleAlert("sweep"); });
  this->server->on("/alert/fill",     [this](){ this->handleAlert("fill"); });
  this->server->on("/alert/flash",    [this](){ this->handleAlert("flash"); });
  this->server->on("/alert/intro",    [this](){ this->handleAlert("intro"); });
  this->server->on("/bg/rainbow",     [this](){ this->handleBg("rainbow"); });
  this->server->on("/bg/cycle",       [this](){ this->handleBg("cycle"); });
  this->server->on("/bg/off",         [this](){ this->handleBg("off"); });
  this->server->on("/bg/color",       [this](){ this->handleBg("color"); });
  this->server->on("/bg/beat",        [this](){ this->handleBg("beat"); });
  
  this->server->on("/settings",       [this](){ this->handleSettings(); });
  this->server->on("/settings/alert", HTTP_GET,  [this](){ this->handleSettingsAlert(); });
  this->server->on("/settings/alert", HTTP_POST, [this](){ this->saveSettingsAlert(); });
  this->server->on("/settings/bg",    HTTP_GET,  [this](){ this->handleSettingsBg(); });
  this->server->on("/settings/bg",    HTTP_POST, [this](){ this->saveSettingsBg(); });
  this->server->on("/off",            [this](){ this->handleOff(); });
  this->server->on("/on",             [this](){ this->handleOn(); });
}

void Controller::handleRoot() {
  Serial.println("handleRoot()");
  
  // Headers
  String page = FPSTR(HTTP_AGS_HEAD);
  page.replace("{v}", "Settings");
  page += FPSTR(HTTP_AGS_SCRIPT);
  page += FPSTR(HTTP_AGS_STYLE);
  page += FPSTR(HTTP_AGS_HEAD_END);

  // Content
  page += FPSTR(HTTP_AGS_TITLE);
  page.replace("{t}", AP_NAME);
  page += String(F("<h3>Home page</h3>"));
  String item = FPSTR(HTTP_AGS_HOME);
  item.replace("{ip}", WiFi.localIP().toString());
  page += item;

  // Footer
  String stickyFooter = FPSTR(HTTP_AGS_FOOTER);
  stickyFooter.replace("{v}", VERSION);
  page += stickyFooter;
  page += FPSTR(HTTP_AGS_END);

  this->server->sendHeader("Content-Length", String(page.length()));
  this->server->send(200, "text/html", page);
}

void Controller::handleAlert(String name) {
  Serial.println("handleAlert() " + name);
  
  if(name == "") {
    this->server->send(204);
    return;
  }
  else if((name == "sweep") || (name == "fill") || (name == "flash"))  {
    // Default parameters
    uint32_t speed = 100;
    CRGB color(0xFF0000);

    if(name == "sweep") {
      speed = this->getAlertSweepSpeed();
    }
    else if(name == "fill") {
      speed = this->getAlertFillSpeed();
    }
    else if(name == "flash") {
      speed = this->getAlertFlashSpeed();
    }

    // Requested parameters (if they exist)
    if(this->server->arg("c") != "") {
      uint32_t colorCode = parseColorCode(this->server->arg("c"));
      if(colorCode <= 0xFFFFFF) {
        color = CRGB(colorCode);
      }
    }
    if(this->server->arg("s") != "") {
      uint32_t number = parseNumber(this->server->arg("s"));
      if((number >= 10) && (number <= 500)) {
        speed = number;
      }
    }

    // Run the animation
    if(name == "sweep") {
      uint32_t total_ms = (uint32_t)6500 * 100 / speed;
      this->startAnimation(new SweepAnimation(total_ms, color));
    }
    else if(name == "fill") {
      uint32_t total_ms = (uint32_t)3250 * 100 / speed;
      this->startAnimation(new FillAnimation(total_ms, color));
    }
    else if(name == "flash") {
      uint32_t total_ms = (uint32_t)3250 * 100 / speed;
      this->startAnimation(new FlashAnimation(total_ms, color));
    }
  }
  else if(name == "intro") {
    // Default parameters
    CRGB color(this->getAlertIntroColor());
    bool glitchy = this->getAlertIntroGlitchy();
    
    // Requested parameters (if they exist)
    if(this->server->arg("c") != "") {
      uint32_t colorCode = parseColorCode(this->server->arg("c"));
      if(colorCode <= 0xFFFFFF) {
        color = CRGB(colorCode);
      }
    }
    if(this->server->arg("g") == "true") {
      glitchy = true;
    }
    
    // Run the animation
    this->startAnimation(new IntroAnimation(glitchy, color));
  }

  this->server->send(204);
}

void Controller::handleBg(String name) {
  Serial.println("handleBg() " + name);
  
  if(name == "") {
    this->server->send(204);
    return;
  }

  // Check name validity
  bool validBgName = false;
  for(uint8_t i = 0; i < Background::num; i++) {  // Check background names
    if(Background::name[i] == name) {
      validBgName = true;
      break;
    }
  }

  if(!validBgName) {
    this->server->send(400, "text/plain", "400: Invalid Request");
    return;
  }

  if(name == "off") {
    // Run the background
      this->startBackground(new OffBackground());
  }
  else if((name == "rainbow") || (name == "cycle"))  {
    // Default parameters
    uint32_t speed = 100;

    if(name == "rainbow") {
      speed = this->getBgRainbowSpeed();
    }
    else if(name == "cycle") {
      speed = this->getBgCycleSpeed();
    }

    // Requested parameters (if they exist)
    if(this->server->arg("s") != "") {
      uint32_t number = parseNumber(this->server->arg("s"));
      if((number >= 10) && (number <= 500)) {
        speed = number;
      }
    }

    // Run the background
    if(name == "rainbow") {
      uint8_t bpm = scale16by8(speed, 38);  // Convert from 100% speed to 15bpm (38/256 ~ 15/100)
      this->startBackground(new RainbowBackground(bpm, false));
    }
    else if(name == "cycle") {
      uint8_t bpm = scale16by8(speed, 38);  // Convert from 100% speed to 15bpm (38/256 ~ 15/100)
      this->startBackground(new CycleBackground(bpm));
    }
  }
  else if(name == "color") {
    // Default parameters
    CRGB color(this->getBgColorColor());
    
    // Requested parameters (if they exist)
    if(this->server->arg("c") != "") {
      uint32_t colorCode = parseColorCode(this->server->arg("c"));
      if(colorCode <= 0xFFFFFF) {
        color = CRGB(colorCode);
      }
    }
    
    // Run the background
    this->startBackground(new ColorBackground(color));
  }
  else if(name == "beat") {
    // Default parameters
    uint32_t speed = this->getBgBeatSpeed();
    CRGB color(this->getBgBeatColor());
    
    // Requested parameters (if they exist)
    if(this->server->arg("s") != "") {
      uint32_t number = parseNumber(this->server->arg("s"));
      if((number >= 10) && (number <= 500)) {
        speed = number;
      }
    }
    if(this->server->arg("c") != "") {
      uint32_t colorCode = parseColorCode(this->server->arg("c"));
      if(colorCode <= 0xFFFFFF) {
        color = CRGB(colorCode);
      }
    }
    
    // Run the background
    uint8_t bpm = scale16by8(speed, 38);  // Convert from 100% speed to 15bpm (38/256 ~ 15/100)
    this->startBackground(new BeatBackground(color, bpm));
  }

  // Copy over to cfg
  strcpy(this->cfg.bg_default, name.c_str());

  this->server->send(204);
}

void Controller::handleSettings() {
  Serial.println("handleSettings()");
  
  // Headers
  String page = FPSTR(HTTP_AGS_HEAD);
  page.replace("{v}", "Settings");
  page += FPSTR(HTTP_AGS_SCRIPT);
  page += FPSTR(HTTP_AGS_STYLE);
  page += FPSTR(HTTP_AGS_HEAD_END);

  // Content
  page += FPSTR(HTTP_AGS_TITLE);
  page.replace("{t}", AP_NAME);
  page += String(F("<h3>Settings</h3>"));
  String buttons = FPSTR(HTTP_AGS_SETTINGS_OPTIONS);
  buttons.replace("{v}", this->getLedsOn() ? "Disable LEDs" : "Enable LEDs");
  buttons.replace("{i}", this->getLedsOn() ? "/off" : "/on");
  page += buttons;

  // Footer
  String stickyFooter = FPSTR(HTTP_AGS_FOOTER);
  stickyFooter.replace("{v}", VERSION);
  page += stickyFooter;
  page += FPSTR(HTTP_AGS_END);

  this->server->sendHeader("Content-Length", String(page.length()));
  this->server->send(200, "text/html", page);
}

void Controller::handleSettingsAlert() {
  Serial.println("handleSettingsAlert()");
  
  // Headers
  String page = FPSTR(HTTP_AGS_HEAD);
  page.replace("{v}", "Alerts settings");
  page += FPSTR(HTTP_AGS_SCRIPT);
  page += FPSTR(HTTP_AGS_STYLE);
  page += FPSTR(HTTP_AGS_HEAD_END);

  // Content
  page += FPSTR(HTTP_AGS_TITLE);
  page.replace("{t}", AP_NAME);
  page += String(F("<h3>Alerts settings</h3>"));
  String item = FPSTR(HTTP_AGS_SETTINGS_FORM_START);
  item.replace("{a}", "/settings/alert"); // Submit action
  item.replace("{cs}", "\"ic\""); // Color fields to convert at submit
  page += item;
  item = FPSTR(HTTP_AGS_SETTINGS_ALERT_SWEEP);
  item.replace("{s}", String(this->getAlertSweepSpeed()));
  page += item;
  item = FPSTR(HTTP_AGS_SETTINGS_ALERT_FILL);
  item.replace("{s}", String(this->getAlertFillSpeed()));
  page += item;
  item = FPSTR(HTTP_AGS_SETTINGS_ALERT_FLASH);
  item.replace("{s}", String(this->getAlertFlashSpeed()));
  page += item;
  item = FPSTR(HTTP_AGS_SETTINGS_ALERT_INTRO);
  item.replace("{g}", this->getAlertIntroGlitchy() ? "checked" : "");
  item.replace("{c}", colorCodeToHex(this->getAlertIntroColor()));
  page += item;
  page += FPSTR(HTTP_AGS_SETTINGS_FORM_END);

  // Footer
  item = FPSTR(HTTP_AGS_FOOTER);
  item.replace("{v}", VERSION);
  page += item;
  page += FPSTR(HTTP_AGS_END);

  this->server->sendHeader("Content-Length", String(page.length()));
  this->server->send(200, "text/html", page);
}

void Controller::saveSettingsAlert() {
  Serial.println("saveSettingsAlert()");

  if(!this->server->hasArg("ss") ||
     !this->server->hasArg("fs") ||
     !this->server->hasArg("fls") ||
     !this->server->hasArg("ic") ||
      this->server->arg("ss") == NULL ||
      this->server->arg("fs") == NULL ||
      this->server->arg("fls") == NULL ||
      this->server->arg("ic") == NULL) {
    this->server->send(400, "text/plain", "400: Invalid Request");
    return;
  }

  // Parse into temporary memory
  Config tmp;
  tmp.alert_sweep_speed = parseNumber(this->server->arg("ss"));
  tmp.alert_fill_speed = parseNumber(this->server->arg("fs"));
  tmp.alert_flash_speed = parseNumber(this->server->arg("fls"));
  tmp.alert_intro_color = parseColorCode(this->server->arg("ic"));
  tmp.alert_intro_glitchy = this->server->arg("intro_glitchy") == "on" ? true : false;

  Serial.println("tmp alert config: ");
  printConfig(tmp);

  // Copy over to cfg
  this->cfg.alert_sweep_speed = tmp.alert_sweep_speed;
  this->cfg.alert_fill_speed = tmp.alert_fill_speed;
  this->cfg.alert_flash_speed = tmp.alert_flash_speed;
  this->cfg.alert_intro_color = tmp.alert_intro_color;
  this->cfg.alert_intro_glitchy = tmp.alert_intro_glitchy;

  // Save to SPIFFS
  if(!this->saveConfig()) {
    this->server->send(500, "text/plain", "500: Failed to save config to SPIFFS");
    return;
  }

  this->server->sendHeader("Location", "/settings", true);
  this->server->send(303, "text/plain", "");
}

void Controller::handleSettingsBg() {
  Serial.println("handleSettingsBg()");
  
  // Headers
  String page = FPSTR(HTTP_AGS_HEAD);
  page.replace("{v}", "Backgrounds settings");
  page += FPSTR(HTTP_AGS_SCRIPT);
  page += FPSTR(HTTP_AGS_STYLE);
  page += FPSTR(HTTP_AGS_HEAD_END);

  // Content
  page += FPSTR(HTTP_AGS_TITLE);
  page.replace("{t}", AP_NAME);
  page += String(F("<h3>Backgrounds settings</h3>"));
  String item = FPSTR(HTTP_AGS_SETTINGS_FORM_START);
  item.replace("{a}", "/settings/bg"); // Submit action
  item.replace("{cs}", "\"cc\", \"bc\""); // Color fields to convert at submit
  page += item;
  page += FPSTR(HTTP_AGS_SETTINGS_BG_DEFAULT_START);
  for(uint8_t i = 0; i < Background::num; i++) {  // Add all backgrounds
    item = FPSTR(HTTP_AGS_SETTINGS_BG_DEFAULT_OPTION);
    item.replace("{v}", Background::name[i]);
    item.replace("{n}", Background::prettyName[i]);
    if(Background::name[i] == this->getBgDefault())
      item.replace("{s}", "selected");

    page += item;
  }
  page += FPSTR(HTTP_AGS_SETTINGS_BG_DEFAULT_END);
  item = FPSTR(HTTP_AGS_SETTINGS_BG_RAINBOW);
  item.replace("{s}", String(this->getBgRainbowSpeed()));
  page += item;
  item = FPSTR(HTTP_AGS_SETTINGS_BG_CYCLE);
  item.replace("{s}", String(this->getBgCycleSpeed()));
  page += item;
  page += FPSTR(HTTP_AGS_SETTINGS_BG_OFF);
  item = FPSTR(HTTP_AGS_SETTINGS_BG_COLOR);
  item.replace("{c}", colorCodeToHex(this->getBgColorColor()));
  page += item;
  item = FPSTR(HTTP_AGS_SETTINGS_BG_BEAT);
  item.replace("{c}", colorCodeToHex(this->getBgBeatColor()));
  item.replace("{s}", String(this->getBgBeatSpeed()));
  page += item;
  page += FPSTR(HTTP_AGS_SETTINGS_FORM_END);

  // Footer
  item = FPSTR(HTTP_AGS_FOOTER);
  item.replace("{v}", VERSION);
  page += item;
  page += FPSTR(HTTP_AGS_END);

  this->server->sendHeader("Content-Length", String(page.length()));
  this->server->send(200, "text/html", page);
}

void Controller::saveSettingsBg() {
  Serial.println("saveSettingsBg()");

  if(!this->server->hasArg("dbg") ||
     !this->server->hasArg("rs") ||
     !this->server->hasArg("cs") ||
     !this->server->hasArg("cc") ||
     !this->server->hasArg("bs") ||
     !this->server->hasArg("bc") ||
      this->server->arg("dbg") == NULL ||
      this->server->arg("rs") == NULL ||
      this->server->arg("cs") == NULL ||
      this->server->arg("cc") == NULL ||
      this->server->arg("bs") == NULL ||
      this->server->arg("bc") == NULL) {
    this->server->send(400, "text/plain", "400: Invalid Request");
    return;
  }

  bool validBgName = false;
  for(uint8_t i = 0; i < Background::num; i++) {  // Check background names
    if(Background::name[i] == this->server->arg("dbg")) {
      validBgName = true;
      break;
    }
  }

  if(!validBgName) {
    this->server->send(400, "text/plain", "400: Invalid Request");
    return;
  }

  // Parse into temporary memory
  Config tmp;
  strcpy(tmp.bg_default, this->server->arg("dbg").c_str());
  tmp.bg_rainbow_speed = parseNumber(this->server->arg("rs"));
  tmp.bg_cycle_speed = parseNumber(this->server->arg("cs"));
  tmp.bg_color_color = parseColorCode(this->server->arg("cc"));
  tmp.bg_beat_speed = parseNumber(this->server->arg("bs"));
  tmp.bg_beat_color = parseColorCode(this->server->arg("bc"));

  Serial.println("tmp bg config: ");
  printConfig(tmp);

  // Copy over to cfg
  strcpy(this->cfg.bg_default, tmp.bg_default);
  this->cfg.bg_rainbow_speed = tmp.bg_rainbow_speed;
  this->cfg.bg_cycle_speed = tmp.bg_cycle_speed;
  this->cfg.bg_color_color = tmp.bg_color_color;
  this->cfg.bg_beat_speed = tmp.bg_beat_speed;
  this->cfg.bg_beat_color = tmp.bg_beat_color;

  // Save to SPIFFS
  if(!this->saveConfig()) {
    this->server->send(500, "text/plain", "500: Failed to save config to SPIFFS");
    return;
  }

  this->initBg(); // Restart the default bg (in case the user tried another bg)

  this->server->sendHeader("Location", "/settings", true);
  this->server->send(303, "text/plain", "");
}

void Controller::handleOff() {
  Serial.println("handleOff()");
  
  this->setLedsOn(false);
  this->server->sendHeader("Location", "settings", true);
  this->server->send(303, "text/plain", "");
}

void Controller::handleOn() {
  Serial.println("handleOn()");
  
  this->setLedsOn(true);
  this->server->sendHeader("Location", "settings", true);
  this->server->send(303, "text/plain", "");
}

bool Controller::loadConfig() {
  File configFile = LittleFS.open("/config.json", "r");
  if (!configFile) {
    Serial.println("Failed to open config file");
    return false;
  }

  size_t size = configFile.size();
  if (size > 1024) {
    Serial.println("Config file size is too large");
    return false;
  }

  // Allocate a buffer to store contents of the file.
  std::unique_ptr<char[]> buf(new char[size]);

  // We don't use String here because ArduinoJson library requires the input
  // buffer to be mutable. If you don't use ArduinoJson, you may as well
  // use configFile.readString instead.
  configFile.readBytes(buf.get(), size);

  StaticJsonDocument<512> doc;
  auto error = deserializeJson(doc, buf.get());
  if (error) {
    Serial.println("Failed to parse config file");
    return false;
  }

  this->cfg.alert_sweep_speed = doc["alert_sweep_speed"];
  this->cfg.alert_fill_speed = doc["alert_fill_speed"];
  this->cfg.alert_flash_speed = doc["alert_flash_speed"];
  this->cfg.alert_intro_color = doc["alert_intro_color"];
  this->cfg.alert_intro_glitchy = doc["alert_intro_glitchy"];
  
  this->cfg.bg_rainbow_speed = doc["bg_rainbow_speed"];
  this->cfg.bg_cycle_speed = doc["bg_cycle_speed"];
  this->cfg.bg_color_color = doc["bg_color_color"];
  this->cfg.bg_beat_speed = doc["bg_beat_speed"];
  this->cfg.bg_beat_color = doc["bg_beat_color"];
  strcpy(this->cfg.bg_default, doc["bg_default"].as<const char*>());

  
  Serial.println("Loaded: ");
  printConfig(this->cfg);

  return true;
}

bool Controller::saveConfig() {
  StaticJsonDocument<512> doc;

 
  Serial.println("Saving: ");
  printConfig(this->cfg);

  doc["alert_sweep_speed"] = this->cfg.alert_sweep_speed;
  doc["alert_fill_speed"] = this->cfg.alert_fill_speed;
  doc["alert_flash_speed"] = this->cfg.alert_flash_speed;
  doc["alert_intro_color"] = this->cfg.alert_intro_color;
  doc["alert_intro_glitchy"] = this->cfg.alert_intro_glitchy;
  
  doc["bg_rainbow_speed"] = this->cfg.bg_rainbow_speed;
  doc["bg_cycle_speed"] = this->cfg.bg_cycle_speed;
  doc["bg_color_color"] = this->cfg.bg_color_color;
  doc["bg_beat_speed"] = this->cfg.bg_beat_speed;
  doc["bg_beat_color"] = this->cfg.bg_beat_color;
  doc["bg_default"] = this->cfg.bg_default;

  File configFile = LittleFS.open("/config.json", "w");
  if (!configFile) {
    Serial.println("Failed to open config file for writing");
    return false;
  }

  serializeJson(doc, configFile);
  return true;
}

uint16_t Controller::getAlertSweepSpeed() {
  return this->cfg.alert_sweep_speed;
}
uint16_t Controller::getAlertFillSpeed() {
  return this->cfg.alert_fill_speed;
}
uint16_t Controller::getAlertFlashSpeed() {
  return this->cfg.alert_flash_speed;
}
uint32_t Controller::getAlertIntroColor() {
  return this->cfg.alert_intro_color;
}
bool Controller::getAlertIntroGlitchy() {
  return this->cfg.alert_intro_glitchy;
}
uint16_t Controller::getBgRainbowSpeed() {
  return this->cfg.bg_rainbow_speed;
}
uint16_t Controller::getBgCycleSpeed() {
  return this->cfg.bg_cycle_speed;
}
uint32_t Controller::getBgColorColor() {
  return this->cfg.bg_color_color;
}
uint16_t Controller::getBgBeatSpeed() {
  return this->cfg.bg_beat_speed;
}
uint32_t Controller::getBgBeatColor() {
  return this->cfg.bg_beat_color;
}
char* Controller::getBgDefault() {
  return this->cfg.bg_default;
}

uint32_t parseColorCode(String c) {
  char pColor[] = "0xFF0000"; // C char array
  
  c.toCharArray(pColor, 9);
  
  if(pColor[0] == '0' && pColor[1] == 'x') {
    return strtol(pColor, NULL, 16);
  }
  else { 
    return (uint32_t)-1;
  }
}

uint32_t parseNumber(String c) {
  char pNumber[11] = ""; // C char array up to 32bit digit
  
  c.toCharArray(pNumber, 11);
  
  return strtol(pNumber, NULL, 10);
}

String colorCodeToHex(uint32_t c) {
  if(c > 0xFFFFFF) return String("Invalid color code: ") + String(c);
  
  char buf[10];
  
  sprintf(buf, "#%06X", c);
  return String(buf);
}

void printConfig(Config &cfg) {
  Serial.println("Config [");
  Serial.print("  alert_sweep_speed\t");    Serial.println(cfg.alert_sweep_speed);
  Serial.print("  alert_fill_speed\t");     Serial.println(cfg.alert_fill_speed);
  Serial.print("  alert_flash_speed\t");    Serial.println(cfg.alert_flash_speed);
  Serial.print("  alert_intro_color\t");    Serial.println(colorCodeToHex(cfg.alert_intro_color));
  Serial.print("  alert_intro_glitchy\t");  Serial.println(cfg.alert_intro_glitchy ? "true" : "false");
  Serial.print("  bg_rainbow_speed\t");     Serial.println(cfg.bg_rainbow_speed);
  Serial.print("  bg_cycle_speed\t");       Serial.println(cfg.bg_cycle_speed);
  Serial.print("  bg_color_color\t");       Serial.println(colorCodeToHex(cfg.bg_color_color));
  Serial.print("  bg_beat_speed\t");        Serial.println(cfg.bg_beat_speed);
  Serial.print("  bg_beat_color\t");        Serial.println(colorCodeToHex(cfg.bg_beat_color));
  Serial.print("  bg_default\t");           Serial.println(cfg.bg_default);
  Serial.println("]");
}
