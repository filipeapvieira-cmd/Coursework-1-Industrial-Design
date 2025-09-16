#include <LiquidCrystal.h>
#include <stdio.h>

// LCD pins
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

// Pins
const int TMP36_PIN = A0;
const int POTENTIOMETER_PIN = A1;
const int LED_PIN = 7;
const int MOSFET_PIN = 9;  // NMOS gate (motor)

// System parameters
const float V_REF = 5.0;                         // Analog reference voltage
const float R_BITS = 10.0;                       // ADC resolution (bits)
const float ADC_STEPS = (1 << int(R_BITS)) - 1;  // Number of steps (2^R_BITS - 1)
const float TOLERANCE = 0.5;                     // °C

// Logger
enum LogLevel : uint8_t { LOG_ERROR = 0,
                          LOG_WARN = 1,
                          LOG_INFO = 2,
                          LOG_DEBUG = 3 };
const bool LOGGING_ACTIVE = true;  // // Set to false to silence all logs


/**
 * Converts a LogLevel enum into a label.
 *
 * @param lv  Logging level (LOG_ERROR, LOG_WARN, LOG_INFO, LOG_DEBUG).
 * @return const char*  Pointer to a string literal ("ERROR", "WARN", "INFO", "DEBUG").
 * */
inline const char* levelToStr(LogLevel lv) {
  switch (lv) {
    case LOG_ERROR: return "ERROR";
    case LOG_WARN: return "WARN";
    case LOG_INFO: return "INFO";
    default: return "DEBUG";
  }
}

/**
 * Emits a timestamped log line over the Serial port in the format:
 *   "<millis> ms LEVEL: message"
 *
 * @param level    Logging severity (LOG_ERROR, LOG_WARN, LOG_INFO, LOG_DEBUG).
 * @param message  Label printed before the value (e.g., "TempC").
 * @return void
 * */
inline void logMessage(LogLevel level, const char* message) {
  if (!LOGGING_ACTIVE) return;
  Serial.print(millis());
  Serial.print(" ms ");
  Serial.print(levelToStr(level));
  Serial.print(": ");
  Serial.println(message);
}

/**
 * Emits a timestamped log line over the Serial port in the format:
 *   "<millis> ms LEVEL: message=value"
 *
 * @param level    Logging severity (LOG_ERROR, LOG_WARN, LOG_INFO, LOG_DEBUG).
 * @param message  Label printed before the value (e.g., "TempC").
 * @param value    Numeric value to print after 'message='.
 * @param digits   Number of decimal places for the value (default: 2).
 * @return void
 * */
inline void logMessage(LogLevel level, const char* message, float value, int digits = 2) {
  if (!LOGGING_ACTIVE) return;
  Serial.print(millis());
  Serial.print(" ms ");
  Serial.print(levelToStr(level));
  Serial.print(": ");
  Serial.print(message);
  Serial.print('=');
  Serial.println(value, digits);
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
 * The value is mapped to the range 15–35 °C, so the 
 * potentiometer acts as a user-adjustable temperature control.
 *
 * @return float  Temperature setpoint in degrees Celsius (15–35 °C).
 */
float readSetpointC() {
  int rawValue = analogRead(POTENTIOMETER_PIN);
  long setC = map(rawValue, 0, 1023, 15, 35);  // 15–35 °C

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
 * Controls the motor (and LED) with tolerance to avoid rapid switching.
 *
 * @param tC     Current measured temperature (°C).
 * @param setC   Temperature setpoint (°C).
 * @return bool  Current motor state (true = ON, false = OFF).
 */
bool controlMotor(float tC, float setC) {
  static bool isMotorStateOn = false;
  bool prev = isMotorStateOn;

  if (tC > setC + TOLERANCE) {
    isMotorStateOn = true;
  } else if (tC < setC - TOLERANCE) {
    isMotorStateOn = false;
  }

  if (isMotorStateOn != prev) {
    if (isMotorStateOn) {
      logMessage(LOG_INFO, "CHANGE - MOTOR ON", setC + TOLERANCE);
    } else {
      logMessage(LOG_INFO, "CHANGE - MOTOR OFF", setC - TOLERANCE);
    }
  }

  return isMotorStateOn;
}

void setup() {
  Serial.begin(9600);

  pinMode(LED_PIN, OUTPUT);
  pinMode(MOSFET_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  digitalWrite(MOSFET_PIN, LOW);

  lcd.begin(16, 2);
  lcd.clear();

  delay(100);
  Serial.print('\n');
  logMessage(LOG_DEBUG, "INITIAL READING - TEMPERATURE C ", readTemperatureC());
  logMessage(LOG_DEBUG, "INITIAL READING - SETPOINT ", readSetpointC());
}

void loop() {
  float tC = readTemperatureC();
  float setC = readSetpointC();
  bool isMotorStateOn = controlMotor(tC, setC);

  // LED and Motor via NMOS
  digitalWrite(LED_PIN, isMotorStateOn ? HIGH : LOW);
  digitalWrite(MOSFET_PIN, isMotorStateOn ? HIGH : LOW);

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
