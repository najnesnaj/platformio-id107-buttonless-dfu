/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 by Daniel Eichhorn
 * Copyright (c) 2016 by Fabrice Weinberg
 * Copyright (c) 2017 by Mark Cooke
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#ifndef OLEDDISPLAYUI_h
#define OLEDDISPLAYUI_h

#include <Arduino.h>
#include "OLEDDisplay.h"

enum AnimationDirection {
  SLIDE_UP,
  SLIDE_DOWN,
  SLIDE_LEFT,
  SLIDE_RIGHT
};

enum IndicatorPosition {
  TOP,
  RIGHT,
  BOTTOM,
  LEFT
};

enum IndicatorDirection {
  LEFT_RIGHT,
  RIGHT_LEFT
};

enum FrameState {
  IN_TRANSITION,
  FIXED
};


const char ANIMATION_activeSymbol[] PROGMEM = {
  0x00, 0x18, 0x3c, 0x7e, 0x7e, 0x3c, 0x18, 0x00
};

const char ANIMATION_inactiveSymbol[] PROGMEM = {
  0x00, 0x0, 0x0, 0x18, 0x18, 0x0, 0x0, 0x00
};


// Structure of the UiState
struct OLEDDisplayUiState {
  uint64_t     lastUpdate                = 0;
  uint16_t      ticksSinceLastStateSwitch = 0;

  FrameState    frameState                = FIXED;
  uint8_t       currentFrame              = 0;

  bool          isIndicatorDrawen         = true;

  // Normal = 1, Inverse = -1;
  int8_t        frameTransitionDirection  = 1;

  bool          manuelControll            = false;

  // Custom data that can be used by the user
  void*         userData                  = NULL;
};

struct LoadingStage {
  const char* process;
  void (*callback)();
};

typedef void (*FrameCallback)(OLEDDisplay *display,  OLEDDisplayUiState* state, int16_t x, int16_t y);
typedef void (*OverlayCallback)(OLEDDisplay *display,  OLEDDisplayUiState* state);
typedef void (*LoadingDrawFunction)(OLEDDisplay *display, LoadingStage* stage, uint8_t progress);

class OLEDDisplayUi {
  private:
    OLEDDisplay             *display;

    // Symbols for the Indicator
    IndicatorPosition   indicatorPosition         = BOTTOM;
    IndicatorDirection  indicatorDirection        = LEFT_RIGHT;

    const char*         activeSymbol              = ANIMATION_activeSymbol;
    const char*         inactiveSymbol            = ANIMATION_inactiveSymbol;

    bool                shouldDrawIndicators      = true;

    // Values for the Frames
    AnimationDirection  frameAnimationDirection   = SLIDE_RIGHT;

    int8_t              lastTransitionDirection   = 1;

    uint16_t            ticksPerFrame             = 151; // ~ 5000ms at 30 FPS
    uint16_t            ticksPerTransition        = 15;  // ~  500ms at 30 FPS

    bool                autoTransition            = true;

    FrameCallback*      frameFunctions;
    uint8_t             frameCount                = 0;

    // Internally used to transition to a specific frame
    int8_t              nextFrameNumber           = -1;

    // Values for Overlays
    OverlayCallback*    overlayFunctions;
    uint8_t             overlayCount              = 0;

    // Will the Indicator be drawen
    // 3 Not drawn in both frames
    // 2 Drawn this frame but not next
    // 1 Not drown this frame but next
    // 0 Not known yet
    uint8_t                indicatorDrawState        = 1;

    // Loading screen
    LoadingDrawFunction loadingDrawFunction       = [](OLEDDisplay *display, LoadingStage* stage, uint8_t progress) {
      display->setTextAlignment(TEXT_ALIGN_CENTER);
      display->setFont(ArialMT_Plain_10);
      display->drawString(64, 18, stage->process);
      display->drawProgressBar(4, 32, 120, 8, progress);
    };

    // UI State
    OLEDDisplayUiState      state;

    // Bookeeping for update
    uint8_t             updateInterval            = 33;

    uint8_t             getNextFrameNumber();
    void                drawIndicator();
    void                drawFrame();
    void                drawOverlays();
    void                tick();
    void                resetState();

