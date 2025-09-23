// Pins
const int TMP36_PIN = A0;
const int POTENTIOMETER_PIN = A1;
const int LED_PIN = 7;
const int MOSFET_PIN = 9;  // NMOS gate (motor)

// System parameters
const float V_REF = 5.0;                         // Analog reference voltage
const float R_BITS = 10.0;                       // ADC resolution (bits)
const float ADC_STEPS = (1 << int(R_BITS)) - 1;  // Number of steps (2^R_BITS - 1)

// Motor parameters
const unsigned long HOLD_MS = 2000;   // 2 second over/under setpoint
const uint8_t PWM_LOW = 100;          // base speed after turning ON (0..255)
const uint8_t PWM_HIGH  = 250;        // max speed
const float   MAX_OVER_RATIO = 0.15f; // 15% above setC → go MAX

// Potentiometer parameters
const int POT_MIN_TEMPERATURE_VALUE = 15; // Minimum value allowed in the potentiometer
const int POT_MAX_TEMPERATURE_VALUE = 35;

// Temperature sensor parameters
const int MIN_SAFE_TEMPERATURE_VALUE = 0;  // Minimum temperature considered safe
const int MAX_SAFE_TEMPERATURE_VALUE = 50; // Maximum temperature considered safe

// Logger
enum LogLevel : uint8_t { LOG_ERROR = 0,
                          LOG_WARN = 1,
                          LOG_INFO = 2,
                          LOG_DEBUG = 3 };
const bool LOGGING_ACTIVE = true;  // Set to false to silence all logs