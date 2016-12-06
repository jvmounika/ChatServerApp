//
//  main.cpp
//  ChatServerClient
//

#include <iostream>
#include <unistd.h>

#include "commonCode.h"
#include "ChatClient.h"

void messageReceived(std::string* message) {
//    std::cout << "The message received is:" << *message << std::endl;
    commonCode::printMessage("Recd msg : " + *message);
    delete message;
}

int main(int argc, const char * argv[]) {
    // insert code here...
    ChatClient chatClient("localhost",1234);
    //chatClient.registerMessageReceivedListener(messageReceived);
    sleep(1);
    //chatClient.sendMessage("Hello");
    //std::cout << " Back to main\n";
    return 0;
}