  public:

    OLEDDisplayUi(OLEDDisplay *display);

    /**
     * Initialise the display
     */
    void init();

    /**
     * Configure the internal used target FPS
     */
    void setTargetFPS(uint8_t fps);

    // Automatic Controll
    /**
     * Enable automatic transition to next frame after the some time can be configured with `setTimePerFrame` and `setTimePerTransition`.
     */
    void enableAutoTransition();

    /**
     * Disable automatic transition to next frame.
     */
    void disableAutoTransition();

    /**
     * Set the direction if the automatic transitioning
     */
    void setAutoTransitionForwards();
    void setAutoTransitionBackwards();

    /**
     *  Set the approx. time a frame is displayed
     */
    void setTimePerFrame(uint16_t time);

    /**
     * Set the approx. time a transition will take
     */
    void setTimePerTransition(uint16_t time);

    // Customize indicator position and style

    /**
     * Draw the indicator.
     * This is the defaut state for all frames if
     * the indicator was hidden on the previous frame
     * it will be slided in.
     */
    void enableIndicator();

    /**
     * Don't draw the indicator.
     * This will slide out the indicator
     * when transitioning to the next frame.
     */
    void disableIndicator();

    /**
     * Enable drawing of indicators
     */
    void enableAllIndicators();

    /**
     * Disable draw of indicators.
     */
    void disableAllIndicators();

    /**
     * Set the position of the indicator bar.
     */
    void setIndicatorPosition(IndicatorPosition pos);

    /**
     * Set the direction of the indicator bar. Defining the order of frames ASCENDING / DESCENDING
     */
    void setIndicatorDirection(IndicatorDirection dir);

    /**
     * Set the symbol to indicate an active frame in the indicator bar.
     */
    void setActiveSymbol(const char* symbol);

    /**
     * Set the symbol to indicate an inactive frame in the indicator bar.
     */
    void setInactiveSymbol(const char* symbol);


    // Frame settings

    /**
     * Configure what animation is used to transition from one frame to another
     */
    void setFrameAnimation(AnimationDirection dir);

    /**
     * Add frame drawing functions
     */
    void setFrames(FrameCallback* frameFunctions, uint8_t frameCount);

    // Overlay

    /**
     * Add overlays drawing functions that are draw independent of the Frames
     */
    void setOverlays(OverlayCallback* overlayFunctions, uint8_t overlayCount);


    // Loading animation
    /**
     * Set the function that will draw each step
     * in the loading animation
     */
    void setLoadingDrawFunction(LoadingDrawFunction loadingFunction);


    /**
     * Run the loading process
     */
    void runLoadingProcess(LoadingStage* stages, uint8_t stagesCount);


    // Manual Control
    void nextFrame();
    void previousFrame();

    /**
     * Switch without transition to frame `frame`.
     */
    void switchToFrame(uint8_t frame);

    /**
     * Transition to frame `frame`, when the `frame` number is bigger than the current
     * frame the forward animation will be used, otherwise the backwards animation is used.
     */
    void transitionToFrame(uint8_t frame);

    // State Info
    OLEDDisplayUiState* getUiState();

    int8_t update();
};

OLEDDisplayUi::OLEDDisplayUi(OLEDDisplay *display) {
  this->display = display;
}

void OLEDDisplayUi::init() {
  this->display->init();
}

void OLEDDisplayUi::setTargetFPS(uint8_t fps){
  float oldInterval = this->updateInterval;
  this->updateInterval = ((float) 1.0 / (float) fps) * 1000;

  // Calculate new ticksPerFrame
  float changeRatio = oldInterval / (float) this->updateInterval;
  this->ticksPerFrame *= changeRatio;
  this->ticksPerTransition *= changeRatio;
}

// -/------ Automatic controll ------\-

