#ifndef __MANAGEPOWERDOWNTASK__
#define __MANAGEPOWERDOWNTASK__

#include "Task.h"
#include "Pir.h"

#define SERIAL_PIN 0

class ManagePowerDownTask: public Task {
  private:
    Pir * pir;
    int pinPir;
    int pinAutoButton;
    int pinManualButton;

  public:
    ManagePowerDownTask(int pinPir, int pinAutoButton, int pinManualButton);
    void init(int period, State * state);
    void tick();
};

#endif
