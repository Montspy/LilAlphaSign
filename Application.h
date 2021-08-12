
#ifndef STRIP_INFO_H
#define STRIP_INFO_H

#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino (in Boards manager, v3.0.2)
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include "WiFiManager.h"          //https://github.com/tzapu/WiFiManager (in Library manager, v2.0.3-alpha)
#include <ESP8266mDNS.h>
#define FASTLED_INTERRUPT_RETRY_COUNT 3
#define FASTLED_ESP8266_NODEMCU_PIN_ORDER
#include <FastLED.h>

#include <ArduinoJson.h>          // in Library manager, v6.18.3
#include "FS.h"
#include <LittleFS.h>

#define AP_NAME "Lil SamWSign"
#define VERSION "0.1.0"

#define DATA_PIN    7
#define LED_TYPE    SK6812
#define COLOR_ORDER GRB
#define NUM_LEDS    37

#define FRAMES_PER_SECOND  60

#define STRIP_BRIGHTNESS   255
#define BG_BRIGHTNESS      176
#define ANIM_BRIGHTNESS    255

#endif
