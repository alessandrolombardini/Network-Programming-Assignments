#ifndef __MANAGEPOTENTIOMETERTASK__
#define __MANAGEPOTENTIOMETERTASK__

#include "Task.h"
#include "Potentiometer.h"
#include "MsgService.h"

class ManagePotentiometerTask: public Task {
  private:
    int pin;
    Potentiometer * potentiometer;
    int lastVelocityRead;                       /* Usefull to check if actual velocity is setted by gui and not by potentiometer read */
    int readVelocity();
    int convertIntoVelocity(int analogValue);
  
  public:
    ManagePotentiometerTask(int pin);
    void init(int period, State * state);
    void tick();
};

#endif
