#include "EvaluateMessagesTask.h"

EvaluateMessagesTask::EvaluateMessagesTask(){ }

void EvaluateMessagesTask::init(int period, State * state){
  Task::init(period, state);
}

void EvaluateMessagesTask::tick(){
  evaluateMessage();
}

void EvaluateMessagesTask::evaluateMessage(){
  if(actualState->hasMessage()){
    String content = actualState->getMessage();
    bool patternCheck = true;
    if (content == "Servo: increase position"){
        if (actualState->getModality() == MANUAL && !actualState->isTracked()){
            actualState->setCloveNumber(actualState->getCloveNumber() < 15 ? actualState->getCloveNumber() +1 : actualState->getCloveNumber());
        }
    } else if(content == "Servo: decrease position"){
        if (actualState->getModality() == MANUAL && !actualState->isTracked()){
            actualState->setCloveNumber(actualState->getCloveNumber() > 0 ? actualState->getCloveNumber() -1 : actualState->getCloveNumber());
        }
    } else if(content == "Modality selected: AUTO" ){
        if(actualState->getModality() != AUTO){
            actualState->setModality(AUTO);
            initModalitySoft();
        }
    } else if(content == "Modality selected: MANUAL"){
        if(actualState->getModality() != MANUAL){
            actualState->setModality(MANUAL);
            initModalitySoft();
        }
    } else if(content == "Modality selected: SINGLE"){
        if(actualState->getModality() != SINGLE){
            actualState->setModality(SINGLE);
            initModality();
        }
    } else if(content == "Speed: 1"){
        actualState->setVelocity(1);
        actualState->setIfVelocityIsSetByConsole(true);
    } else if(content == "Speed: 2"){
        actualState->setVelocity(2);
        actualState->setIfVelocityIsSetByConsole(true);
    } else if(content == "Speed: 3"){
        actualState->setVelocity(3);
        actualState->setIfVelocityIsSetByConsole(true);
    } else if(content == "Speed: 4"){
        actualState->setVelocity(4);
        actualState->setIfVelocityIsSetByConsole(true);
    } else if(content == "Speed: 5"){
        actualState->setVelocity(5);
        actualState->setIfVelocityIsSetByConsole(true);
    } else if(content == "Speed: 6"){
        actualState->setVelocity(6);
        actualState->setIfVelocityIsSetByConsole(true);
    } else if(content == "Speed: 7"){
        actualState->setVelocity(7);
        actualState->setIfVelocityIsSetByConsole(true);
    } else if(content == "Speed: 8"){
        actualState->setVelocity(8);
        actualState->setIfVelocityIsSetByConsole(true);
    } else {
        patternCheck = false;
    }
    actualState->removeMessage();
    if(patternCheck){
        MsgService.sendMsg("[OK]");
    }
  }
}


void EvaluateMessagesTask::initModalitySoft(){
  actualState->setAlarm(false);
  actualState->setTrack(false);
  actualState->setHasDetected(false);
  actualState->setIsModaliyChanged(true);
  if(actualState->getCloveNumber() == 0 || actualState->getCloveNumber() == 15){
    actualState->setMotorDirection(actualState->getCloveNumber() == 0 ? RIGHT : LEFT);
  }
}

void EvaluateMessagesTask::initModality(){
  actualState->setCloveNumber(INITIAL_MOTOR_POSITION);
  initModalitySoft();
}

