#ifndef __STATE__
#define __STATE__

#include "Global.h"
#include "Arduino.h"

#define SIZE_OF_BUFFER 20
#define NONE_MESSAGE -1

class State {
  private:
    bool alarm;                             /* TRUE: alarm is on */
    bool track;                             /* TRUE: tracking is on */
    bool detect;                            /* TRUE: an object has been detected */
    int cloveNumber;                        /* Clove towards which the sensor is directed */
    int motorDirection;                     /* Direction of motor motion */
    int velocity;                           /* Velocity of the motion of the motor */
    bool modalityChanged;                   /* TRUE: modality has changed */
    bool velocityByConsole;                 /* TRUE: velocity set by console */
    Modality modality;                      /* Actual modality */
    String messageBuffer[SIZE_OF_BUFFER];   /* Messages buffer */
    int lastMessageIndex;                   /* Index of last message in messageBuffer */
    bool powerMode;                         /* TRUE: it just exit by power mode */

  public:
    State();
    Modality getModality();
    void setModality(Modality modality);
    bool isAlarmed();
    void setAlarm(bool isAlarmed);
    bool isTracked();
    void setTrack(bool isTracked);
    int getCloveNumber();
    void setCloveNumber(int nClove);
    int getMotorDirection();
    void setMotorDirection(int Direction);
    int getVelocity();
    void setVelocity(int velocity);
    bool hasDetected();
    void setHasDetected(bool hasDetected);
    bool isModalityChanged();
    void setIsModaliyChanged(bool isChanged);
    bool isVelocitySetByConsole();
    void setIfVelocityIsSetByConsole(bool isSetByConsole);
    bool addMessage(String message);
    bool hasMessage();
    String getMessage();
    void removeMessage();
    bool isExitByPowerMode();
    void setIfExitByPowerMode(bool isExitByPowerMode);
};

#endif
