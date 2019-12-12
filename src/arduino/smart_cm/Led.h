#ifndef __LED__
#define __LED__

class Led { 
  int pin;
   
  public:
    Led(int pin);
    void switchOn();
    void switchOff();     
};

#endif
