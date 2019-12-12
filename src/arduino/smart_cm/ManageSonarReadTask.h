#ifndef __MANAGESONARREADTASK__
#define __MANAGESONARREADTASK__

#include "Task.h"
#include "Sonar.h"
#include "MsgService.h"

#define DISTANCE_NEAR 0.2 /* Metri */
#define DISTANCE_FAR 0.4  /* Metri */

class ManageSonarReadTask: public Task {
  private:
    Sonar * sonar;
    int trigPin;
    int echoPin;
    bool somethingFounded;              /* TRUE: something has been founded while scanning */
    bool waitingLapEnd;                 /* TRUE: an object has been founded and you have to finish the actual lap */
    bool recheckObject;                 /* TRUE: an object has been founded and you have to check if it's here yet.
                                                 Doing another lap. */
    bool objectRefounded;               /* While rescanning something has been founded */
    int timeElapsedSinceLastScan;       /* Time elapsed since last scan */

    float doScan();                                       /* Do a scan */
    void evaluateDistanceAutoMode(float value);           /* Get the distance obteined and do what have to do with that distance in auto mode */
    void evaluateDistanceSingleManualMode(float value);   /* Get the distance obteined and do what have to do with that distance in single and manual mode */
    void printAngle();                                    /* Print actual angle obteined by clove number */
    void resetScan();                                     /* Reset status of scan */
    bool checkIfHasToScan();                              /* Check if the time request to do a scan is elapsed (it's calculated by velocity) */
    
  public:
    ManageSonarReadTask(int trigPin, int echoPin);
    void init(int period, State * state);
    void tick();
};
#endif
