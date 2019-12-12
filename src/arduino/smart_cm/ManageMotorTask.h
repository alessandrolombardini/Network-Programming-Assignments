#ifndef __MANAGEMOTORTASK__
#define __MANAGEMOTORTASK__

#include "Task.h"
#include "ServoMotor.h"

class ManageMotorTask: public Task {
  private:
    ServoMotor * motor;      
    int pin;
    int timeElapsedSinceLastMovement;     /* Time elapsed since last movement */
    void resetMotor();                    /* Reset motor status */             
    void setNextClove();                  /* Set the next clove in auto mode and single mode*/             
    void moveMotor();                     /* Move the motor to the clove setted */
    bool checkIfHasToMove();              /* Check if the time request to move is elapsed (it's calculated by velocity) */
  
  public:
    ManageMotorTask(int pin);
    void init(int period, State * state);
    void tick();
};

#endif
