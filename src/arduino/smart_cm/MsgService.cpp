#include "Arduino.h"
#include "MsgService.h"

String content;

MsgServiceClass MsgService;

bool MsgServiceClass::isMsgAvailable(){
  return msgAvailable;
}

Msg* MsgServiceClass::receiveMsg(){
  if (msgAvailable){
    Msg* msg = currentMsg;
    msgAvailable = false;
    currentMsg = NULL;
    content = "";
    return msg;  
  } else {
    return NULL; 
  }
}

void MsgServiceClass::init(){
  Serial.begin(9600);
  content.reserve(256);
  content = "";
  composedMsg = "";
  currentMsg = NULL;
  msgAvailable = false;  
}

void MsgServiceClass::sendMsg(const String& msg){
  Serial.println(msg);  
  Serial.flush();
}

void MsgServiceClass::sendComposedMessage(){
  if(composedMsg != ""){
    Serial.println(composedMsg);
    composedMsg = "";
  }
}

void MsgServiceClass::composeMessage(const String& msg){
  composedMsg += msg;
}

void serialEvent() {
  /* reading the content */
  while (Serial.available()) {
    char ch = (char) Serial.read();
    if (ch == '\n'){
      MsgService.currentMsg = new Msg(content);
      MsgService.msgAvailable = true;      
    } else {
      content += ch;      
    }
  }
}
  
