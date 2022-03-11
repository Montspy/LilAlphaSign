
#ifndef STRIP_INFO_H
#define STRIP_INFO_H

#include <WiFi.h>                 //https://github.com/espressif/arduino-esp32 (in Boards manager, v1.0.6)
#include <DNSServer.h>            //https://github.com/espressif/arduino-esp32/tree/master/libraries/DNSServer
#include <WebServer.h>            //https://github.com/espressif/arduino-esp32/tree/master/libraries/WebServer
#include "WiFiManager.h"          //https://github.com/tzapu/WiFiManager (in Library manager, v2.0.3-alpha)

#include <ESPmDNS.h>              //https://github.com/espressif/arduino-esp32/tree/master/libraries/ESPmDNS


#include <FastLED.h>              //https://github.com/FastLED/FastLED/ (in Library manager, v3.4.0)

#include <ArduinoJson.h>          // in Library manager, v6.18.3
#include <FS.h>
#include <LITTLEFS.h>             //https://github.com/lorol/LITTLEFS

#define AP_NAME "Lil SamWSign"
#define HOSTNAME "lil-samw-sign"
#define VERSION "0.2.0"

#define DATA_PIN    0
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define NUM_LEDS    37

#define FRAMES_PER_SECOND  60

#define STRIP_BRIGHTNESS   255
#define BG_BRIGHTNESS      192
#define ANIM_BRIGHTNESS    255

#endif
