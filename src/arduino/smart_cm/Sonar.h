#ifndef __SONAR__
#define __SONAR__

#define TIME_OUT_TIME 5000                  /* Timeout in microseconds */
#define RETURN_OUT_OF_TIME 10               /* Value that has to be returned if pulseInLong catch an interrupt */

class Sonar { 
  private:
    const float vs = 331.45 + 0.62*20;      /* Velocità del suono in un ambiente a 20 °C */
    int trigPin;
    int echoPin; 
    
  public:
    Sonar(int trigPin, int echoPin);
    float getDistance();     
};

#endif
