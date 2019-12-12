#ifndef __EVALUATEMESSAGESTASK__
#define __EVALUATEMESSAGESTASK__

#include "Task.h"

class EvaluateMessagesTask: public Task {
  private:
    void evaluateMessage(); 
    void initModalitySoft();
    void initModality();
  
  public:
    EvaluateMessagesTask();
    void init(int period, State * state);
    void tick();
};
#endif
