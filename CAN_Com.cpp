#include <ACAN_T4.h>
#include <math.h>
#include <string.h>

#include "CAN_Com.h"
#include "Data.h"
#include "Motor_Control.h"

// Implementation of functions for CAN communication between Teensy and ESP32
// ACAN_T4 docuementation: https://github.com/pierremolinaro/acan-t4

// Set to 1 for per-command prints (slows the FOC loop at 100 Hz, so keep off normally)
#define CAN_DEBUG_PRINTS 0

bool handleCANCommand(CANMessage &message);

// Print at most once per second so a persistent fault can't flood the serial port
static void warnRateLimited(const char* msg) {
  static uint32_t lastWarnMs = 0;
  if (millis() - lastWarnMs >= 1000) {
    lastWarnMs = millis();
    Serial.println(msg);
  }
}


void setupCAN() {

  // returns settings object fully initialized with CAN bit settings for the wished bitrate
  ACAN_T4_Settings settings (CAN_BAUD_RATE) ;

  settings.mRxPin = 23 ; // 13 or 23 allowed
  settings.mTxPin = 22 ; // 11 or 22 allowed

  settings.mLoopBackMode = false ;
  settings.mSelfReceptionMode = false ;

  // Only accept this motor's command ID
  const ACANPrimaryFilter primaryFilters [] = {
    ACANPrimaryFilter (kData, kStandard, CAN_ID_MOTOR_COMMAND)
  };

  const uint32_t errorCode = ACAN_T4::can1.begin (settings, primaryFilters , 1) ;

  if (0 == errorCode) {
    Serial.print("Motor ") ;
    Serial.print(MOTOR_NUMBER);
    Serial.print(" ready for CAN communication, command id 0x");
    Serial.print(CAN_ID_MOTOR_COMMAND, HEX);
    Serial.print(", current telemetry id 0x");
    Serial.print(CAN_ID_MOTOR_CURRENT_TELEMETRY, HEX);
    Serial.print(", dynamic telemetry id 0x");
    Serial.println(CAN_ID_MOTOR_DYNAMIC_TELEMETRY, HEX);
  }
  else {
    Serial.print("ERROR: Motor ") ;
    Serial.print(MOTOR_NUMBER);
    Serial.print(" not initiated successfully, error 0x") ;
    Serial.println (errorCode, HEX) ;
  }
}


// Receive motor current commands from the ESP32 (call every loop pass)
void receiveCANCommand() {

  CANMessage message;

  // Drain everything waiting so a backlog can't build if updateMotor() is slow
  while (ACAN_T4::can1.receive(message)) {

    // Verify standard data frame and correct motor command ID
    if (!message.rtr && !message.ext &&
        message.id == CAN_ID_MOTOR_COMMAND &&
        message.len == sizeof(float)) {

      handleCANCommand(message);
    }
  }
}


// Handle motor command and store in motor command structure
bool handleCANCommand(CANMessage &message) {

  float current_q_cmd;

  memcpy(&current_q_cmd, message.data, sizeof(float));

  if (!isfinite(current_q_cmd)) {
    warnRateLimited("ERROR: Invalid current command!");
    return false;
  }

  if (fabsf(current_q_cmd) > MAX_CURRENT) {
    warnRateLimited("ERROR: Current command exceeds limit!");
    return false;
  }

  setMotorCurrent(current_q_cmd);

#if CAN_DEBUG_PRINTS
  Serial.print("Motor ");
  Serial.print(MOTOR_NUMBER);
  Serial.print(" accepted command: ");
  Serial.println(current_q_cmd, 4);
#endif

  return true;
}


// Transmit q and d current measurements
void transmitCurrentTelemetry() {

  CANMessage message;

  message.id = CAN_ID_MOTOR_CURRENT_TELEMETRY;
  message.ext = false;
  message.rtr = false;
  message.len = 8;

  // Package q and d currents [mA]
  memcpy(&message.data[0], &motorTelemetry.current_q, sizeof(float));
  memcpy(&message.data[4], &motorTelemetry.current_d, sizeof(float));

  // tryToSend only reports whether the frame entered the driver TX buffer,
  // not whether anyone on the bus ACKed it
  if (!ACAN_T4::can1.tryToSend(message)) {
    warnRateLimited("ERROR: Current telemetry CAN buffer full!");
  }
}

// Transmit motor velocity and angle measurements
void transmitDynamicTelemetry() {

  CANMessage message;

  message.id = CAN_ID_MOTOR_DYNAMIC_TELEMETRY;
  message.ext = false;
  message.rtr = false;
  message.len = 8;

  // Package velocity and angle [rad/s , rad]
  memcpy(&message.data[0], &motorTelemetry.velocity, sizeof(float));
  memcpy(&message.data[4], &motorTelemetry.angle, sizeof(float));

  if (!ACAN_T4::can1.tryToSend(message)) {
    warnRateLimited("ERROR: Dynamic telemetry CAN buffer full!");
  }
}