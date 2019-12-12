#include "State.h"

State::State(){
  this->alarm = false;
  this->track = false;
  this->cloveNumber = INITIAL_MOTOR_POSITION;      
  this->motorDirection = RIGHT;                        
  this->velocity = 0;
  this->detect = false;
  this->modalityChanged = false;
  this->modality = MANUAL;
  this->lastMessageIndex = NONE_MESSAGE;
}

void State::setAlarm(bool isAlarmed){
  this->alarm = isAlarmed;
}

bool State::isAlarmed(){
  return this->alarm;
}

void State::setTrack(bool isTracked){
  this->track = isTracked;
}

bool State::isTracked(){
  return this->track;
}

int State::getCloveNumber(){
  return this->cloveNumber;
}

void State::setCloveNumber(int nClove){
  if(nClove >= 0 || nClove<= 15){
      this->cloveNumber = nClove;
  }
}

int State::getMotorDirection(){
  return this->motorDirection;
}

void State::setMotorDirection(int direction){
  this->motorDirection = direction;
}

int State::getVelocity(){
  return this->velocity;
}

void State::setVelocity(int velocity){
  this->velocity = velocity;
}

bool State::hasDetected(){
  return this->detect;
}

void State::setHasDetected(bool hasDetected){
  this->detect = hasDetected;
}

void State::setModality(Modality modality){
  this->modality = modality;
}

Modality State::getModality(){
  return this->modality;
}

bool State::isModalityChanged(){
  return this->modalityChanged;
}

void State::setIsModaliyChanged(bool isChanged){
  this->modalityChanged = isChanged;
}

bool State::isVelocitySetByConsole(){
  return this->velocityByConsole;
}

void State::setIfVelocityIsSetByConsole(bool isSetByConsole){
  this->velocityByConsole = isSetByConsole;
}


bool State::isExitByPowerMode(){
  return this->powerMode;
} 

void State::setIfExitByPowerMode(bool isExitByPowerMode){
  this->powerMode = isExitByPowerMode;
}

bool State::addMessage(String message){
  if(this->lastMessageIndex < SIZE_OF_BUFFER - 1){
    this->lastMessageIndex=this->lastMessageIndex+1;
    this->messageBuffer[this->lastMessageIndex] = message;
    return true;
  }
  return false;
}

bool State::hasMessage(){
  return this->lastMessageIndex >= 0;
}

String State::getMessage(){
  if(hasMessage()){
    return this->messageBuffer[0];
  }
  return "";
}

void State::removeMessage(){
  if(this->lastMessageIndex >= 1){
    for(int i = 0; i < this->lastMessageIndex; i++){
      this->messageBuffer[i] = this->messageBuffer[i+1];
    }
    this->lastMessageIndex=this->lastMessageIndex-1;
  } else if(this->lastMessageIndex == 0){
    this->lastMessageIndex = NONE_MESSAGE;
  }
}
