#include <LiquidCrystal.h>

// LCD pins: RS, EN, D4, D5, D6, D7
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

void setup() {
  lcd.begin(16, 2);   // Initialize LCD with 16 columns and 2 rows
  lcd.clear();        // Clear screen

  lcd.setCursor(0, 0);   // Column 0, Row 0 (first line)
  lcd.print("HELLO");

  lcd.setCursor(0, 1);   // Column 0, Row 1 (second line)
  lcd.print("WORLD");
}

void loop() {
  // Nothing needed, text stays displayed
}

