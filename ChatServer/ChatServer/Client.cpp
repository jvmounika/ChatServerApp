//
//  Client.cpp
//  ChatServer
//

#include "Client.h"
#include <unistd.h>
#include<stdlib.h>
#include<stdio.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<string.h>
#include <arpa/inet.h>
#include <netdb.h>

// @todo use strings instead of char array if time permits
// Syntax: {username, password, validBit, usernameSize, passwordSize}
// Upon 3 unsuccessful attempts validBit will be reset to 0
char login_credentials[MAX_USERS][5][MAX_USER_NAME_LEN] = {{ "user1", "pwd1", "1", "5", "4"},
    { "user2", "pwd2", "1", "5", "4"},
    { "user3", "pwd3", "1", "5", "4"},
    { "user4", "pwd4", "1", "5", "4"},
    { "user5", "pwd5", "1", "5", "4"},
    { "user6", "pwd6", "1", "5", "4"},
    { "user7", "pwd7", "1", "5", "4"},
    { "user8", "pwd8", "1", "5", "4"},
    { "user9", "pwd9", "1", "5", "4"},
    { "user0", "pwd0", "1", "5", "4"}};

// @todo make this a static member of the class and provide accessor methods
#define MAX_TOPICS 50
std::string current_active_topics[MAX_TOPICS] = {"Current economy",
                                                 "Sports",
                                                 "Job opportunities"};

std::vector<Post> ChatClient::postBuffer;
std::mutex ChatClient::postAccessMtx;

ChatClient::ChatClient(int sockfd)
{
    std::cout << "ChatClient Constructor" <<  std::endl;
    this->newsockfd = sockfd;
    this->postCountSeen = 0;
    readThread = std::thread(&ChatClient::readThreadWorker, this);
}

int ChatClient::authenticate_session_server()
{
    //1. receive greeting from client
    receiveMessage();
    
    //2. Send greeting to client
    sendMessage(TCP_SEND_SERVER_GREETING);
    
    // 3. Recv Username & authenticate
    receiveMessage();
    bool bFoundUser = false;
    int foundLoginID = -1;
    if (bytesReadSoFar != 0)
    {
        // verify if the username exists
        for (int i = 0; i < MAX_USERS; i++)
        {
            // We support only usernames upto 10 characters.
            // we subtract the ASCII value of the username size with ASCII value of
            // '0' to extract the size
            if ((login_credentials[i][2][0] == '1') &&
                ((login_credentials[i][3][0] - '0') == (bytesReadSoFar - 2)) &&
                (strncmp(messageBuffer+2, login_credentials[i][0], bytesReadSoFar-2) == 0))
            {
                // Matching username
                std::cout << "Found user\n";
                bFoundUser = true;
                foundLoginID = i;
                break;
            }
        }
    }
    // 4. Acpt/Deny
    if (bFoundUser)
    {
        sendMessage(TCP_ACCEPT_CONNECTION);
    }
    else
    {
        sendMessage(TCP_DENY_CONNECTION);
        return -1;
    }
    
    // 5. Recv password & authenticate
    for (int i = 0; i < 3; i++)
    {
        bFoundUser = false;
        receiveMessage();
        if (bytesReadSoFar != 0)
        {
            // verify if the password is correct
            if ((strncmp(messageBuffer+2, login_credentials[foundLoginID][1], bytesReadSoFar-2) == 0) &&
                ((login_credentials[i][4][0] - '0') == (bytesReadSoFar - 2)))
            {
                // Matching password
                printMessage("Password matched");
                bFoundUser = true;
                break;
            }
            else
            {
                printMessage("Password incorrect.. Please try again");
                if (i < 2)
                {
                    sendMessage(TCP_RESEND_PWD);
                }
                else
                {
                    // Mark user login Id as invalid after 3 unsuccessful attempts
                    login_credentials[foundLoginID][2][0] = '0';
                }
            }
        }
    }
    
    // 6. Acpt/Deny
    if (bFoundUser)
    {
        sendMessage(TCP_ACCEPT_CONNECTION);
    }
    else
    {
        sendMessage(TCP_DENY_CONNECTION);
        return -1;
    }
    
    printMessage("Session established after authentication");
    return 0;
}

void ChatClient::addPost(int topicIndex, std::string postText)
{
    postAccessMtx.lock();
    postBuffer.push_back(Post(topicIndex, postText));
    postAccessMtx.unlock();
}

#define GET_USERS '1'
#define GET_TOPICS '2'
#define SUBSCRIBE_TO_A_TOPIC '3'
#define PUBLISH_TO_A_TOPIC '4'
#define GET_NEW_POSTS '5'
#define GET_RECENT_POSTS '6'
#define CLOSE_SESSION '7'

