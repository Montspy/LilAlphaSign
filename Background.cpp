
#include "Background.h"


const String Background::name[] = {"rainbow", "cycle", "off", "color", "beat"};
const String Background::prettyName[] = {"Rainbow", "Cycles hues", "Off", "Solid color", "Heartbeat"};

void Background::execute(CRGB* bgLeds) {
  if(this->bgTimer == NULL)
    this->bgTimer = new CEveryNMillis(1000/FRAMES_PER_SECOND);

  if(this->bgTimer)
    this->frame++;
  
  if(this->millisStart == -1) {
    this->millisStart = millis();
  }
  
  this->animate(bgLeds);
}

void Background::animate(CRGB* leds) {
  Serial.println("Default BG animate!!!");
  return;
}

void Background::fillRainbow(CRGB * pFirstLED, int numToFill, uint8_t initialhue, uint8_t deltahue, uint8_t value) {
  CHSV hsv;
  hsv.hue = initialhue;
  hsv.val = value;
  hsv.sat = 240;
  
  for(int i = 0; i < numToFill; i++) {
    pFirstLED[i] = hsv;
    hsv.hue += deltahue;
  }
}

void OffBackground::animate(CRGB* leds) {
  fill_solid(leds, NUM_LEDS, 0x000000);
}

void ColorBackground::animate(CRGB* leds) {
  fill_solid(leds, NUM_LEDS, this->color);
}

void BeatBackground::animate(CRGB* leds) {
  uint8_t beat = beatsin8(this->bpm, 96, 255, this->millisStart, 64);
  
  CRGB c(this->color);
  c.nscale8(beat); // Sinusoid beat, starting at highest brightness
  fill_solid(leds, NUM_LEDS, c);
}

void RainbowBackground::animate(CRGB* leds) {
//  fract8 progress = this->frame % 256;
  fract8 progress = beat8(this->bpm, this->millisStart);
  
  if(reversed) {
    this->fillRainbow(leds, NUM_LEDS/2, progress, 10, dim8_raw(BG_BRIGHTNESS)); // Forward
    this->fillRainbow(&leds[NUM_LEDS/2], NUM_LEDS/2, progress, -10, dim8_raw(BG_BRIGHTNESS)); // Aaand back
  }
  else {
    this->fillRainbow(leds, NUM_LEDS, progress, 10, dim8_raw(BG_BRIGHTNESS));
  }
}

void CycleBackground::animate(CRGB* leds) {
//  fract8 progress = this->frame % 256;
  fract8 progress = beat8(this->bpm, this->millisStart);
  
  this->fillRainbow(leds, NUM_LEDS, progress, 0, dim8_raw(BG_BRIGHTNESS));
}
