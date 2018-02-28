
// Living Room header file

#define ESP_ID 0fe5a1

String MQTT_PATH = "/ha/livroom";

const char* clientID = "esp8266_Liv"; 
const char* TempOutTopic = "/ha/livroom/temp";
const char* HumOutTopic = "/ha/livroom/hum";
const char* TargetTempInTopic = "/ha/livroom/targetTemp";
const float TempAdjustment = 0.0; // used to adjust the temperature to the wired thermosat
const float MinTempSetpoint = 2.5; // lowest temp setpoint
const float MaxTempSetpoint = 30.0; // highest temp setpoint
const int CalibrationInterval = 8000; // interval required for calibration
const int BtnPressInterval = 80; // interval to keep button pushed
const int BtnDepressInterval = 80; // interval between button push


