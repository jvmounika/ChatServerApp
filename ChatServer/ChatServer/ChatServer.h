//
//  ChatServer.h
//  ChatServer
//

#ifndef __ChatServer__ChatServer__
#define __ChatServer__ChatServer__

#include <iostream>
#include <thread>
#include "Client.h"

class ChatServer {
public:
    ChatServer();
    ~ChatServer();
private:
    std::thread msgProcessingThread;
    std::thread newConnectionThread;
    void privateMethod();
    ChatClient* client;
    void msgCallback(std::string* message, ChatClient *pChatClient);
};

#endif /* defined(__ChatServer__ChatServer__) */
