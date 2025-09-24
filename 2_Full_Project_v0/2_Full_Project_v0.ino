#include <LiquidCrystal.h>
#include <stdio.h>
#include "config.h"
#include "logging.h"
#include "safety.h"

// LCD pins
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

/**
 * Reads the TMP36 analog sensor and converts the raw ADC value to °C.
 *
 * Conversion formula (TMP36 datasheet, p. 8):
 *
 * At 0 °C, the TMP36 outputs 0.5 V, with a scale factor of 10 mV/°C.
 *
 * @return float  Temperature in degrees Celsius.
 * @see https://www.analog.com/media/en/technical-documentation/data-sheets/TMP35_36_37.pdf
 */
float readTemperatureC() {
  /**
   * Reads the raw analog value (0–1023) from the specified pin.
   *
   * The ADC maps input voltages between 0 V and the reference voltage
   * into integer values between 0 and 1023.
   *
   * @return int  Raw ADC value (0–1023).
   * @see https://docs.arduino.cc/language-reference/en/functions/analog-io/analogRead/
  */
  int rawValue = analogRead(TMP36_PIN);
  float voltage = (rawValue / ADC_STEPS) * V_REF;
  return (voltage - 0.5f) * 100.0f;
}

/**
 * Reads the potentiometer and converts its raw ADC value (0–1023)
 * into a temperature setpoint in °C.
 *
 * The value is mapped to the range POT_MIN_TEMPERATURE_VALUE–POT_MAX_TEMPERATURE_VALUE °C, so the 
 * potentiometer acts as a user-adjustable temperature control.
 *
 * @return float  Temperature setpoint in degrees Celsius.
 */
float readSetpointC() {
  int rawValue = analogRead(POTENTIOMETER_PIN);
  long setC = map(rawValue, 0, 1023, POT_MIN_TEMPERATURE_VALUE, POT_MAX_TEMPERATURE_VALUE);

  static bool seeded = false;
  static long lastSetC = -1;

  if (!seeded) {
    lastSetC = setC;
    seeded = true;  // don’t log the first time (already logged in setup)
  } else if (setC != lastSetC) {
    char label[48];
    snprintf(label, sizeof(label), "CHANGE - SETPOINT - FROM %ld C TO %ld C", lastSetC, setC);
    logMessage(LOG_INFO, label);
    lastSetC = setC;
  }

  return (float)setC;
}


/**
 * Two-speed motor control with 1s hold-time (no tolerance).
 * - Turn ON only if tC > setC continuously for HOLD_MS.
 * - Turn OFF only if tC < setC continuously for HOLD_MS.
 * - While ON: LOW speed unless tC >= setC*(1+MAX_OVER_RATIO) → HIGH.
 *
 * @return uint8_t PWM applied (0, PWM_LOW, or PWM_HIGH)
 */
uint8_t updateMotorTwoSpeed(float tC, float setC) {
  static bool on = false;
  static uint8_t curPwm = 0;

  // Timers for "over" and "under" conditions
  static bool overTiming = false, underTiming = false;
  static unsigned long overStart = 0, underStart = 0;
  unsigned long now = millis();

  // ON condition: tC > setC for >= HOLD_MS
  if (tC > setC) {
    if (!overTiming) {
      overTiming = true;
      overStart = now;
    }
    if (!on && (now - overStart >= HOLD_MS)) {
      on = true;
      logMessage(LOG_INFO, "CHANGE - MOTOR ON");
    }
  } else {
    overTiming = false;
  }

  // OFF condition: tC < setC for >= HOLD_MS
  if (tC < setC) {
    if (!underTiming) {
      underTiming = true;
      underStart = now;
    }
    if (on && (now - underStart >= HOLD_MS)) {
      on = false;
      logMessage(LOG_INFO, "CHANGE - MOTOR OFF");
    }
  } else {
    underTiming = false;
  }

  // Decide target PWM
  uint8_t targetPwm = 0;
  if (on) {
    const float highThresholdC = setC * (1.0f + MAX_OVER_RATIO);
    targetPwm = (tC >= highThresholdC) ? PWM_HIGH : PWM_LOW;
  }

  // Apply outputs (log on change)
  if (targetPwm != curPwm) {
    char msg[64];
    snprintf(msg, sizeof(msg), "CHANGE - MOTOR PWM FROM %u TO %u", curPwm, targetPwm);
    logMessage(LOG_INFO, msg);
    curPwm = targetPwm;
  }

  analogWrite(MOSFET_PIN, curPwm);
  digitalWrite(LED_PIN, curPwm > 0 ? HIGH : LOW);

  return curPwm;
}

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
