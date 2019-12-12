#ifndef __MSGSERVICE__
#define __MSGSERVICE__

#include "Arduino.h"

class Msg {
  String content;
  public:
    Msg(String content){
      this->content = content;
    }
    String getContent(){
      return content;
    }
};

class Pattern {
  public:
    virtual boolean match(const Msg& m) = 0;  
};

class MsgServiceClass {
  private:
    String composedMsg;
  public:   
    Msg* currentMsg;
    bool msgAvailable;
    void init();  
    bool isMsgAvailable();
    Msg* receiveMsg();
    /* note: message deallocation is responsibility of the client */
    void sendMsg(const String& msg);
    void sendComposedMessage();
    void composeMessage(const String& msg);
};

extern MsgServiceClass MsgService;

#endif
