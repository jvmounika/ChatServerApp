//
//  ChatClient.cpp
//  ChatServerClient
//

#include "ChatClient.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

ChatClient::ChatClient(std::string ipaddress, int portNumber)
{
    printMessage("Chat Client constructor");
    // call connect on the port number and start the read thread
    int socketfd;
    struct sockaddr_in server_addr;
    struct hostent* server;
    int ret;

    socketfd = socket(AF_INET,SOCK_STREAM,0);
    printMessageAndNumber("Socket created: ", socketfd);
    
    
    bzero((char *) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(portNumber);
    
    
    server = gethostbyname(ipaddress.c_str());
    memcpy(server->h_addr,&(server_addr.sin_addr.s_addr),sizeof(server->h_length));
    ret = connect(socketfd,(struct sockaddr*)&server_addr,sizeof(server_addr));
    printMessageAndNumber("Connect returned: ", ret);
    if(ret == -1)
    {
        return;
    }
    this->socketfd = socketfd;
    bytesReadSoFar = 0;

    // TODO : create a thread
    readThread = std::thread(&ChatClient::readThreadWorker, this);
}

ChatClient::~ChatClient()
{
    readThread.join();
}

int ChatClient::authenticate_session_client()
{
    char inputMessage[100];
    // send greeting message
    sendMessage(TCP_SEND_CLIENT_GREETING);
    
    // receive greeting message
    receiveMessage();
    
    // send username
    printMessage(("Enter the user name"));
    std::cin >> inputMessage;
    
    sendMessage(inputMessage);

    // Recv if the connection is Accepted/Denied
    receiveMessage();
    
	if (strncmp(messageBuffer+2, TCP_ACCEPT_CONNECTION, bytesReadSoFar-2) != 0)
	{
        printMessage("Connection deined ");
		return -1;
	}
	for(int i = 0; i < 3; i++)
	{
	    // Send password
        printMessage("Enter password");
        std::cin >> inputMessage;
        sendMessage(inputMessage);
        
	    // Recv if the connection is Accepted/Denied
        receiveMessage();
        
	    if (strncmp(messageBuffer+2, TCP_ACCEPT_CONNECTION, bytesReadSoFar-2) == 0)
	    {
            printMessage("Connection accepted");
            return 0;
	    	break;
	    }
	    else if ((strncmp(messageBuffer+2, TCP_RESEND_PWD, bytesReadSoFar-2) == 0) && (i < 2))
	    {
            printMessage("Try again");
	    }
	    else if (strncmp(messageBuffer+2, TCP_DENY_CONNECTION, bytesReadSoFar-2) == 0)
	    {
            printMessage("Connection denied");
	    	break;
	    }
	}
    
	// Cannot find user record
	return -1;
}

void ChatClient::registerMessageReceivedListener(std::function<void(std::string*)> msgReceivedCallback)
{
    this->msgReceivedCallback = msgReceivedCallback;
}

int ChatClient::getMenuInputFromUser()
{
    int inputNumber = 0;
    while(1)
    {
        printMessage("Please select one from the following");
        printMessage("1. Get list of users");
        printMessage("2. Get list of topics");
        printMessage("3. Subscribe to a topic");
        printMessage("4. Publish on a topic");
        printMessage("5. Get new posts");
        printMessage("6. Get recent posts");
        printMessage("7. Close session");
        std::cin >> inputNumber;
        if ((inputNumber >= 1) && (inputNumber <= 7))
        {
            printMessageAndNumber("You have entered number:", inputNumber);
            return inputNumber;
        }
        printMessage("Invalid input. Please retry");
        std::cin.ignore();
    }
}

void ChatClient::processMessageClient(std::string input)
{
    std::string message;
    if (input[0] == GET_USERS)
    {
        printMessage("List of available users ");
    }
    else if (input[0] == GET_TOPICS)
    {
        printMessage("List of available topics ");
    }
    else if (input[0] == SUBSCRIBE_TO_A_TOPIC)
    {
        printMessageBuffer();
        return;
    }
    else if (input[0] == PUBLISH_TO_A_TOPIC)
    {
        printMessageBuffer();
        return;
    }
    else if (input[0] == GET_NEW_POSTS)
    {
        printMessage("New posts available are ");
    }
    else if (input[0] == GET_RECENT_POSTS)
    {
        printMessage("Recent posts available are ");
    }
    for (int i = 2; i < bytesReadSoFar; i++)
    {
        if (messageBuffer[i] == ',')
        {
            message += '\n';
        }
        else
        {
            message += messageBuffer[i];
        }
    }

    printMessage(message);

}

static const int MSG_START = 1;
static const int MSG_DATA = 2;
void ChatClient::readThreadWorker()
{
    // Wait until you are connected to the server
    //while (authenticate_session_client());
    printMessage("Now connected");
    
    while(1)
    {
        int inputNumber;
        std::string messageToBeSent;
        // Show menu and get input from user
        inputNumber = getMenuInputFromUser();
        messageToBeSent = std::to_string(inputNumber);
        if ((std::to_string(inputNumber)[0] == SUBSCRIBE_TO_A_TOPIC) ||
            (std::to_string(inputNumber)[0] == PUBLISH_TO_A_TOPIC))
        {
            std::string strInput;
            printMessage("Enter the topic number you wish to subscribe/post");
            std::cin.ignore();
            std::getline(std::cin, strInput);
            messageToBeSent += ",";
            messageToBeSent += strInput;
        }
        if (std::to_string(inputNumber)[0] == PUBLISH_TO_A_TOPIC)
        {
            std::string strInput;
            printMessage("Enter the post you want to publish");
            std::getline(std::cin, strInput);
            messageToBeSent += ",";
            messageToBeSent += strInput;
        }
        // Send input to server
        sendMessage(messageToBeSent);
        if (messageToBeSent[0] == CLOSE_SESSION)
        {
            printMessage("End session");
            break;
        }

        // Get response from server
        receiveMessage();
        
        processMessageClient(std::to_string(inputNumber));
        
        // display it to user   
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
        printMessageAndNumber("Waiting for data from Server: ", socketfd);
        //bytesRead = read(socketfd,buffer + bufferOffset,MAX_MSG_SIZE-bufferOffset);
        receiveMessage();
        printMessageAndNumber("BytesRead: ", bytesRead);
        if(bytesRead <= 0) {
            printMessage("Error Reading from socket");
            close(socketfd);
            return;
        }
        printMessageBuffer();
        
        switch (state) {
            case MSG_START:
                if(bytesRead >= 2) {
                    messageLength = (buffer[0]&0xFF) + ((buffer[1] << 8)&0xFF00);
                } else {
                    std::cout << "Error in parsing" << std::endl;
                }
                printMessageAndNumber("MessageLength: ", messageLength);
                if(messageLength == bytesRead-2) {
                    printMessage("Complete Message is received");
                    message = new std::string(buffer+2);
                    msgReceivedCallback(message);
                    bufferOffset = 0;
                    memset(buffer,0,sizeof(buffer));
                } else {
                    bufferOffset += bytesRead;
                    state = MSG_DATA;
                }
                break;
            case MSG_DATA:
                if(bytesRead + bufferOffset == messageLength-2) {
                    std::cout << "Complete Message Is received" << std::endl;
                    message = new std::string(buffer+2);
                    //
                    std::cout << "Complete Message Is received:" << *message << std::endl;
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
   // printMessageAndNumber("Sending a message of length:", length);
   //printMessage(message);
    write(socketfd,buffer,length+2);
}

void ChatClient::receiveMessage()
{
    bytesReadSoFar = read(socketfd, messageBuffer, MAX_MSG_SIZE);
    //printMessageAndNumber("Message received in bytes:", bytesReadSoFar);
    //printMessageBuffer();
}