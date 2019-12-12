#include "SerialReceiveTask.h"

SerialReceiveTask::SerialReceiveTask(){
    MsgService.init();
}

void SerialReceiveTask::init(int period, State * state){
  Task::init(period, state);
}

void SerialReceiveTask::tick(){
  if (MsgService.isMsgAvailable()) {
    Msg* msg = MsgService.receiveMsg();    
    String content = msg->getContent();
    actualState->addMessage(content);
    delete msg;
  }
}
