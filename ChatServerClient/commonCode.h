//
//  commonCode.h
//  ChatServer
//

#ifndef __ChatServer__commonCode__
#define __ChatServer__commonCode__

#include <iostream>

#define MAX_USERS 10
#define MAX_USER_NAME_LEN 10

// Handles to create authentication between Server and client
#define TCP_SEND_SERVER_GREETING "HELO"
#define TCP_SEND_CLIENT_GREETING "HAII"
#define TCP_ACCEPT_CONNECTION "ACPT"
#define TCP_DENY_CONNECTION "DENY"
#define TCP_RESEND_PWD "RPWD"

#define MAX_BUFFER_SIZE 1024

/*User Menu*/
#define GET_USERS '1'
#define GET_TOPICS '2'
#define SUBSCRIBE_TO_A_TOPIC '3'
#define PUBLISH_TO_A_TOPIC '4'
#define GET_NEW_POSTS '5'
#define GET_RECENT_POSTS '6'
#define CLOSE_SESSION '7'

class commonCode
{
public:
    int bytesReadSoFar;
    char messageBuffer[MAX_BUFFER_SIZE];
    // Mutex to avoid overlapped messages because of multiple threads competing for the std outstream
    static std::mutex mtx;
    void printMessageAndNumber(std::string message, int number);
    // Static func can be called without an instance
    static void printMessage(std::string message);
    void printMessageBuffer();
};

#endif /* defined(__ChatServer__commonCode__) */
