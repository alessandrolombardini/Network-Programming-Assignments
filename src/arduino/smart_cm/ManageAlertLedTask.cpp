#include "ManageAlertLedTask.h"

ManageAlertLedTask::ManageAlertLedTask(int pin){
  this->pin = pin;
}

void ManageAlertLedTask::init(int period, State * state){
  Task::init(period, state);
  led = new Led(pin);
}

void ManageAlertLedTask::tick(){
  if(actualState->isAlarmed()){
      led->switchOn();
      delay(5);
      led->switchOff();
  }
}
