#ifndef BUTTONS_H
#define BUTTONS_H

#include "MCU_config.h"
#include "ODCC_config.h"
#include "Peripherals.h"
#include <OneButton.h>

// extern int value;

enum KeyType {
  KeyNone,
  KeyUp,
  KeyDown,
  KeyLeft,
  KeyRight,
  KeyEnter,
  KeyMenuEnter,  // New key type for menu entry
  KeyEsc,
  KeyExit,
  DoubleClick  // Add this if not already defined
};

extern KeyType currentKey;

class Buttons {
public:
  Buttons();
  void begin();
  void tick();
  void beginLongPress();
  void beginDoubleClick();

  // Callbacks for button actions
  static void handleUpClick();
  static void handleDownClick();
  static void handleEnterClick();
  static void handleEnterLongPress();
  static void handleEscClick();
  static void handleDoubleClick();


  static void onLongPressUp();
  static void onLongPressDown();

  // Variables for button states and actions
  KeyType currentKey;
  int value;

private:
  void setupPins();
  static Buttons* instance;  // Static instance pointer

  OneButton buttonUP;
  OneButton buttonDOWN;
  OneButton buttonOK;
  OneButton buttonESC;

  unsigned long lastRepeatMillis = 0;
  uint16_t repeatInterval = 150;  // Adjust for slower/faster stepping (ms)
};

#endif  // BUTTONS_H
