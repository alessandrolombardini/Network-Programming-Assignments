#include "ManagePotentiometerTask.h"

ManagePotentiometerTask::ManagePotentiometerTask(int pin){
  this->pin = pin;
}

void ManagePotentiometerTask::init(int period, State * state){
  Task::init(period, state);
  potentiometer = new Potentiometer(pin);
  lastVelocityRead = readVelocity();
}

void ManagePotentiometerTask::tick(){
  int velocity = readVelocity();
  /* If velocity just read is different by last velocity read, we have to set that value like actual system velocity 
     becouse someone has changed potentiometer value. Otherwise, if the value hasn't changed, we haven't to do anything
     becouse or it isn't changed or it is setted by GUI. */
  if (velocity != lastVelocityRead){
    actualState->setVelocity(velocity);
    lastVelocityRead = velocity;
    /* Send velocity to GUI */
    MsgService.composeMessage("Speed: ");
    MsgService.composeMessage((String) actualState->getVelocity());
    MsgService.sendComposedMessage();
  }
}

int ManagePotentiometerTask::readVelocity(){
  int analogValue = analogRead(pin);  
  int velocity = convertIntoVelocity(analogValue);
  return velocity;
}

int ManagePotentiometerTask::convertIntoVelocity(int analogValue){
  int temp = analogValue % 128 ? analogValue/128 + 1 : analogValue/128; 
  temp += temp == 0 ? 1 : 0;
  return temp;
}
