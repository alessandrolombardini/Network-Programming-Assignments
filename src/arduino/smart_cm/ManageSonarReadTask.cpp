#include "ManageSonarReadTask.h"

ManageSonarReadTask::ManageSonarReadTask(int trigPin, int echoPin){
  this->trigPin = trigPin;
  this->echoPin = echoPin;
}

void ManageSonarReadTask::init(int period, State * state){
  Task::init(period, state);
  sonar = new Sonar(trigPin, echoPin);
  resetScan();
}

void ManageSonarReadTask::resetScan(){
  somethingFounded = false;
  waitingLapEnd = false;     
  recheckObject = false;
  objectRefounded = false;
  timeElapsedSinceLastScan = 0;
}

void ManageSonarReadTask::tick(){
  /* Is exit by power down mode */
  if(actualState->isExitByPowerMode()){
    timeElapsedSinceLastScan = 0;
  }
  /* Modality is changed: reset scan */
  if(actualState->isModalityChanged()){
    resetScan();
  } else {
    if(actualState->getModality() == AUTO && checkIfHasToScan()){
      /* Do scan */
      float value = doScan();
      evaluateDistanceAutoMode(value);
      MsgService.sendComposedMessage();
    } else if(actualState->getModality() == SINGLE && actualState->hasDetected() && checkIfHasToScan()) {
      /* Do scan */
      float value = doScan();
      evaluateDistanceSingleManualMode(value);
      MsgService.sendComposedMessage();
    } else if (actualState->getModality() == MANUAL && checkIfHasToScan()){
      /* Do scan */
      float value = doScan();
      evaluateDistanceSingleManualMode(value);
      MsgService.sendComposedMessage();
    }
  }
}

float ManageSonarReadTask::doScan(){
   /* Do scan */
  float value = sonar->getDistance();
  printAngle();
  MsgService.composeMessage("Distance: ");
  if( value > DISTANCE_FAR || value == (float)RETURN_OUT_OF_TIME){
    MsgService.composeMessage("out of reach. ");
  }else{
    MsgService.composeMessage((String) value);
    MsgService.composeMessage(" m. ");
  }
  return value;
}

void ManageSonarReadTask::evaluateDistanceAutoMode(float value){
  if(value < DISTANCE_NEAR){
    actualState->setTrack(true);
  } else if(value >= DISTANCE_NEAR && value <= DISTANCE_FAR){
    actualState->setTrack(false);
    somethingFounded = true;
  } else if (value > DISTANCE_FAR) {
    actualState->setTrack(false);
  }
  
  /* Check if something has been founded while nothing has been founded yet */
  if(somethingFounded && !waitingLapEnd && !recheckObject){
    actualState->setAlarm(true);
    waitingLapEnd = true;
  }
  /* Something has been founded during a lap: check if that lap is over.
    If it's over do another one to do another lap to check if there is something yet */
  if(waitingLapEnd){
    if(actualState->getCloveNumber() == 0 || actualState->getCloveNumber() == CLOVE_NUMBER - 1){
      waitingLapEnd = false;
      recheckObject = true;
    }
  } else if(recheckObject){ /* First lap is over: now check if during this new lap there is something */
    /* Founded something: memorize thats something has been founded during the 'check lap' */
    if(somethingFounded){
      objectRefounded = true;
    }
    /* If the 'check lap' is over, check if something has been founded.
      If true, do another 'check lap'.
      If false, back to normal situation. */
    if(actualState->getCloveNumber() == 0 || actualState->getCloveNumber() == CLOVE_NUMBER - 1){
      if(objectRefounded == false){
        recheckObject = false;
        actualState->setAlarm(false);
      } 
      objectRefounded = false;
    }
  }
  if (actualState->isAlarmed() && actualState->isTracked()){
    MsgService.composeMessage("[ALARM] [TRACKING]");
  } else if (actualState->isAlarmed()){
    MsgService.composeMessage("[ALARM]");
  } else if (actualState->isTracked()){
    MsgService.composeMessage("[TRACKING]");
  }
  somethingFounded = false;
}

void ManageSonarReadTask::evaluateDistanceSingleManualMode(float value){
  if(value < DISTANCE_NEAR){
    actualState->setTrack(true);
    actualState->setAlarm(false);
    MsgService.composeMessage("[TRACKING] [OBJECT FOUNDED]");
  } else if(value >= DISTANCE_NEAR && value <= DISTANCE_FAR){
    actualState->setTrack(false);
    actualState->setAlarm(true);
    MsgService.composeMessage("[OBJECT FOUNDED]");
  } else if (value > DISTANCE_FAR || value == (float)RETURN_OUT_OF_TIME) {
    actualState->setTrack(false);
    actualState->setAlarm(false);
  }
}

void ManageSonarReadTask::printAngle(){
  int angle = 180/15 * actualState->getCloveNumber();
  MsgService.composeMessage("(Angle ");
  MsgService.composeMessage((String) angle);
  MsgService.composeMessage(" degrees) ");
}

bool ManageSonarReadTask::checkIfHasToScan(){
  int timeToWait = BASE_WAIT - (actualState->getVelocity() - 1) * VELOCITY_MULTIPLIER;
  timeElapsedSinceLastScan += getMyPeriod();
  if (timeElapsedSinceLastScan >= timeToWait){
    timeElapsedSinceLastScan = 0;
    return true;
  } else {
    return false;
  }
}