void OLEDDisplayUi::enableAutoTransition(){
  this->autoTransition = true;
}
void OLEDDisplayUi::disableAutoTransition(){
  this->autoTransition = false;
}
void OLEDDisplayUi::setAutoTransitionForwards(){
  this->state.frameTransitionDirection = 1;
  this->lastTransitionDirection = 1;
}
void OLEDDisplayUi::setAutoTransitionBackwards(){
  this->state.frameTransitionDirection = -1;
  this->lastTransitionDirection = -1;
}
void OLEDDisplayUi::setTimePerFrame(uint16_t time){
  this->ticksPerFrame = (int) ( (float) time / (float) updateInterval);
}
void OLEDDisplayUi::setTimePerTransition(uint16_t time){
  this->ticksPerTransition = (int) ( (float) time / (float) updateInterval);
}

// -/------ Customize indicator position and style -------\-
void OLEDDisplayUi::enableIndicator(){
  this->state.isIndicatorDrawen = true;
}

void OLEDDisplayUi::disableIndicator(){
  this->state.isIndicatorDrawen = false;
}

void OLEDDisplayUi::enableAllIndicators(){
  this->shouldDrawIndicators = true;
}

void OLEDDisplayUi::disableAllIndicators(){
  this->shouldDrawIndicators = false;
}

void OLEDDisplayUi::setIndicatorPosition(IndicatorPosition pos) {
  this->indicatorPosition = pos;
}
void OLEDDisplayUi::setIndicatorDirection(IndicatorDirection dir) {
  this->indicatorDirection = dir;
}
void OLEDDisplayUi::setActiveSymbol(const char* symbol) {
  this->activeSymbol = symbol;
}
void OLEDDisplayUi::setInactiveSymbol(const char* symbol) {
  this->inactiveSymbol = symbol;
}


// -/----- Frame settings -----\-
void OLEDDisplayUi::setFrameAnimation(AnimationDirection dir) {
  this->frameAnimationDirection = dir;
}
void OLEDDisplayUi::setFrames(FrameCallback* frameFunctions, uint8_t frameCount) {
  this->frameFunctions = frameFunctions;
  this->frameCount     = frameCount;
  this->resetState();
}

// -/----- Overlays ------\-
void OLEDDisplayUi::setOverlays(OverlayCallback* overlayFunctions, uint8_t overlayCount){
  this->overlayFunctions = overlayFunctions;
  this->overlayCount     = overlayCount;
}

// -/----- Loading Process -----\-

void OLEDDisplayUi::setLoadingDrawFunction(LoadingDrawFunction loadingDrawFunction) {
  this->loadingDrawFunction = loadingDrawFunction;
}

void OLEDDisplayUi::runLoadingProcess(LoadingStage* stages, uint8_t stagesCount) {
  uint8_t progress = 0;
  uint8_t increment = 100 / stagesCount;

  for (uint8_t i = 0; i < stagesCount; i++) {
    display->clear();
    this->loadingDrawFunction(this->display, &stages[i], progress);
    display->display();

    stages[i].callback();

    progress += increment;
    yield();
  }

  display->clear();
  this->loadingDrawFunction(this->display, &stages[stagesCount-1], progress);
  display->display();

  delay(150);
}

// -/----- Manuel control -----\-
void OLEDDisplayUi::nextFrame() {
  if (this->state.frameState != IN_TRANSITION) {
    this->state.manuelControll = true;
    this->state.frameState = IN_TRANSITION;
    this->state.ticksSinceLastStateSwitch = 0;
    this->lastTransitionDirection = this->state.frameTransitionDirection;
    this->state.frameTransitionDirection = 1;
  }
}
void OLEDDisplayUi::previousFrame() {
  if (this->state.frameState != IN_TRANSITION) {
    this->state.manuelControll = true;
    this->state.frameState = IN_TRANSITION;
    this->state.ticksSinceLastStateSwitch = 0;
    this->lastTransitionDirection = this->state.frameTransitionDirection;
    this->state.frameTransitionDirection = -1;
  }
}

void OLEDDisplayUi::switchToFrame(uint8_t frame) {
  if (frame >= this->frameCount) return;
  this->state.ticksSinceLastStateSwitch = 0;
  if (frame == this->state.currentFrame) return;
  this->state.frameState = FIXED;
  this->state.currentFrame = frame;
  this->state.isIndicatorDrawen = true;
}

