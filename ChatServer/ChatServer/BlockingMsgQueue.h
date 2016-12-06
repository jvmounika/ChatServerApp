//
//  BlockingMsgQueue.h
//  ChatServer
//
//  Created by Mounika Jilla on 11/7/15.
//  Copyright (c) 2015 . All rights reserved.
//

#ifndef __ChatServer__BlockingMsgQueue__
#define __ChatServer__BlockingMsgQueue__

#include <iostream>
#include <string>
#include <sstream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>

using namespace std;

template<typename Data>
class BlockingMsgQueue
{
private:
    std::queue<Data> the_queue;
    mutable boost::mutex the_mutex;
    boost::condition_variable the_condition_variable;
public:
    void push(Data const& data);
    bool empty() const;
    bool try_pop(Data& popped_value);
    void wait_and_pop(Data& popped_value);
};


#endif /* defined(__ChatServer__BlockingMsgQueue__) */
