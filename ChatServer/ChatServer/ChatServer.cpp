//
//  ChatServer.cpp
//  ChatServer
//

#include "ChatServer.h"
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

ChatServer::ChatServer()
{
    std::cout << "Chatserver Constructor" <<  std::endl;
    msgProcessingThread = std::thread(&ChatServer::privateMethod, this);
}

void ChatServer::msgCallback(std::string* message, ChatClient *pChatClient)
{
    //std::cout << "Message from client:" << *message << std::endl;
    //delete message;
    //pChatClient->sendMessage("Message From Server This is");
    std::cout << " Back to msgCallback\n";
}

void ChatServer::privateMethod()
{
    int sockfd, newsockfd, portno;
    socklen_t clilen;
    
    struct sockaddr_in serv_addr;
    struct sockaddr cli_addr;
    
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
    if (sockfd < 0)
    {
        std::cout<< "ERROR opening socket";
    }
    
    bzero((char *) &serv_addr, sizeof(serv_addr)); // to clear all previous server addresses and junk values
    
    portno = 1234;
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    {
        std::cout << "ERROR on Binding";
    }
    
    listen(sockfd, 5); // upto 5 clients server can listen
    clilen = sizeof(cli_addr);
    
    while (newsockfd = accept(sockfd,&cli_addr, &clilen))
    {
        std::cout<< "New client connected" << std::endl;
        client = new ChatClient(newsockfd);
        //client->registerMessageReceivedCallback(std::bind(&ChatServer::msgCallback, this, std::placeholders::_1, std::placeholders::_2));
        //std::string msg = "Hello";
        //msgCallback(&msg, client);
    }
        
    if (newsockfd < 0)
    {
        std::cout<< "ERROR on acceptance";
    }
}

ChatServer::~ChatServer()
{
    msgProcessingThread.join();
}