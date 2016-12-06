//
//  commonCode.cpp
//  ChatServer
//

#include "commonCode.h"

// Define the static mutex member of ChatClient before using it
std::mutex commonCode::mtx;

void commonCode::printMessageAndNumber(std::string message, int number)
{
    mtx.lock();
    std::cout << message << number << std::endl;
    mtx.unlock();
}

void commonCode::printMessage(std::string message)
{
    mtx.lock();
    std::cout << message << std::endl;
    mtx.unlock();
}

void commonCode::printMessageBuffer()
{
    mtx.lock();
    // The first 2bytes are message size so leave it
    for (int i = 2; i < bytesReadSoFar; i++)
    {
        std::cout << messageBuffer[i];
    }
    std::cout << std::endl;
    mtx.unlock();
}

