
#ifndef BACKGROUND_H
#define BACKGROUND_H

#include "Application.h"

class Background {
  public:
    Background() {}
    
    void execute(CRGB* bgLeds);
    
    static const uint8_t num = 5;
    static const String name[];
    static const String prettyName[];
    
  protected:
    virtual void animate(CRGB* leds);
    CEveryNMillis* bgTimer = NULL;
    uint64_t frame = 0;
    uint64_t millisStart = -1;

    void fillRainbow(CRGB * pFirstLED, int numToFill, uint8_t initialhue, uint8_t deltahue, uint8_t value = 255);
};

class OffBackground: public Background {
  public:
    OffBackground()
      : Background() {}
      
  protected:
    void animate(CRGB* leds);
};

class ColorBackground: public Background {
  public:
    ColorBackground(CRGB color)
      : Background() {
      uint16_t v = max(color.r, max(color.g, color.b)); // Obtain V from HSV
      
      if(v > dim8_raw(BG_BRIGHTNESS)) { // Scale down to V = dim8_raw(BG_BRIGHTNESS)
        fract8 s = (uint16_t)dim8_raw(BG_BRIGHTNESS) * 255 / v;
        color.nscale8(s);
      }
      
      this->color = color;
    }
      
  protected:
    void animate(CRGB* leds);
    CRGB color;
};

class BeatBackground: public ColorBackground {
  public:
    BeatBackground(CRGB color, uint8_t bpm = 15)
      : ColorBackground(color), bpm(bpm) {}

    void execute(CRGB* bgLeds);
      
  protected:
    void animate(CRGB* leds);
    uint8_t bpm;
};

class RainbowBackground: public Background {
  public:
    RainbowBackground(uint8_t bpm = 15, bool reversed = true)
      : Background(), bpm(bpm), reversed(reversed) {}
      
  protected:
    void animate(CRGB* leds);
    uint8_t bpm;
    bool reversed;
};

class CycleBackground: public Background {
  public:
    CycleBackground(uint8_t bpm = 15)
      : Background(), bpm(bpm) {}
      
  protected:
    void animate(CRGB* leds);
    uint8_t bpm;
};

#endif
