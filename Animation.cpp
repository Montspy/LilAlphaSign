
#include "Animation.h"

bool Animation::execute(CRGB* animLeds) {
  // Execute state
  switch(this->state) {
    case State::INIT:
      this->stateTimer = new CEveryNMillis(this->easein_ms);
      this->animTimer = new CEveryNMillis(this->total_ms);
      this->alpha = 0;
      break;

    case State::EASEIN:
      this->easeInAlpha();
      this->animate(animLeds);
      break;

    case State::SUSTAIN:
      this->alpha = 255;
      this->animate(animLeds);
      break;

    case State::EASEOUT:
      this->easeOutAlpha();
      this->animate(animLeds);
      break;

    case State::STOP:
      this->alpha = 0;
      break;
  }
  
  // Update state
  switch(this->state) {
    case State::INIT:
      this->state = State::EASEIN;
      break;

    case State::EASEIN:
      if(this->stateTimer->ready()) {
        this->stateTimer->setPeriod(this->total_ms - this->easein_ms - this->easeout_ms);
        this->stateTimer->reset();

        this->state = State::SUSTAIN;
      }
      break;

    case State::SUSTAIN:
      if(this->stateTimer->ready()) {
        this->stateTimer->setPeriod(this->easeout_ms);
        this->stateTimer->reset();

        this->state = State::EASEOUT;
      }
      break;

    case State::EASEOUT:
      if(this->stateTimer->ready()) {
        this->state = State::STOP;
      }
      break;

    case State::STOP:
      return false;
      break;
  }
  
  return true;
}

fract8 Animation::getAlpha() {
  return this->alpha;
}

void Animation::easeInAlpha() {
  this->alpha = ease8InOutApprox(min((uint64_t)255, (uint64_t)255 * this->stateTimer->getElapsed() / this->stateTimer->getPeriod()));
}

void Animation::easeOutAlpha() {
  this->alpha = ease8InOutApprox(max((uint64_t)0, (uint64_t)255 * this->stateTimer->getRemaining() / this->stateTimer->getPeriod()));
}

void Animation::animate(CRGB* leds) {
  Serial.println("Default Anim animate!!!");
  return;
}

void RainbowAnimation::animate(CRGB* leds) {
  fract8 progress = min((uint64_t)255, (uint64_t)255 * this->animTimer->getElapsed() / this->animTimer->getPeriod());
  this->fillRainbow(leds, NUM_LEDS, progress, 20, dim8_raw(ANIM_BRIGHTNESS));
}

void RainbowAnimation::fillRainbow(CRGB * pFirstLED, int numToFill, uint8_t initialhue, uint8_t deltahue, uint8_t value) {
  CHSV hsv;
  hsv.hue = initialhue;
  hsv.val = value;
  hsv.sat = 240;
  
  for(int i = 0; i < numToFill; i++) {
    pFirstLED[i] = hsv;
    hsv.hue += deltahue;
  }
}

void Sine1onAnimation::animate(CRGB* leds) {
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy(leds, NUM_LEDS, 20);
  int pos = beatsin16(13, 0, NUM_LEDS-1);
  CRGB dimmedColor = this->color;
  dimmedColor.nscale8(dim8_raw(ANIM_BRIGHTNESS));
  leds[pos] += dimmedColor;
}

void SweepAnimation::animate(CRGB* leds) {
  // 2 cycles per animation (theta goes from 0 to 2*2pi = 0 to 65535  twice for sin16/cos16)
  uint16_t theta = min((uint64_t)131071, (uint64_t)131071 * this->animTimer->getElapsed() / this->animTimer->getPeriod());
  uint16_t beatcos = cos16(theta + 32768) + 32768;  // Phase offset to start cos(theta + offset) == 0
  uint16_t scaledbeat = scale16(beatcos, NUM_LEDS-1);
  
  fadeToBlackBy(leds, NUM_LEDS, 20);
  CRGB dimmedColor = this->color;
  dimmedColor.nscale8(dim8_raw(ANIM_BRIGHTNESS));
  leds[scaledbeat] += dimmedColor;
}

