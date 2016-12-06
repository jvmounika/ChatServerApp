//
//  ChatClient.h
//  ChatServerClient
//

#ifndef __ChatServerClient__ChatClient__
#define __ChatServerClient__ChatClient__

#include <iostream>
#include <thread>
#include <mutex> 

#include "commonCode.h"

class ChatClient: public commonCode
{
public:
    ChatClient(std::string ipaddress, int portNumber);
    ~ChatClient();
    void registerMessageReceivedListener(std::function<void(std::string*)> msgReceivedCallback);
    void sendMessage(std::string message);
    void receiveMessage();
    int authenticate_session_client();
    static std::mutex mtx;
private:
    int socketfd;
    std::thread readThread;
    const int MAX_MSG_SIZE = 1024;
    void readThreadWorker();
    std::function<void(std::string*)> msgReceivedCallback;
    int getMenuInputFromUser();
    void processMessageClient(std::string input);
};

#endif /* defined(__ChatServerClient__ChatClient__) */
