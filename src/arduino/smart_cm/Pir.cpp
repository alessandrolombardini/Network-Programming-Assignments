#include "Pir.h"
#include "Arduino.h"
#include "MsgService.h"

#define CALIBRATION_TIME_SEC 7

Pir::Pir(int pin){
    this->pin = pin;
    pinMode(pin, INPUT);
    calibrate();
}

void Pir::calibrate(){
    MsgService.sendMsg("Calibrating pir...");
    for(int i = 0; i < CALIBRATION_TIME_SEC; i++){
        delay(1000);
    }
    MsgService.sendMsg("Pir ready");
}

bool Pir::detect(){
    int detected = digitalRead(this->pin);
    if (detected == HIGH){
        return true;
    }
    return false;
}
