#ifndef __MANAGETRACKINGLEDTASK__
#define __MANAGETRACKINGLEDTASK__

#include "Task.h"
#include "Led.h"
#include "State.h"

class ManageTrackLedTask: public Task {
  private:
    int pin;
    Led* led;
  
  public:
    ManageTrackLedTask(int pin);
    void init(int period, State * state);
    void tick();
};

#endif
