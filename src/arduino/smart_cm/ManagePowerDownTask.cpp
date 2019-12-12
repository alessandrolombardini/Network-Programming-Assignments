#include "ManagePowerDownTask.h"
#include <EnableInterrupt.h>
#include <avr/sleep.h>

ManagePowerDownTask::ManagePowerDownTask(int pinPir, int pinAutoButton, int pinManualButton){
  this->pinPir = pinPir;
  this->pinAutoButton = pinAutoButton;
  this->pinManualButton = pinManualButton;
}

void wakeUp(){ }

void ManagePowerDownTask::init(int period, State * state){
  Task::init(period, state);
  pir = new Pir(pinPir); 
  pinMode(pinPir,INPUT);
  pinMode(pinAutoButton,INPUT);
  pinMode(pinManualButton,INPUT);
}

void ManagePowerDownTask::tick(){
  /* If the sistem is in single mode and it doesn't do a scan, go in power down mode */
  if(actualState->getModality()==SINGLE && !actualState->hasDetected()){
    enableInterrupt(pinPir, wakeUp, RISING);              /* Pir interrupt */
    enableInterrupt(pinAutoButton, wakeUp, CHANGE);       /* Auto button interrupt */
    enableInterrupt(pinManualButton, wakeUp, CHANGE);     /* Manual button interrupt */
    enableInterrupt(SERIAL_PIN, wakeUp, CHANGE);          /* RX interrupt */
    MsgService.sendMsg("Going in power down mode...");
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);  
    sleep_enable();
    sleep_mode();  
    /* First thing to do when waked up is disable sleep. */  
    sleep_disable(); 
    actualState->setHasDetected(true);
    actualState->setIfExitByPowerMode(true);
    MsgService.sendMsg("Wake up!");
    /* Detach all interrupts */
    disableInterrupt(pinPir);
    disableInterrupt(pinAutoButton);
    disableInterrupt(pinManualButton);
    disableInterrupt(SERIAL_PIN);
  } 
}
