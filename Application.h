
#ifndef STRIP_INFO_H
#define STRIP_INFO_H

#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include "WiFiManager.h"          //https://github.com/tzapu/WiFiManager
#include <ESP8266mDNS.h>
#define FASTLED_INTERRUPT_RETRY_COUNT 3
#define FASTLED_ESP8266_NODEMCU_PIN_ORDER
#include <FastLED.h>

#include <ArduinoJson.h>
#include "FS.h"
#include <LittleFS.h>

#define AP_NAME "Lil AlphaSign"
#define VERSION "0.1.0"

#define DATA_PIN    7
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define NUM_LEDS    56

#define FRAMES_PER_SECOND  60

#define STRIP_BRIGHTNESS   255
#define BG_BRIGHTNESS      176
#define ANIM_BRIGHTNESS    255

#endif
