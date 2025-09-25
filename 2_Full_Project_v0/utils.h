
/**
 * Returns the median value of three integers.
 *
 * @param a  First reading
 * @param b  Second reading
 * @param c  Third reading
 * @return   The median of the three values
 */
int median3(int a, int b, int c) {
  if ((a >= b && a <= c) || (a <= b && a >= c)) return a;
  else if ((b >= a && b <= c) || (b <= a && b >= c)) return b;
  else return c;
}

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
  int r1 = analogRead(TMP36_PIN);
  int r2 = analogRead(TMP36_PIN);
  int r3 = analogRead(TMP36_PIN);
  int rawValue = median3(r1, r2, r3);

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