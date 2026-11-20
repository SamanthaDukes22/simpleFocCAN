#include "Dancing_Functions.h"
#include "Data.h"

// Implementation of functions for motor motion testing

//counter
unsigned long mil = 0;
long interval = 1000;
bool flip1 = false;
bool flip2 = false;
bool lastdir = false;
//uint8_t COUNTPIN = 13;
int numberCycles = 0;
int maxCycles = 123376;
  unsigned long currentmil = 0;

// Need setup function
void initMotorDancing() {
}


// Spin 1 direction for 60 seconds then other direction for 60 seconds
void longWhirl(){/*
  currentmil = millis();
  if (currentmil - mil >= interval){
    mil = millis();
    flip1 = !flip1;
  }
  if(flip1){
    //digitalWrite(COUNTPIN, LOW);
    if(flip2){
      motor.target = 1.0;
      lastdir = true;
      interval = 5000;
    }else{
      motor.target = -1.0;
      lastdir = false;
      interval = 5000;
    }
  }else{
    motor.target = 0.0;
    interval = 5000;
    //digitalWrite(COUNTPIN, HIGH);
    numberCycles = numberCycles + 1;
    flip2 = !lastdir;
  }
  if (numberCycles >= maxCycles){
    Serial.println("LongWhirlFinished!");
    while(true){}
  }*/
}
