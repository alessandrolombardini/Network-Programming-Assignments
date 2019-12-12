#include "ServoMotor.h"
#include "Arduino.h"

ServoMotor::ServoMotor(int pin){
  this->pin = pin;  
} 

void ServoMotor::on(){
  motor.attach(pin);    
}

void ServoMotor::setPosition(int angle){
  float coeff = (MAX_PULSE_WIDTH-MIN_PULSE_WIDTH)/180;
  motor.write(MIN_PULSE_WIDTH + angle*coeff);                 
}

void ServoMotor::off(){
  motor.detach();    
}
