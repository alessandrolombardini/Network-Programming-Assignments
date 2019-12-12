#include "Scheduler.h"
#include "Arduino.h"
#include <avr/sleep.h>
#include <avr/power.h>

Scheduler::Scheduler(){}

void Scheduler::init(int basePeriod){
  this->basePeriod = basePeriod;
  timer = new Timer();
  timer->setupPeriod(basePeriod);  
  nTasks = 0; 
}

bool Scheduler::addTask(Task* task){
  if (nTasks < MAX_TASKS-1){
    taskList[nTasks] = task;
    nTasks++;
    return true;
  } else {
    return false; 
  }
}

void Scheduler::schedule(){
  timer->waitForNextTick();
  for (int i = 0; i < nTasks; i++){
    if (taskList[i]->updateAndCheckTime(basePeriod)){
      taskList[i]->tick();
    }    
  } 
}
