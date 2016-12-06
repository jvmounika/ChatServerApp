//
//  BlockingMsgQueue.cpp
//  ChatServer
//
//  Created by Mounika Jilla on 11/7/15.
//  Copyright (c) 2015. All rights reserved.
//

#include "BlockingMsgQueue.h"

template<typename Data> void BlockingMsgQueue<Data>::push(Data const& data)
{
    boost::mutex::scoped_lock lock(the_mutex);
    the_queue.push(data);
    lock.unlock();
    the_condition_variable.notify_one();
}

template<typename Data> bool BlockingMsgQueue<Data>::empty() const
{
    boost::mutex::scoped_lock lock(the_mutex);
    return the_queue.empty();
}

template<typename Data> bool BlockingMsgQueue<Data>::try_pop(Data& popped_value)
{
    boost::mutex::scoped_lock lock(the_mutex);
    if(the_queue.empty())
    {
        return false;
    }
    
    popped_value=the_queue.front();
    the_queue.pop();
    return true;
}

template<typename Data> void BlockingMsgQueue<Data>::wait_and_pop(Data& popped_value)
{
    boost::mutex::scoped_lock lock(the_mutex);
    while(the_queue.empty())
    {
        the_condition_variable.wait(lock);
    }
    
    popped_value=the_queue.front();
    the_queue.pop();
}
