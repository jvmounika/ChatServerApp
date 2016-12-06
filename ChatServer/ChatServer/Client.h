//
//  Client.h
//  ChatServer
//

#ifndef __ChatClient__Client__
#define __ChatClient__Client__

#include <thread>
#include <vector>
#include <map>
#include "commonCode.h"

struct Post
{
    int postTopicType;
    //        int postIndex;
    std::string text;
    //        std::string user;
    Post()
    {
        postTopicType = 0;
        text = "";
    }
    Post(int topicIndex, std::string postText)
    {
        postTopicType = topicIndex;
        text = postText;
    }
};

class ChatClient: public commonCode
{
private:
    std::thread readThread;
    int newsockfd;
    void readThreadWorker();
    std::function<void(std::string*, ChatClient *pChatClient)> msgReceivedCallback;
    const int MAX_MSG_SIZE = 1024;
    void processInputFromClient();
    static std::vector<Post> postBuffer;
    static int totalPostsCount;
    static std::mutex postAccessMtx;
    int postCountSeen;
    std::map<int, bool> subscribedTopics;

public:
    ChatClient(int sockfd);
    ~ChatClient();
    void sendMessage(std::string message);
    void receiveMessage();
    void registerMessageReceivedCallback(std::function<void(std::string*, ChatClient *pChatClient)> callback);
    int authenticate_session_server();
    void addPost(int topicIndex, std::string text);
};

#endif /* defined(__ChatClient__Client__) */
