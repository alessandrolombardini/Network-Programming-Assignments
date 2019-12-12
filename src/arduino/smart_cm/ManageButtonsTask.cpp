#include "ManageButtonsTask.h"

ManageButtonsTask::ManageButtonsTask(int pinSingle, int pinManual, int pinAuto){
  this->pinSingle = pinSingle;
  this->pinManual = pinManual;
  this->pinAuto = pinAuto;
}

void ManageButtonsTask::init(int period, State * state){
  Task::init(period, state);
  buttonSingle = new Button(this->pinSingle);
  buttonManual = new Button(this->pinManual);
  buttonAuto = new Button(this->pinAuto);
  printModalityUsed();
}

void ManageButtonsTask::tick(){
  Modality pastValue = actualState->getModality();
  /* Check if a button is pressed */
  bool hasBeenPressed = false;
  if (buttonSingle->isPressed()){
    long currentTime = millis();
    if (currentTime - prevTimeSingle > BOUNCE_DELAY) {
      prevTimeSingle = currentTime;
      actualState->setModality(SINGLE);
    }
    hasBeenPressed = true;
  }
  if (buttonManual->isPressed()){
    long currentTime = millis();
    if (currentTime - prevTimeManual > BOUNCE_DELAY) {
      prevTimeManual = currentTime;
      actualState->setModality(MANUAL);
    }
    hasBeenPressed = true;
  }
  if (buttonAuto->isPressed()){
    long currentTime = millis();
    if (currentTime - prevTimeAuto > BOUNCE_DELAY) {
      prevTimeAuto = currentTime;
      actualState->setModality(AUTO);
    }
    hasBeenPressed = true;
  }
  /* If a button was pressed, reset all state values and change modality */
  if(hasBeenPressed){
    if(pastValue != actualState->getModality()){
      printModalityUsed();
      if(actualState->getModality() == SINGLE){
        initModality();
      } else{
        initModalitySoft();
      }
    }
  }
}

void ManageButtonsTask::printModalityUsed(){
  MsgService.composeMessage("Modality selected: ");
  MsgService.composeMessage(actualState->getModality() == SINGLE ? "SINGLE" : (actualState->getModality() == AUTO ? "AUTO" : "MANUAL"));
  MsgService.sendComposedMessage();
}

void ManageButtonsTask::initModalitySoft(){
  actualState->setAlarm(false);
  actualState->setTrack(false);
  actualState->setHasDetected(false);
  actualState->setIsModaliyChanged(true);
  if(actualState->getCloveNumber() == 0 || actualState->getCloveNumber() == 15){
    actualState->setMotorDirection(actualState->getCloveNumber() == 0 ? RIGHT : LEFT);
  }
}

void ManageButtonsTask::initModality(){
  actualState->setCloveNumber(INITIAL_MOTOR_POSITION);
  initModalitySoft();
}
