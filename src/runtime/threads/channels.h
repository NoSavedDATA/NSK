#pragma once


#include <condition_variable>
#include <iostream>
#include <map>
#include <mutex>
#include <string>


#include "../data_types/array.h"
#include "barrier.h"




struct Channel {
    int buffer_size; 
    uint16_t type;
    bool terminated=false;

    DT_array *_array;
    size_t head=0, tail=0, size=0;
    size_t *seq;
    
    std::mutex mtx;
    std::condition_variable push_cv, pop_cv, cv;

    Channel();
    void New(Scope_Struct *, uint16_t, int);
};

void channel_Clean_Up(void *ptr, int);


void channel_handle_pool(Scope_Struct *scope_struct, void *ptr, char *data_name);
