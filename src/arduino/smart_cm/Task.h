#ifndef __TASK__
#define __TASK__

#include "State.h"
#include "Arduino.h"
#include "MsgService.h"

class Task {
  private:
    int myPeriod;
    int timeElapsed;

  protected:
    State * actualState;
    virtual int getMyPeriod(){
      return myPeriod;
    }
    
  public:
    virtual void tick() = 0;
    virtual void init(int period, State * state){
      myPeriod = period;
      timeElapsed = 0;
      actualState = state;
    }
    virtual bool updateAndCheckTime(int basePeriod){
      timeElapsed += basePeriod;
      if (timeElapsed >= myPeriod){
        timeElapsed = 0;
        return true;
      } else {
        return false;
      }
    }
};

#endif
