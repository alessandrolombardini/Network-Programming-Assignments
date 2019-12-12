/* Project Group: Baiardi Martina, 
                  Giachin Marco, 
                  Lombardini Alessandro */

#include "Scheduler.h"
#include "State.h"
#include "ManageAlertLedTask.h"
#include "ManageTrackLedTask.h"
#include "ManageSonarReadTask.h"
#include "ManageMotorTask.h"
#include "ManagePotentiometerTask.h"
#include "ManagePowerDownTask.h"
#include "ManageButtonsTask.h"
#include "SerialReceiveTask.h"
#include "EvaluateMessagesTask.h"

#define TICK_TIME 75                   
#define PIR_TASK_PERIOD 75
#define BUTTONS_TASK_PERIOD 75
#define POTENTIOMETER_TASK_PERIOD 75
#define SONAR_TASK_PERIOD 75
#define MOTOR_TASK_PERIOD 75
#define LED_ALERT_TASK_PERIOD 75
#define LED_TRACK_TASK_PERIOD 75
#define SERIAL_COMMUNICATION_TASK 75
#define EVALUATE_SERIAL_MESSAGE 75

#define LED_ALERT_PIN 8
#define LED_TRACK_PIN 9
#define SONAR_TRIG_PIN 3
#define SONAR_ECHO_PIN 4
#define MOTOR_PIN 13
#define POTENTIOMETER_PIN A5
#define PIR_PIN 2
#define BUTTON_SINGLE_PIN 5
#define BUTTON_MANUAL_PIN 6
#define BUTTON_AUTO_PIN 7

Scheduler * scheduler;    /* Scheduler of tasks */
State * state;            /* State of the application: it's shared by all tasks*/

void setup() {
  Serial.begin(9600);
  state = new State();
  scheduler = new Scheduler();
  scheduler->init(TICK_TIME);

  /* Task to manage the potentiometer */
  Task* task4 = new ManagePotentiometerTask(POTENTIOMETER_PIN);
  task4->init(POTENTIOMETER_TASK_PERIOD, state);
  scheduler->addTask(task4);

  /* Task to manage the sonar */
  Task* task0 = new ManageSonarReadTask(SONAR_TRIG_PIN, SONAR_ECHO_PIN);
  task0->init(SONAR_TASK_PERIOD, state);
  scheduler->addTask(task0);

  /* Task to manage the alert led */
  Task* task1 = new ManageAlertLedTask(LED_ALERT_PIN);
  task1->init(LED_ALERT_TASK_PERIOD, state);
  scheduler->addTask(task1);

  /* Task to manage the track led */
  Task* task2 = new ManageTrackLedTask(LED_TRACK_PIN);
  task2->init(LED_TRACK_TASK_PERIOD, state);
  scheduler->addTask(task2);

  /* Task to manage the motor */
  Task* task3 = new ManageMotorTask(MOTOR_PIN);
  task3->init(MOTOR_TASK_PERIOD, state);
  scheduler->addTask(task3); 

  /* Task to manage buttons */
  Task* task6 = new ManageButtonsTask(BUTTON_SINGLE_PIN,BUTTON_MANUAL_PIN,BUTTON_AUTO_PIN);
  task6->init(BUTTONS_TASK_PERIOD, state);
  scheduler->addTask(task6);

  /* Serial Task */
  Task* task7 = new SerialReceiveTask();
  task7->init(SERIAL_COMMUNICATION_TASK, state);
  scheduler->addTask(task7); 

  /* Task to evalueate messages */
  Task* task8 = new EvaluateMessagesTask();
  task8->init(EVALUATE_SERIAL_MESSAGE, state);
  scheduler->addTask(task8);

  /* Task to manage the pir */
  Task* task5 = new ManagePowerDownTask(PIR_PIN, BUTTON_AUTO_PIN, BUTTON_MANUAL_PIN);
  task5->init(PIR_TASK_PERIOD, state);
  scheduler->addTask(task5); 
}

void loop() {
  scheduler->schedule();
}
