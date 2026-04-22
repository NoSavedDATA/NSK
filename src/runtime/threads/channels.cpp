#include <cassert>
#include <cstddef>
#include <cstring>


#include "../pool/include.h"
#include "channels.h"



Channel::Channel() {
}

void Channel::New(Scope_Struct *scope_struct, uint16_t type, int buffer_size) {
    this->buffer_size = buffer_size;
    this->type = type;

    int elem_size;
    if(data_type_to_size.count(type)>0)
      elem_size = data_type_to_size[type];
    else
      elem_size = 8;

    _array = newT<DT_array>(scope_struct, "array");
    _array->New(scope_struct, buffer_size, elem_size, scope_struct->thread_id, type);
    __atomic_store_n(&_array->virtual_size, buffer_size, __ATOMIC_RELEASE);

    // data_list = new DT_list();
}


extern "C" void *channel_Create(Scope_Struct *scope_struct, uint16_t type, int buffer_size) {
    Channel *ch = newT<Channel>(scope_struct, "channel");
    ch->New(scope_struct, type, buffer_size);
    return ch;
}





// extern "C" void *void_channel_message(Scope_Struct *scope_struct, void *ptr, Channel *ch) {    
//     std::unique_lock<std::mutex> lock(ch->mtx);

//     ch->cv.wait(lock, [&]{ return ch->terminated || ch->data_list->data->size() > 0; } );    
//     if(ch->terminated)
//         return ptr;

//     void *ret = ch->data_list->unqueue<void*>();
//     ch->cv.notify_all();

//     return ret;
// }

// extern "C" float channel_void_message(Scope_Struct *scope_struct, Channel *ch, void *ptr) {
//     std::unique_lock<std::mutex> lock(ch->mtx);

//     ch->cv.wait(lock, [&]{ return ch->terminated || ch->data_list->data->size() < ch->buffer_size; } );    
//     if(ch->terminated)
//         return -1;

//     ch->data_list->append(std::any(ptr), "any");
//     ch->cv.notify_all();
    
//     return 0;
// }



extern "C" void str_channel_terminate(Scope_Struct *scope_struct, Channel *ch) {
    {
        std::unique_lock<std::mutex> lock(ch->mtx);
        ch->terminated = true;
    }
    ch->cv.notify_all();  // wake up all waiting senders/receivers
}


extern "C" int str_channel_alive(Scope_Struct *scope_struct, Channel *ch) {
    return !ch->terminated;
}















extern "C" float float_channel_terminate(Scope_Struct *scope_struct, Channel *ch) {
    {
        std::unique_lock<std::mutex> lock(ch->mtx);
        ch->terminated = true;
    }
    ch->cv.notify_all();  // wake up all waiting senders/receivers
    return 0;
}
extern "C" int float_channel_alive(Scope_Struct *scope_struct, Channel *ch) {
    return !ch->terminated;
}




// int x <- ch
extern "C" int int_channel_message(Scope_Struct *scope_struct, void *ptr, Channel *ch) {    
    std::unique_lock<std::mutex> lock(ch->mtx);
    std::cout << "consumer " <<ch->size << ", " << ch->head << " - " << ch->tail << "\n";
    ch->pop_cv.wait(lock, [&]{ return ch->terminated || ch->size > 0; } );
    if(ch->terminated)
        return -1;

    DT_array *arr = ch->_array;
    int *data = (int*)arr->data;
    int x = data[ch->head];
    ch->head = (ch->head+1)%ch->buffer_size;
    ch->size--;

    ch->push_cv.notify_all();

    return x;
}

// ch <- msg
extern "C" float channel_int_message(Scope_Struct *scope_struct, Channel *ch, int x) {
    std::unique_lock<std::mutex> lock(ch->mtx);
    DT_array *arr = ch->_array;
    std::cout << "producer " <<ch->size << ", " << ch->head << " - " << ch->tail << "\n";
    ch->push_cv.wait(lock, [&]{ return ch->terminated || \
                ch->size < ch->buffer_size; } );
    if(ch->terminated)
        return -1;

    int *data = (int*)arr->data;
    data[ch->tail] = x;
    ch->tail = (ch->tail+1)%ch->buffer_size;
    ch->size++;

    std::cout << "producer DONE " <<ch->size << ", " << ch->head << " - " << ch->tail << "\n";
    ch->pop_cv.notify_all();
    
    return 0;
}

extern "C" int int_channel_Idx(Scope_Struct *scope_struct, Channel *ch, int idx) {
    std::unique_lock<std::mutex> lock(ch->mtx);
    DT_array *arr = ch->_array;

    ch->cv.wait(lock, [&]{ return ch->terminated || ch->size > 0; } );    
    if(ch->terminated)
        return -1;

    int *data = (int*)arr->data;
    int res = data[idx];
    std::cout << "todo  int_channel_Idx check size" << "\n";
    // ch->size--;

    ch->cv.notify_all();

    return res;
}

extern "C" int int_channel_sum(Scope_Struct *scope_struct, Channel *ch) {
    std::unique_lock<std::mutex> lock(ch->mtx);
    DT_array *arr = ch->_array;

    ch->cv.wait(lock, [&]{ return ch->terminated || ch->size <= ch->buffer_size; } );
    if(ch->terminated)
        return -1;

    int *data = (int*)arr->data;

    int sum=0;
    for(int i=0; i<ch->buffer_size; ++i)
        sum += data[i];

    ch->cv.notify_all();
    
    return sum;
}



extern "C" float int_channel_terminate(Scope_Struct *scope_struct, Channel *ch) {
    {
        std::unique_lock<std::mutex> lock(ch->mtx);
        ch->terminated = true;
    }
    ch->cv.notify_all();  // wake up all waiting senders/receivers
    return 0;
}

extern "C" bool int_channel_alive(Scope_Struct *scope_struct, Channel *ch) {
    return !ch->terminated;
}


void channel_Clean_Up(void *ptr, int tid) {
}
