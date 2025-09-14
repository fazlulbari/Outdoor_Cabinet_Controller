#ifndef LCD_H
#define LCD_H

#include "MCU_config.h"
#include <LiquidCrystal.h>
#include <STM32FreeRTOS.h>

class LCD {
public:
  LCD();
  void begin();

  // Single argument template
  template<typename T>
  // void print(T message);
  void print(T message) {
    lcd.print(message);
  }

  // Two-argument overload for float + decimal places
  void print(float message, int digits) {
    lcd.print(message, digits);
  }

  void clear();
  void setCursor(uint8_t col, uint8_t row);
  void backlight(uint8_t brightness);
  void backlightFlash();
  void controlBacklight(bool isUserActive);
  void scrollText(const String &message, uint8_t row, uint8_t smoothness, uint8_t maxCycles);
  void scrollTextF(const char *message, uint8_t row, uint8_t smoothness, uint8_t maxCycles);

private:
  LiquidCrystal lcd;
};

#endif  // LCD_H
