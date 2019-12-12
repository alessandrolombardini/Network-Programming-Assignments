#ifndef __TIMER__
#define __TIMER__

class Timer {
  public:  
    Timer();
    void setupPeriod(int period);        /* Setup period of tick in milliseconds */  
    void waitForNextTick();              /* Wait for next tick */
};


#endif

