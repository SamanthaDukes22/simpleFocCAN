#include <Arduino.h>

#include "CAN_Com.h"
#include "Dancing_Functions.h"
#include "Data.h"
#include "Motor_Control.h"
#include "Watchdog.h"

// Motor 1 implementation

void setup() {

  Serial.begin(115200); // output the debug information to the serial

  // Start CAN first so the ESP32 sees a healthy, ACKing bus even if the motor side fails to
  // initialize. Move this after initMotor() later if you prefer.
  setupCAN();

  if (!initMotor()) {
      Serial.println("Motor initialization failed!");
      while (true) {}
  }

  // initMotorDancing();

  // initWatchdog();
}


void loop() {

  receiveCANCommand();
  updateMotor();

  // Sample motor data @ 100 Hz and send to ESP32
  static uint32_t lastTelemetryUs = 0;
  uint32_t nowUs = micros();

  if ((uint32_t)(nowUs - lastTelemetryUs) >= 10000U) {   // 10,000 us = 100 Hz

    lastTelemetryUs += 10000U;                            // fixed step, no drift

    // If the loop stalled for a long time, resync instead of sending a burst of catch-up frames
    if ((uint32_t)(nowUs - lastTelemetryUs) > 50000U) {
      lastTelemetryUs = nowUs;
    }

    updateMotorTelemetry();
    transmitCurrentTelemetry();
    transmitDynamicTelemetry();
  }
}