void FillAnimation::animate(CRGB* leds) {
  // 1 fill per animation (progress goes from 0 to NUM_LEDS*256*2 = 28672)
  // ease in-out fill for 50% of the time (0 to 14335)
  // then, brightness ramp for 25% of the time (14336 to 21504)

  uint16_t progress = min((uint64_t)28672, (uint64_t)28672 * this->animTimer->getElapsed() / this->animTimer->getPeriod());
  
  uint8_t numFilled = NUM_LEDS;
  uint8_t lastFill = 255;
  if(progress < 14336) {
    numFilled = progress >> 8;
    lastFill = progress - (numFilled << 8);
  }

  uint8_t brightness = 192;
  if((progress >= 14336) && (progress < 21504))
    brightness = map(progress, 14336, 21504, 192, ANIM_BRIGHTNESS);
  else if(progress >= 21504)
    brightness = ANIM_BRIGHTNESS;

  CRGB dimmedColor = this->color;
  dimmedColor.nscale8(dim8_raw(brightness));

  fill_solid(leds, NUM_LEDS, 0x000000);
  fill_solid(leds, numFilled, dimmedColor);
  dimmedColor.nscale8(lastFill);
  leds[numFilled] = dimmedColor;
}

void FlashAnimation::animate(CRGB* leds) {
  uint16_t progress = min((uint64_t)128, (uint64_t)128 * this->animTimer->getElapsed() / this->animTimer->getPeriod());
  
  uint8_t brightness = ease8InOutCubic(128 - progress);
  brightness = map(brightness, 0, 128, 192, ANIM_BRIGHTNESS);

  CRGB dimmedColor = this->color;
  dimmedColor.nscale8(dim8_raw(brightness));

  fill_solid(leds, NUM_LEDS, dimmedColor);
}

void IntroAnimation::animate(CRGB* leds) {
  // 60fps frame counter, 4.25s (255 frames)
  uint8_t frame = min((uint64_t)255, (uint64_t)255 * this->animTimer->getElapsed() / this->animTimer->getPeriod());
  
  fill_solid(leds, NUM_LEDS, 0x000000);

  // 1.5s brightness ramps
  uint8_t brightness = 192;
  if((frame > this->frameOffsetInner[2] + 34) && (frame <= this->frameOffsetInner[2] + 34 + 90))
    brightness = map(frame - (this->frameOffsetInner[2] + 34), 0, 90, 192, ANIM_BRIGHTNESS);
  if(frame > this->frameOffsetInner[2] + 34 + 90)
    brightness = ANIM_BRIGHTNESS;
  
  // Add sweeps
  CRGB dimmedColor = this->color;
  dimmedColor.nscale8(dim8_raw(scale8(brightness, 160)));
  this->addSweep(leds, max((int)0, (int)frame), dimmedColor);
  dimmedColor = this->color;
  dimmedColor.nscale8(dim8_raw(scale8(brightness, 255)));
  this->addSweep(leds, max((int)0, (int)frame - 20), dimmedColor);

  if(this->glitchy) {
    // Add prism blur
    this->addPrismBlur(leds, frame - 20, 160);
  
    // Add glitches when sweeping in and sweeping out
    this->addGlitch(leds, frame - 20, 80);
  }
}

