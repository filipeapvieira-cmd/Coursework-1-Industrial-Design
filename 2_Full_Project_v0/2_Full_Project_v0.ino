#include <LiquidCrystal.h>
#include <stdio.h>
#include "config.h"
#include "logging.h"
#include "safety.h"
#include "utils.h"

// LCD pins
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

void setup() {
  Serial.begin(9600);

  pinMode(LED_PIN, OUTPUT);
  pinMode(MOSFET_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  digitalWrite(MOSFET_PIN, LOW);

  lcd.begin(16, 2);
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Initializing...");

  delay(500);

  lcd.clear();

  Serial.print('\n');
  logMessage(LOG_DEBUG, "INITIAL READING - TEMPERATURE C ", readTemperatureC());
  logMessage(LOG_DEBUG, "INITIAL READING - SETPOINT ", readSetpointC());
}

void loop() {
  float tC = readTemperatureC();
  float setC = readSetpointC();
  bool isSafe = checkRanges(tC, setC);

  if (!isSafe) {
    digitalWrite(MOSFET_PIN, LOW);
    digitalWrite(LED_PIN, LOW);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("ERROR - ALERT");
    delay(5000);
  } else {
    updateMotorTwoSpeed(tC, setC);

    // LCD
    lcd.setCursor(0, 0);
    lcd.print("Temp:");
    lcd.print(tC, 1);
    lcd.write((byte)223);
    lcd.print("C  ");

    lcd.setCursor(0, 1);
    lcd.print("Set:");
    lcd.print(setC, 1);
    lcd.write((byte)223);
    lcd.print("C ");

    delay(200);
  }
}
