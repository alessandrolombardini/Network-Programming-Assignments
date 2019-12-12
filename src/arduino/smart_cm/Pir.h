#ifndef __PIR__
#define __PIR__

class Pir { 
  public:
    Pir(int pin);
    bool detect();    
    
  private:
    void calibrate();
    int pin;  
};

#endif