void IntroAnimation::addSweep(CRGB* leds, uint8_t frame, CRGB dimmedColor) {
  uint8_t sweepEnd, sweepBegin;
  
  // Inner sweep #1
  sweepEnd   = 28 + this->sweepLut_1[ constrain(frame - this->frameOffsetInner[0], 0, 33) ];
  sweepBegin = 28 + this->sweepLut_1[ constrain(frame - this->frameOffsetInner[1], 0, 33) ];
  fill_solid(&leds[sweepBegin], sweepEnd - sweepBegin, dimmedColor);

  // Inner sweep #2
  sweepEnd   = 28 + this->sweepLut_1[ constrain(frame - this->frameOffsetInner[2], 0, 33) ];
  sweepBegin = 28 + this->sweepLut_1[ constrain(frame - this->frameOffsetInner[3], 0, 33) ];
  fill_solid(&leds[sweepBegin], sweepEnd - sweepBegin, dimmedColor);
  
  
  // Outer sweep #1
  sweepEnd   = 28 - this->sweepLut_1[ constrain(frame - this->frameOffsetOuter[0], 0, 33) ];
  sweepBegin = 28 - this->sweepLut_1[ constrain(frame - this->frameOffsetOuter[1], 0, 33) ];
  fill_solid(&leds[sweepEnd], sweepBegin - sweepEnd, dimmedColor);
  
  // Outer sweep #2
  sweepEnd   = 28 - this->sweepLut_1[ constrain(frame - this->frameOffsetOuter[2], 0, 33) ];
  sweepBegin = 28 - this->sweepLut_1[ constrain(frame - this->frameOffsetOuter[3], 0, 33) ];
  fill_solid(&leds[sweepEnd], sweepBegin - sweepEnd, dimmedColor);
}

void IntroAnimation::addPrismBlur(CRGB* leds, uint8_t frame, uint8_t glitchChance) {
  // The red channel will be shifted by a random amount of pixels to split white into Red/White/Cyan glitches
  // Simplex noise based on frame number, scaled to [0; 3]
  int8_t shift = inoise8(0x8000 + ((uint16_t)frame) << 3) >> 7;
  
  bool timeForEffect = false;
  if((frame > this->frameOffsetInner[0]) && (frame <= this->frameOffsetInner[2] + 34))
    timeForEffect = inoise8(0x4000 + ((uint16_t)frame) << 5) < glitchChance;
//  else if((frame > this->frameOffsetInner[3] - 34))
//    timeForEffect = inoise8(0x4000 + ((uint16_t)frame) << 5) < glitchChance;
  
  if(timeForEffect) {
    CRGB shiftedLeds[NUM_LEDS];
    fill_solid(shiftedLeds, NUM_LEDS, 0x000000);
    
    for(int i = 0; i < NUM_LEDS; i++) {
      shiftedLeds[i] = leds[i];
    }

    for(int i = 0; i < NUM_LEDS/2 - shift; i++) {
      // Outer segment
      shiftedLeds[i + shift].red = leds[i].red;

      // Inner segment
      shiftedLeds[NUM_LEDS/2 + i + shift].red = leds[NUM_LEDS/2 + i].red;
    }

    // Fill in ends based on first and last pixel
    for(int i = 0; i < shift; i++) {
      // Outer segment
      shiftedLeds[i].red = 0;
      shiftedLeds[NUM_LEDS/2 - 1 - i].green = 0;
      shiftedLeds[NUM_LEDS/2 - 1 - i].blue = 0;
      
      // Inner segment
      shiftedLeds[NUM_LEDS/2 + i].red = 0;
      shiftedLeds[NUM_LEDS - 1 - i].green = 0;
      shiftedLeds[NUM_LEDS - 1 - i].blue = 0;
    }

    for(int i = 0; i < NUM_LEDS; i++) {
      leds[i] = shiftedLeds[i];
    }
  }
}

void IntroAnimation::addGlitch(CRGB* leds, uint8_t frame, uint8_t glitchChance) {
  static uint16_t idx1 = 0, idx2 = 0;
  static uint8_t cnt = 0;

  bool timeForEffect = 0;
//  if((frame > this->frameOffsetInner[0]) && (frame <= this->frameOffsetInner[2] + 34))
//    timeForEffect = true;
  if((frame > this->frameOffsetInner[3]) && (frame <= this->frameOffsetInner[3] + 34))
    timeForEffect = true;
  
  if(timeForEffect && ((cnt < frame) || (cnt > frame + 10))) { // Previous glitch expired
    if(random8() < glitchChance) {
      cnt = frame + random8(2, 5);
      idx1 = random8(NUM_LEDS);
      idx2 = random8(NUM_LEDS);
    }
  }

  if(timeForEffect && (cnt >= frame)) {  // Ongoing glitch
    leds[idx1] = this->color - CRGB(CRGB::Red);
    leds[idx2] = this->color - CRGB(CRGB::Cyan);
  }
}
