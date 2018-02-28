
// Living Room header file

#define ESP_ID 2c1db6

const char* clientID = "esp8266_Bed"; 
const char* TempOutTopic = "/ha/bedroom/temp";
const char* HumOutTopic = "/ha/bedroom/hum";
const char* TargetTempInTopic = "/ha/bedroom/targetTemp";
const float TempAdjustment = -2.0; // used to adjust the temperature to the wired thermosat
const float MinTempSetpoint = 5; // lowest temp setpoint
const float MaxTempSetpoint = 30.0; // highest temp setpoint
const int CalibrationInterval = 5000; // interval required for calibration
const int BtnPressInterval = 80; // interval to keep button pushed
const int BtnDepressInterval = 80; // interval between button push


