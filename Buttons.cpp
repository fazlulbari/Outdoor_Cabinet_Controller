#include "Buttons.h"

// Static instance pointer
// static Buttons* instance = nullptr;
Buttons* Buttons::instance = nullptr;
extern Peripherals peripherals;

extern bool isUserActive;
extern unsigned long lastUserActive;

Buttons::Buttons()
  : buttonUP(BUTTON_UP, true, true),
    buttonDOWN(BUTTON_DOWN, true, true),
    buttonOK(BUTTON_OK, true, true),
    buttonESC(BUTTON_ESC, true, true),
    currentKey(KeyExit),
    value(0) {
  // Set the instance pointer to the current object
  instance = this;
}

void Buttons::begin() {
  buttonUP.attachClick(handleUpClick);
  buttonDOWN.attachClick(handleDownClick);
  buttonOK.attachClick(handleEnterClick);
  buttonOK.attachLongPressStart(handleEnterLongPress);  // Long press to enter menu
  buttonESC.attachClick(handleEscClick);
}

void Buttons::beginLongPress() {

  buttonUP.attachClick(onLongPressUp);
  buttonDOWN.attachClick(onLongPressDown);
  buttonUP.attachDuringLongPress(onLongPressUp);
  buttonDOWN.attachDuringLongPress(onLongPressDown);
}

void Buttons::beginDoubleClick() {
  buttonOK.attachDoubleClick(handleDoubleClick);
}

void Buttons::tick() {
  buttonUP.tick();
  buttonDOWN.tick();
  buttonOK.tick();
  buttonESC.tick();
}
// Static callback functions

void Buttons::handleUpClick() {
  instance->currentKey = KeyUp;

  peripherals.buttonPressFeedback();
  isUserActive = true;
  lastUserActive = millis();
  DEBUG_PRINTLN("Button Pressed!");
}

void Buttons::handleDownClick() {
  instance->currentKey = KeyDown;

  peripherals.buttonPressFeedback();
  isUserActive = true;
  lastUserActive = millis();
  DEBUG_PRINTLN("Button Pressed!");
}

void Buttons::handleEnterClick() {
  instance->currentKey = KeyEnter;

  peripherals.buttonPressFeedback();
  isUserActive = true;
  lastUserActive = millis();
  DEBUG_PRINTLN("Button Pressed!");
}
void Buttons::handleEnterLongPress() {
  instance->currentKey = KeyMenuEnter;  // New key type for menu entry

  peripherals.buttonPressFeedback();
  isUserActive = true;
  lastUserActive = millis();
  DEBUG_PRINTLN("Button Pressed!");
}

void Buttons::handleEscClick() {
  instance->currentKey = KeyExit;

  peripherals.buttonPressFeedback();
  isUserActive = true;
  lastUserActive = millis();
  DEBUG_PRINTLN("Button Pressed!");
}

void Buttons::onLongPressUp() {
  unsigned long now = millis();
  if (now - instance->lastRepeatMillis >= instance->repeatInterval) {
    instance->value += 1;
    instance->value = constrain(instance->value, 0, 512);
    instance->lastRepeatMillis = now;
    isUserActive = true;
    lastUserActive = now;
    DEBUG_PRINTLN("Button Pressed!");
  }
}


void Buttons::onLongPressDown() {
  unsigned long now = millis();
  if (now - instance->lastRepeatMillis >= instance->repeatInterval) {
    instance->value -= 1;
    instance->value = constrain(instance->value, 0, 512);
    instance->lastRepeatMillis = now;
    isUserActive = true;
    lastUserActive = now;
    DEBUG_PRINTLN("Button Pressed!");
  }
}




void Buttons::handleDoubleClick() {
  instance->currentKey = DoubleClick;

  isUserActive = true;
  lastUserActive = millis();
  DEBUG_PRINTLN("Button Pressed!");
}
