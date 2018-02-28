//#define THERMO_DR_DEBUG 1

#ifdef THERMO_DR_DEBUG
  #define DEBUG_PRINT(...) { DEBUG_PRINTER.print(__VA_ARGS__); }
  #define DEBUG_PRINTLN(...) { DEBUG_PRINTER.println(__VA_ARGS__); }
#else
  #define DEBUG_PRINT(...) {}
  #define DEBUG_PRINTLN(...) {}
#endif


const int InitialWaitInterval = 3000; // wait interval after last target temp received
const int CalibrationRequiredTimeout = 6000; // the interval for redoing calibration

// thermostat variables
unsigned long tsLastMillis = 0;
unsigned long tsLastReceived = 0;
unsigned long tsLastCalibration = 223372036854775808L;

float targetTemp;
float nextTargetTemp = NAN;
float currentTempSet;

// Thermostat adjust stages
#define TS_STAGE_NONE -1 // No stage
#define TS_STAGE_0 0 // Wait for a timout before setting the target temp
#define TS_STAGE_1 1 // Calibrating thermostat
#define TS_STAGE_2 2 // Setting target temp, btn up
#define TS_STAGE_3 3 // Setting target temp, btn down

int tsStage = TS_STAGE_NONE;

void setNextTargetTemp(float desiredTemp) {
  DEBUG_PRINTLN(nextTargetTemp);
  nextTargetTemp = ((int)(desiredTemp*2))/2.0;
  nextTargetTemp = constrain(nextTargetTemp, MinTempSetpoint, MaxTempSetpoint);
  DEBUG_PRINT("Next target Temp: ");
  DEBUG_PRINTLN(nextTargetTemp);

  adjustToNextTargetTemp();
}

void adjustToNextTargetTemp() {
  if (tsStage <= TS_STAGE_0 && !isnan(nextTargetTemp)) {
    // start adjustment stage
    targetTemp = nextTargetTemp;
    nextTargetTemp = NAN;
    tsLastReceived = millis();
    tsStage = TS_STAGE_0;
  }
}

void downButtonPress() { // press temp Down button
  digitalWrite(BUILTIN_LED, LOW);
  DEBUG_PRINTLN("Pressing Down Btn");
  digitalWrite(DOWN_PIN, HIGH);
  currentTempSet = currentTempSet - 0.5;
}
void downButtonRelease() { // release temp Down button
  DEBUG_PRINTLN("Releasing Down Btn");
  digitalWrite(DOWN_PIN, LOW);
  digitalWrite(BUILTIN_LED, HIGH);
}

void upButtonPress() { // press temp Up button
  digitalWrite(BUILTIN_LED, LOW);
  DEBUG_PRINTLN("Pressing Up Btn");
  digitalWrite(UP_PIN, HIGH);
  currentTempSet = currentTempSet + 0.5;
  Serial.print("currentTempSet: ");
  Serial.println(currentTempSet);
}
void upButtonRelease() { // press temp Up button
  DEBUG_PRINTLN("Releasing Up Btn");
  digitalWrite(UP_PIN, LOW);
  digitalWrite(BUILTIN_LED, LOW);
}

void pressTempAdjustButton() {
  DEBUG_PRINT("Trying to adjust, current temp: ");
  DEBUG_PRINTLN(currentTempSet);
  if (targetTemp == currentTempSet) {
    return;
  }
  
  if (targetTemp > currentTempSet) {
    upButtonPress();
  } else {
    downButtonPress();
  }
}

bool releaseTempAdjustButton() {
  DEBUG_PRINTLN("Releasing Btns");
  upButtonRelease();
  downButtonRelease();
  if (targetTemp == currentTempSet) {
    DEBUG_PRINT("Temp adjusted to ");
    DEBUG_PRINTLN(currentTempSet);
    return true;
  } else {
    return false;
  }
}

bool startCalibration() {
  DEBUG_PRINTLN(tsLastCalibration);
  if (isnan(tsLastCalibration) || (millis() - tsLastCalibration > CalibrationRequiredTimeout)) {  //check if clibration is required
    DEBUG_PRINTLN("Calibration required, calibrating...");
    tsLastCalibration = millis();
    downButtonPress();
    return true;
  } else {
    DEBUG_PRINT("Calibration not required, interval: ");
    DEBUG_PRINTLN(millis() - tsLastCalibration);
    return false;
  }
}

void stopCalibration() {
  downButtonRelease();
  currentTempSet = MinTempSetpoint; // temp set to min setpoint
}

void handleTSStage() {
  unsigned long currentMillis = millis();
  
  switch (tsStage) {
    case TS_STAGE_NONE:
     return; // nothing to do

    case TS_STAGE_0:
      if (currentMillis - tsLastReceived > InitialWaitInterval) { // wait for timeout
        bool calibrationStarted = startCalibration();
        if (calibrationStarted) {
          tsLastMillis = currentMillis;
          tsStage = TS_STAGE_1; // advance to wait for calibration to happen stage
        } else { // calibration not needed
          tsLastMillis = currentMillis;
          tsStage = TS_STAGE_2; // advance to setting the target temp
        }
      }
      break;
      
    case TS_STAGE_1:
      if (currentMillis - tsLastMillis > CalibrationInterval) {
        stopCalibration();
        tsLastMillis = currentMillis;
        tsStage = TS_STAGE_2;
        
      }
      break;
      
    case TS_STAGE_2:
      if (currentMillis - tsLastMillis > BtnDepressInterval) {
        pressTempAdjustButton();
        tsLastMillis = currentMillis;
        tsStage = TS_STAGE_3;
      }
      break;
      
    case TS_STAGE_3:
      if (currentMillis - tsLastMillis > BtnDepressInterval) {
        bool finishedAdjustment = releaseTempAdjustButton();
        tsLastMillis = currentMillis;
        if (!finishedAdjustment) {
          tsStage = TS_STAGE_2;
        } else {
          tsStage = TS_STAGE_NONE; // done adjusting
          adjustToNextTargetTemp(); // adjust to next temp if present
        }
      }
      break;
  }
}