void ChatClient::processInputFromClient()
{
    char input = messageBuffer[2];
    std::string messageToBeSent;
    if (input == GET_USERS)
    {
        // Append all users to a string , seperated by ","
        for (int i = 0 ; i < MAX_USERS; i++)
        {
            messageToBeSent += login_credentials[i][0];
            messageToBeSent.append(",");
        }
    }
    else if (input == GET_TOPICS)
    {
        for (int i = 0 ; i < MAX_TOPICS; i++)
        {
            if (current_active_topics[i].empty())
            {
                break;
            }
            messageToBeSent += std::to_string(i);
            messageToBeSent += ". " + current_active_topics[i] + ",";
        }
    }
    else if (input == SUBSCRIBE_TO_A_TOPIC)
    {
        // Message Format: Size[2bytes] input "," topicNumber
        // For now assume topic is always a single digit
        // @todo only add when it is a valid topic number and not already part of the list
        int topicIndex = messageBuffer[4] - '0';
        if (subscribedTopics[topicIndex])
        {
            messageToBeSent = "Already subscribed to this topic. Choose again";
        }
        else
        {
            subscribedTopics[topicIndex] = true;
            messageToBeSent = "Successfully subscribed to the new topic";
        }
    }
    else if (input == PUBLISH_TO_A_TOPIC)
    {
        int topicIndex = messageBuffer[4] - '0';
        std::string text;
        for (int i = 6; i < bytesReadSoFar; i++)
        {
            text.push_back(messageBuffer[i]);
        }
        addPost(topicIndex, text);
        messageToBeSent += "Successfully published the post";
    }
    else if (input == GET_NEW_POSTS)
    {
        postAccessMtx.lock();
        int tempIndex = 0;
        for (int postIndex = postCountSeen; postIndex < postBuffer.size(); postIndex++, postCountSeen++)
        {
            tempIndex = postBuffer[postIndex].postTopicType;
            if (!(subscribedTopics[tempIndex]))
            {
                continue;
            }
            messageToBeSent += postBuffer[postIndex].text;
            messageToBeSent += ",";
        }
        postAccessMtx.unlock();
    }
    else if (input == GET_RECENT_POSTS)
    {
        postAccessMtx.lock();
        int tempIndex = 0;
        int numPostsGathered = 0;
        // Use reverse iterator to gather the most recent posts
        for (std::vector<Post>::reverse_iterator it = postBuffer.rbegin(); it != postBuffer.rend(); it++)
        {
            tempIndex = it->postTopicType;
            if (!(subscribedTopics[tempIndex]))
            {
                continue;
            }
            messageToBeSent += it->text;
            messageToBeSent += ",";
            numPostsGathered++;
            // Return only the last 10 posts
            if (numPostsGathered == 10)
            {
                break;
            }
        }
        postAccessMtx.unlock();
    }
    sendMessage(messageToBeSent);
}

static const int MSG_START = 1;
static const int MSG_DATA = 2;

void ChatClient::readThreadWorker()
{
    // Keep looping until the client authenticates itself
    //while (authenticate_session_server());
    
    printMessage("Connection established");

    while(1)
    {
        // Get user input from client
        receiveMessage();
        printMessage("New request from client:");
        printMessageBuffer();
    
        if (messageBuffer[2] == CLOSE_SESSION)
        {
            printMessage("closing session");
            break;
        }
        // Always assume we get a right valid input from client since the way it is written
        // Process the input accordingly and send a message to client
        processInputFromClient();
        //sendMessage("Processing your input please wait");
        
    }
    /*
    char buffer[MAX_MSG_SIZE];
    int bytesRead;
    int state  = MSG_START;
    memset(buffer,0,sizeof(buffer));
    int bufferOffset = 0;
    unsigned short messageLength = 0;
    std::string *message;
    
    while(1) {
        printMessageAndNumber("Waiting for data from client ", newsockfd);
        receiveMessage();
//        bytesRead = read(newsockfd,buffer + bufferOffset,MAX_MSG_SIZE - bufferOffset);
        printMessageAndNumber("BytesRead: ", bytesRead);
        if(bytesRead <= 0) {
            std::cout << "Error Reading from socket" << std::endl;
            close(newsockfd);
            return;
        }
        printMessageBuffer();
        
        switch (state) {
            case MSG_START:
                if(bytesRead >= 2) {
                    messageLength = (buffer[0]&0xFF) + ((buffer[1] << 8)&0xFF00);
                } else {
                    printMessage("Error in parsing");
                }
                printMessageAndNumber("MessageLength: ", messageLength);
                if(messageLength == bytesRead-2) {
                    printMessage("Complete Message is received");
                    message = new std::string(buffer+2);
                    //msgReceivedCallback(message, this);
                    bufferOffset = 0;
                    memset(buffer,0,sizeof(buffer));
                } else {
                    bufferOffset += bytesRead;
                    state = MSG_DATA;
                }
                break;
            case MSG_DATA:
                if(bytesRead + bufferOffset == messageLength-2) {
                    printMessage("Complete Message Is received");
                    message = new std::string(buffer+2);
                    printMessage("Complete Message Is received:");
                    printMessage(*message);
                    bufferOffset = 0;
                    memset(buffer,0,sizeof(buffer));
                    state = MSG_START;
                } else {
                    bufferOffset += bytesRead;
                }
                break;
        }
    }*/
}

void ChatClient::sendMessage(std::string message) {
    unsigned short length = message.size();
    char* buffer = new char[length+2];
    buffer[0] = length & 0xFF;
    buffer[1] = (length >> 8)&(0xFF);
    message.copy(buffer+2,length);
    //printMessageAndNumber("Sending a message of length:", length);
    //printMessage(message);
    write(newsockfd,buffer,length+2);
}

void ChatClient::receiveMessage()
{
    bytesReadSoFar = read(newsockfd, messageBuffer, MAX_MSG_SIZE);
    //printMessageAndNumber("Message received in bytes:", bytesReadSoFar);
    //printMessageBuffer();
}

void ChatClient::registerMessageReceivedCallback(std::function<void(std::string*, ChatClient *pChatClient)> callback)
{
    msgReceivedCallback = callback;
}


