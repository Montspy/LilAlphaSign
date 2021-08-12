/***
 * Board: NodeMCU 1.0 (ESP-12E)
 * Speed: 160MHz
 * Libraries: see Application.h file
 * Additional: 
 *     + Upload files config.json to LittleFS after full flash erase (https://arduino-esp8266.readthedocs.io/en/3.0.2/filesystem.html#uploading-files-to-file-system)
 */

#include "Application.h"
#include "Animation.h"
#include "Background.h"
#include "Controller.h"

void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  //if you used auto generated SSID, print it
  Serial.println(myWiFiManager->getConfigPortalSSID());
}

ESP8266WebServer server;

FASTLED_USING_NAMESPACE
CRGB aLeds[NUM_LEDS];
CRGB bLeds[NUM_LEDS];
CRGB outLeds[NUM_LEDS];

Animation* currAnim = NULL;
Background* currBg = NULL;
bool isOn = true;

uint8_t gHue = 0;

Controller ctrl(&server, &currAnim, &currBg);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  if (!LittleFS.begin()) {
    Serial.println("Failed to mount file system");
    return;
  }
  ctrl.loadConfig();
  
  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
  //reset settings - for testing
  //wifiManager.resetSettings();

  //set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
  wifiManager.setAPCallback(configModeCallback);

  //fetches ssid and pass and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  if(!wifiManager.autoConnect(AP_NAME)) {
    Serial.println("failed to connect and hit timeout");
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(1000);
  } 

  //if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");
  
  Serial.print("Local IP: ");
  Serial.println(WiFi.localIP());
  if(MDNS.begin("esp8266", WiFi.localIP()))
    Serial.println("MDNS Responder Started!");

  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(outLeds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  //FastLED.addLeds<LED_TYPE,DATA_PIN,CLK_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  // set master brightness control
  FastLED.setBrightness(STRIP_BRIGHTNESS);
  
  server.begin();

  fill_solid(aLeds, NUM_LEDS, 0x000000);
  fill_solid(bLeds, NUM_LEDS, 0x000000);
  fill_solid(outLeds, NUM_LEDS, 0x000000);

//  currBg = new OffBackground();
//  currBg = new ColorBackground(CRGB::White);
//  currBg = new BeatBackground(CRGB::Purple);
//  currBg = new RainbowBackground(false);
//  currBg = new CycleBackground();
  ctrl.initBg();
}

void loop() {
  handleWifiTasks();

  // send the 'leds' array out to the actual LED strip
  EVERY_N_MILLISECONDS(1000/FRAMES_PER_SECOND) {
    if(!ctrl.getLedsOn()) {
      fill_solid(outLeds, NUM_LEDS, 0x000000);
    }
    else {
      // Background!
      if(currBg != NULL) { 
        currBg->execute(bLeds);
      }
      
      // Animate alerts away!
      fract8 animOpacity = 0;
      if(currAnim != NULL) {
        if(currAnim->execute(aLeds)) {
          animOpacity = currAnim->getAlpha();
        }
        else {  // Delete animation once it's completed
          delete currAnim;
          currAnim = NULL;
        }
      }

      // Merge bg and anim
      blend(bLeds, aLeds, outLeds, NUM_LEDS, animOpacity);
    }
    
    FastLED.show();
  }
}

void handleWifiTasks() {
  if(ensureWifi()) {
    server.handleClient();
  }
  else {
  }
}

bool ensureWifi() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Disconnected :(");
    WiFiManager wifiManager;
    wifiManager.setConnectTimeout(180);
    wifiManager.setTimeout(180);
    
    wifiManager.autoConnect(AP_NAME);
  }
  
  return (WiFi.status() == WL_CONNECTED);
}
