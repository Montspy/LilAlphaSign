
#ifndef ANIMATION_H
#define ANIMATION_H

#include "Application.h"

class Animation {
  public:
    Animation(uint32_t total_ms, uint32_t easein_ms = 0, uint32_t easeout_ms = 0)
      : total_ms(total_ms), easein_ms(easein_ms), easeout_ms(easeout_ms) {}
    
    bool execute(CRGB* animLeds);
    fract8 getAlpha();
    
  private:
    CEveryNMillis* stateTimer = NULL;
    uint32_t total_ms, easein_ms, easeout_ms;
    enum struct State {
      INIT,
      EASEIN,
      SUSTAIN,
      EASEOUT,
      STOP
    };
    State state = State::INIT;
    
    fract8 alpha = 0;
    void easeInAlpha();
    void easeOutAlpha();
    
  protected:
    virtual void animate(CRGB* leds);
    CEveryNMillis* animTimer = NULL;
};

class RainbowAnimation: public Animation {
  public:
    RainbowAnimation(uint32_t total_ms, uint32_t easein_ms = 1, uint32_t easeout_ms = 1)
      : Animation(total_ms, easein_ms, easeout_ms) {}
      
  protected:
    void animate(CRGB* leds);
    void fillRainbow(CRGB * pFirstLED, int numToFill, uint8_t initialhue, uint8_t deltahue, uint8_t value = 255);
};

class Sine1onAnimation: public Animation {
  public:
    Sine1onAnimation(uint32_t total_ms, CRGB color = CRGB::White, uint32_t easein_ms = 1, uint32_t easeout_ms = 1)
      : Animation(total_ms, easein_ms, easeout_ms)
      , color(color) {}
    
  protected:
    CRGB color;
    void animate(CRGB* leds);
};

class SweepAnimation: public Animation {
  public:
    SweepAnimation(uint32_t total_ms = 6500, CRGB color = CRGB::White, uint32_t easein_ms = 200, uint32_t easeout_ms = 500)
      : Animation(total_ms, easein_ms, easeout_ms)
      , color(color) {}
    
  protected:
    CRGB color;
    void animate(CRGB* leds);
};

class FillAnimation: public Animation {
  const uint64_t maxProgress = NUM_LEDS * 256 * 2;
  const uint64_t halfProgress = (NUM_LEDS * 256 * 2) >> 1;
  const uint64_t threeQuarterProgress = 3 * ((NUM_LEDS * 256 * 2) >> 2);
  public:
    FillAnimation(uint32_t total_ms = 3250, CRGB color = CRGB::White, uint32_t easein_ms = 200, uint32_t easeout_ms = 500)
      : Animation(total_ms, easein_ms, easeout_ms)
      , color(color) {}
    
  protected:
    CRGB color;
    void animate(CRGB* leds);
};

class FlashAnimation: public Animation {
  public:
    FlashAnimation(uint32_t total_ms = 3250, CRGB color = CRGB::White, uint32_t easein_ms = 1, uint32_t easeout_ms = 500)
      : Animation(total_ms, easein_ms, easeout_ms)
      , color(color) {}
    
  protected:
    CRGB color;
    void animate(CRGB* leds);
};

class IntroAnimation: public Animation {
  public:
    IntroAnimation(bool glitchy = true, CRGB color = CRGB::White, uint32_t total_ms = 4250, uint32_t easein_ms = 200, uint32_t easeout_ms = 500)
      : Animation(total_ms, easein_ms, easeout_ms)
      , glitchy(glitchy), color(color){}
    
  protected:
    bool glitchy;
    void animate(CRGB* leds);
    void addSweep(CRGB* leds, uint8_t frame, CRGB color = CHSV(0, 0, ANIM_BRIGHTNESS));
    void addGlitch(CRGB* leds, uint8_t frame, uint8_t glitchChance = 80);
    void addPrismBlur(CRGB* leds, uint8_t frame, uint8_t glitchChance = 80);

    // Frame to pixel index LUT: At frame i, pixel index sweepLut_1[i] is at the edge of the transition
    uint8_t sweepLut_1[34] = {0, 0, 0, 1, 1, 2, 2, 3, 4, 5, 6, 8, 9, 11, 12, 14, 16, 18, 20, 22, 23, 25, 26, 28, 29, 30, 31, 32, 32, 33, 33, 34, 34, 34}; // Quad

    // Frame offset for sweep start (4 transitions: in1, out1, in2, out2)
    fract8 frameOffset[4] = {12, 28, 32, 171};

    CRGB color;
};

#endif