void OLEDDisplayUi::transitionToFrame(uint8_t frame) {
  if (frame >= this->frameCount) return;
  this->state.ticksSinceLastStateSwitch = 0;
  if (frame == this->state.currentFrame) return;
  this->nextFrameNumber = frame;
  this->lastTransitionDirection = this->state.frameTransitionDirection;
  this->state.manuelControll = true;
  this->state.frameState = IN_TRANSITION;
  this->state.frameTransitionDirection = frame < this->state.currentFrame ? -1 : 1;
}


// -/----- State information -----\-
OLEDDisplayUiState* OLEDDisplayUi::getUiState(){
  return &this->state;
}


int8_t OLEDDisplayUi::update(){
  long frameStart = millis();
  int8_t timeBudget = this->updateInterval - (frameStart - this->state.lastUpdate);
  if ( timeBudget <= 0) {
    // Implement frame skipping to ensure time budget is keept
    if (this->autoTransition && this->state.lastUpdate != 0) this->state.ticksSinceLastStateSwitch += ceil(-timeBudget / this->updateInterval);

    this->state.lastUpdate = frameStart;
    this->tick();
  }
  return this->updateInterval - (millis() - frameStart);
}


void OLEDDisplayUi::tick() {
  this->state.ticksSinceLastStateSwitch++;

  switch (this->state.frameState) {
    case IN_TRANSITION:
        if (this->state.ticksSinceLastStateSwitch >= this->ticksPerTransition){
          this->state.frameState = FIXED;
          this->state.currentFrame = getNextFrameNumber();
          this->state.ticksSinceLastStateSwitch = 0;
          this->nextFrameNumber = -1;
        }
      break;
    case FIXED:
      // Revert manuelControll
      if (this->state.manuelControll) {
        this->state.frameTransitionDirection = this->lastTransitionDirection;
        this->state.manuelControll = false;
      }
      if (this->state.ticksSinceLastStateSwitch >= this->ticksPerFrame){
          if (this->autoTransition){
            this->state.frameState = IN_TRANSITION;
          }
          this->state.ticksSinceLastStateSwitch = 0;
      }
      break;
  }

  this->display->clear();
  this->drawFrame();
  if (shouldDrawIndicators) {
    this->drawIndicator();
  }
  this->drawOverlays();
  this->display->display();
}

void OLEDDisplayUi::resetState() {
  this->state.lastUpdate = 0;
  this->state.ticksSinceLastStateSwitch = 0;
  this->state.frameState = FIXED;
  this->state.currentFrame = 0;
  this->state.isIndicatorDrawen = true;
}

void OLEDDisplayUi::drawFrame(){
  switch (this->state.frameState){
     case IN_TRANSITION: {
       float progress = (float) this->state.ticksSinceLastStateSwitch / (float) this->ticksPerTransition;
       int16_t x, y, x1, y1;
       switch(this->frameAnimationDirection){
        case SLIDE_LEFT:
          x = -display->getDisplayWidth() * progress;
          y = 0;
          x1 = x + display->getDisplayWidth();
          y1 = 0;
          break;
        case SLIDE_RIGHT:
          x = display->getDisplayWidth() * progress;
          y = 0;
          x1 = x - display->getDisplayWidth();
          y1 = 0;
          break;
        case SLIDE_UP:
          x = 0;
          y = -display->getDisplayHeight() * progress;
          x1 = 0;
          y1 = y + display->getDisplayHeight();
          break;
        case SLIDE_DOWN:
          x = 0;
          y = display->getDisplayHeight() * progress;
          x1 = 0;
          y1 = y - display->getDisplayHeight();
          break;
       }

       // Invert animation if direction is reversed.
       int8_t dir = this->state.frameTransitionDirection >= 0 ? 1 : -1;
       x *= dir; y *= dir; x1 *= dir; y1 *= dir;

       bool drawenCurrentFrame;


       // Prope each frameFunction for the indicator Drawen state
       this->enableIndicator();
       (this->frameFunctions[this->state.currentFrame])(this->display, &this->state, x, y);
       drawenCurrentFrame = this->state.isIndicatorDrawen;

       this->enableIndicator();
       (this->frameFunctions[this->getNextFrameNumber()])(this->display, &this->state, x1, y1);

       // Build up the indicatorDrawState
       if (drawenCurrentFrame && !this->state.isIndicatorDrawen) {
         // Drawen now but not next
         this->indicatorDrawState = 2;
       } else if (!drawenCurrentFrame && this->state.isIndicatorDrawen) {
         // Not drawen now but next
         this->indicatorDrawState = 1;
       } else if (!drawenCurrentFrame && !this->state.isIndicatorDrawen) {
         // Not drawen in both frames
         this->indicatorDrawState = 3;
       }

       // If the indicator isn't draw in the current frame
       // reflect it in state.isIndicatorDrawen
       if (!drawenCurrentFrame) this->state.isIndicatorDrawen = false;

       break;
     }
     case FIXED:
      // Always assume that the indicator is drawn!
      // And set indicatorDrawState to "not known yet"
      this->indicatorDrawState = 0;
      this->enableIndicator();
      (this->frameFunctions[this->state.currentFrame])(this->display, &this->state, 0, 0);
      break;
  }
}

