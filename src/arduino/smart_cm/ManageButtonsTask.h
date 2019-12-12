#ifndef __MANAGEBUTTONSTASK__
#define __MANAGEBUTTONSTASK__

#include "Task.h"
#include "Button.h"
#include "MsgService.h"

class ManageButtonsTask: public Task {
  private:
    const int BOUNCE_DELAY = 150;
    int pinSingle;
    int pinManual;
    int pinAuto;
    Button * buttonSingle;
    Button * buttonManual;
    Button * buttonAuto;
    long prevTimeSingle = 0; /* Usefull to manage the bouncing of the single mode button */
    long prevTimeManual = 0; /* Usefull to manage the bouncing of the manual mode button */
    long prevTimeAuto = 0;   /* Usefull to manage the bouncing of the auto mdoe button */
    void printModalityUsed();
    void initModality();
    void initModalitySoft(); /* Without set motor to the initial position */

  
  public:
    ManageButtonsTask(int pinSingle, int pinManual, int pinAuto);
    void init(int period, State * state);
    void tick();
};

#endif
