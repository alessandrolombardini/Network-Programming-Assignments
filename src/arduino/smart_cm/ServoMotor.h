#ifndef __SERVO_MOTOR__
#define __SERVO_MOTOR__

#include "ServoTimer2.h"

class ServoMotor {
  private:
    int pin; 
    ServoTimer2 motor; 
    
  public:
    ServoMotor(int pin);
    void on();
    void setPosition(int angle);
    void off();
};

#endif
