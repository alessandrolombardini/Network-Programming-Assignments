#include "ManageTrackLedTask.h"

ManageTrackLedTask::ManageTrackLedTask(int pin){
  this->pin = pin;
}

void ManageTrackLedTask::init(int period, State * state){
  Task::init(period, state);
  led = new Led(pin);
}

void ManageTrackLedTask::tick(){
  switch (actualState->isTracked()){
    case true:
      led->switchOn();
      break;
    case false:
      led->switchOff();
      break;
   }
}