void OLEDDisplayUi::drawIndicator() {

    // Only draw if the indicator is invisible
    // for both frames or
    // the indiactor is shown and we are IN_TRANSITION
    if (this->indicatorDrawState == 3 || (!this->state.isIndicatorDrawen && this->state.frameState != IN_TRANSITION)) {
      return;
    }

    uint8_t posOfHighlightFrame;
    float indicatorFadeProgress = 0;

    // if the indicator needs to be slided in we want to
    // highlight the next frame in the transition
    uint8_t frameToHighlight = this->indicatorDrawState == 1 ? this->getNextFrameNumber() : this->state.currentFrame;

    // Calculate the frame that needs to be highlighted
    // based on the Direction the indiactor is drawn
    switch (this->indicatorDirection){
      case LEFT_RIGHT:
        posOfHighlightFrame = frameToHighlight;
        break;
      case RIGHT_LEFT:
        posOfHighlightFrame = this->frameCount - frameToHighlight;
        break;
    }

    switch (this->indicatorDrawState) {
      case 1: // Indicator was not drawn in this frame but will be in next
        // Slide IN
        indicatorFadeProgress = 1 - ((float) this->state.ticksSinceLastStateSwitch / (float) this->ticksPerTransition);
        break;
      case 2: // Indicator was drawn in this frame but not in next
        // Slide OUT
        indicatorFadeProgress = ((float) this->state.ticksSinceLastStateSwitch / (float) this->ticksPerTransition);
        break;
    }

    uint16_t frameStartPos = (12 * frameCount / 2);
    const char *image;
    uint16_t x,y;
    for (byte i = 0; i < this->frameCount; i++) {

      switch (this->indicatorPosition){
        case TOP:
          y = 0 - (8 * indicatorFadeProgress);
          x = 64 - frameStartPos + 12 * i;
          break;
        case BOTTOM:
          y = 56 + (8 * indicatorFadeProgress);
          x = 64 - frameStartPos + 12 * i;
          break;
        case RIGHT:
          x = 120 + (8 * indicatorFadeProgress);
          y = 32 - frameStartPos + 2 + 12 * i;
          break;
        case LEFT:
          x = 0 - (8 * indicatorFadeProgress);
          y = 32 - frameStartPos + 2 + 12 * i;
          break;
      }

      if (posOfHighlightFrame == i) {
         image = this->activeSymbol;
      } else {
         image = this->inactiveSymbol;
      }

      this->display->drawFastImage(x, y, 8, 8, image);
    }
}

void OLEDDisplayUi::drawOverlays() {
 for (uint8_t i=0;i<this->overlayCount;i++){
    (this->overlayFunctions[i])(this->display, &this->state);
 }
}

uint8_t OLEDDisplayUi::getNextFrameNumber(){
  if (this->nextFrameNumber != -1) return this->nextFrameNumber;
  return (this->state.currentFrame + this->frameCount + this->state.frameTransitionDirection) % this->frameCount;
}

#endif
