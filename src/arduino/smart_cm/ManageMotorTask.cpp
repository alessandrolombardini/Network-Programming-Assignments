#include "ManageMotorTask.h"

ManageMotorTask::ManageMotorTask(int pin){
  this->pin = pin;
}

void ManageMotorTask::init(int period, State * state){
  Task::init(period, state);
  motor = new ServoMotor(pin);
  motor->on();
  resetMotor();
}

void ManageMotorTask::tick(){
  /* Is exit by power down mode */
  if(actualState->isExitByPowerMode()){
    timeElapsedSinceLastMovement = 0;
    actualState->setIfExitByPowerMode(false);
  }
  /* Modality is changed: reset motor */
  if(actualState->isModalityChanged()){
    resetMotor();
  } else {
    /* If an object is tracked don't move the motor */   
    if(!actualState->isTracked()){
      if(actualState->getModality() == AUTO && checkIfHasToMove()){                                           /* Auto mode */
        setNextClove();
        moveMotor();
      } else if (actualState->getModality() == SINGLE && actualState->hasDetected() && checkIfHasToMove()){   /* Single mode */
        setNextClove();
        /* If the motor is arrived to the last clove wait to the next movement detection to restart to move */
        if(actualState->getCloveNumber() == -1 || actualState->getCloveNumber() == CLOVE_NUMBER){
          actualState->setHasDetected(false);
          /* Necessary to do double scan on end cloves: first at the end of the lap and the second at the start of the next lap */
          if(actualState->getCloveNumber() == -1){
            actualState->setCloveNumber(0);
          } else {
            actualState->setCloveNumber(CLOVE_NUMBER -1);
          }
        } else {
          moveMotor();
        }
      } else if (actualState->getModality() == MANUAL && checkIfHasToMove()){                                 /* Manual mode */
        moveMotor();
      }
    }
  }
}

void ManageMotorTask::resetMotor() { 
  actualState->setIsModaliyChanged(false); 
  timeElapsedSinceLastMovement = 0;
  moveMotor();
  delay(5);
}

void ManageMotorTask::moveMotor(){
  int angle = 180/15 * actualState->getCloveNumber();
  motor->setPosition(angle);
  delay(5);
}

void ManageMotorTask::setNextClove() {
  int actualCloveNumber = actualState->getCloveNumber();
  int direction = actualState->getMotorDirection();
  actualCloveNumber+=direction;
  if((actualState->getModality() == AUTO && (actualCloveNumber >= CLOVE_NUMBER-1 || actualCloveNumber <= 0)) ||
     (actualState->getModality() == SINGLE && (actualCloveNumber >= CLOVE_NUMBER || actualCloveNumber <= -1)) ){
    direction=-direction;
    actualState->setMotorDirection(direction);
  } 
  actualState->setCloveNumber(actualCloveNumber);
}

bool ManageMotorTask::checkIfHasToMove(){
  int timeToWait = BASE_WAIT - (actualState->getVelocity() - 1) * VELOCITY_MULTIPLIER;
  timeElapsedSinceLastMovement += getMyPeriod();
  if (timeElapsedSinceLastMovement >= timeToWait){
    timeElapsedSinceLastMovement = 0;
    return true;
  } else {
    return false;
  }
}
