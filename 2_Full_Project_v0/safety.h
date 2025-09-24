/**
 * Validates the current temperature and setpoint against configured safety limits.
 * - Rejects NaN readings for both temperature and setpoint.
 * - Ensures temperature is within [MIN_SAFE_TEMPERATURE_VALUE, MAX_SAFE_TEMPERATURE_VALUE].
 * - Ensures setpoint is within [POT_MIN_TEMPERATURE_VALUE, POT_MAX_TEMPERATURE_VALUE].
 *
 * @param tC    Measured temperature in °C.
 * @param setC  Desired setpoint in °C.
 * @return true  if values are valid and within bounds.
 * @return false if values are invalid.
 */

bool checkRanges(float tC, float setC) {
  if (isnan(tC)) {
    logMessage(LOG_ERROR, "TEMPERATURE NaN", tC);
    return false;
  }
  if (isnan(setC)) {
    logMessage(LOG_ERROR, "SETPOINT NaN", setC);
    return false;
  }
  if (tC < MIN_SAFE_TEMPERATURE_VALUE || tC > MAX_SAFE_TEMPERATURE_VALUE) {
    logMessage(LOG_ERROR, "TEMP OUT OF SAFE RANGE", tC);
    return false;
  }
  if (setC < POT_MIN_TEMPERATURE_VALUE || setC > POT_MAX_TEMPERATURE_VALUE) {
    logMessage(LOG_ERROR, "SETPOINT OUT OF ALLOWED RANGE", setC);
    return false;
  }
  return true;
}