#include "LCD.h"

int maxBacklightBrightness = 255;
int minBacklightBrightness = 20;
uint8_t backlightBrightness = 255;

unsigned long lastFadeTime = 0;
const uint8_t FADE_STEP = 3;
const unsigned long FADE_INTERVAL = 20;


LCD::LCD()
  : lcd(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7) {}

void LCD::begin() {
  lcd.begin(LCD_COL, LCD_ROW);
  pinMode(LCD_BL, OUTPUT);  // Set backlight pin as output
                            // backlight(maxBacklightBrightness);

  analogWrite(LCD_BL, backlightBrightness);
}

// template <typename T>
// void LCD::print(T message) {
//     lcd.print(message);
// }

// void LCD::clear() {
//   lcd.clear();
// }

void LCD::clear() {
  lcd.setCursor(0, 0);
  lcd.print("                ");
  lcd.setCursor(0, 1);
  lcd.print("                ");
}

void LCD::setCursor(uint8_t col, uint8_t row) {
  lcd.setCursor(col, row);
}

void LCD::backlight(uint8_t brightness) {
  analogWrite(LCD_BL, brightness);
}

void LCD::backlightFlash() {
  for (int i = 0; i < 2; i++) {  // Flash twice
    analogWrite(LCD_BL, 200);    // High brightness
    vTaskDelay(100 / portTICK_PERIOD_MS);

    analogWrite(LCD_BL, 10);  // Low brightness
    vTaskDelay(200 / portTICK_PERIOD_MS);
  }

  // After flashing, set backlight back to normal brightness
  analogWrite(LCD_BL, backlightBrightness);
}

void LCD::controlBacklight(bool isUserActive) {
  unsigned long currentTime = millis();

  if (isUserActive) {
    backlightBrightness = maxBacklightBrightness;
    analogWrite(LCD_BL, backlightBrightness);
    // digitalWrite(LCD_BL, HIGH);
  } else {
    if (backlightBrightness > 0 && (currentTime - lastFadeTime >= FADE_INTERVAL)) {
      backlightBrightness = max(backlightBrightness - FADE_STEP, minBacklightBrightness);
      analogWrite(LCD_BL, backlightBrightness);
      // digitalWrite(LCD_BL, LOW);
      lastFadeTime = currentTime;
    }
  }
}



void LCD::scrollText(const String &message, uint8_t row, uint8_t smoothness, uint8_t maxCycles) {
  // Constrain smoothness: 0 (fast) to 10 (slow)
  smoothness = constrain(smoothness, 0, 10);

  // Convert smoothness level to delay in milliseconds
  uint16_t delayTime = 50 + (10 - smoothness) * 95;  // Range: 50ms to 1000ms

  // String scrollMessage = String(' '* LCD_COL) + message + String(' '* LCD_COL);  // Pad both sides
  String scrollMessage = "                " + message + "                ";  // Add padding to the message
  int scrollLength = scrollMessage.length();

  for (uint8_t cycle = 0; cycle < maxCycles; cycle++) {
    for (int i = 0; i <= scrollLength - LCD_COL; i++) {
      String frame = scrollMessage.substring(i, i + LCD_COL);

      lcd.setCursor(0, row);
      lcd.print(frame);
      // vTaskDelay(delayTime / portTICK_PERIOD_MS);
      delay(delayTime);
    }
  }
}
