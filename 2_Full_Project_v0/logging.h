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