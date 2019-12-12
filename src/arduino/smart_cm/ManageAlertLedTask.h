#ifndef __MANAGEALERTLEDTASK__
#define __MANAGEALERTLEDTASK__

#include "Task.h"
#include "Led.h"

class ManageAlertLedTask: public Task {
  private:
    int pin;
    Led* led; 
  
  public:
    ManageAlertLedTask(int pin);
    void init(int period, State * state);
    void tick();
};
#endif
