#ifndef PERIPHERALS_H
#define PERIPHERALS_H

#include "ODCC_config.h"

class Peripherals {
public:
  Peripherals();
  void begin();
  void triggerRelay(int relay, bool state);
  void controlLight(bool state);
  void alertBuzzer(int count);
  void confirmBeep();  // Short confirmation beep

  void buttonPressFeedback();

private:
  void setupPins();
};

#endif  // PERIPHERALS_H
