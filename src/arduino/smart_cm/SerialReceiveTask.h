#ifndef __SERIALRECEIVETASK__
#define __SERIALRECEIVETASK__

#include "Task.h"
#include "State.h"
#include "SerialReceiveTask.h"

class SerialReceiveTask: public Task {  
  public:
    SerialReceiveTask();
    void init(int period, State * state);
    void tick();
};

#endif