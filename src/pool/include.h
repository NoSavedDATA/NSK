#pragma once

#include <iostream>


#include "../compiler_frontend/global_vars.h"
#include "../compiler_frontend/logging_v.h"
#include "../mangler/scope_struct.h"
#include "../mark_sweep/include.h"

#include "pool.h"

extern std::unordered_map<int, std::unordered_map<int, std::vector<void*>>> memory_cache;

template<typename T>
T *allocate(Scope_Struct *scope_struct, int size, std::string type) {
    if (size==0)
        return nullptr;


    auto it = data_name_to_type.find(type);
    if (it==data_name_to_type.end())
        LogErrorC(-1, std::string("Type ") + type + " not implemented.");
    uint16_t type_id = it->second;


    int alloc_size = size*sizeof(T);
    // void *v_ptr = malloc(alloc_size);
    void *v_ptr = scope_struct->Allocate(alloc_size, type_id);
    
    scope_struct->gc->size_occupied += alloc_size;
    scope_struct->gc->allocations++;
    
    return static_cast<T*>(v_ptr);
}



template<typename T>
T *newT(Scope_Struct *scope_struct, std::string type) {
    T *ptr;

    if(scope_struct!=nullptr)
    {
        auto it = data_name_to_type.find(type);
        if (it==data_name_to_type.end())
            LogErrorC(-1, std::string("Type ") + type + " not implemented.");
        uint16_t type_id = it->second;

        void *v_ptr = scope_struct->Allocate(sizeof(T), type_id);
        ptr = new (v_ptr) T();

        // std::cout << " -- newT of --> " << type << "/" << type_id << " - / - " << ptr << ".\n";

        int size = sizeof(T);
        scope_struct->gc->size_occupied += size;
        scope_struct->gc->allocations++;

        // scope_struct->gc.pointer_nodes.push_back(GC_Node(static_cast<void *>(ptr), type));
   } else {
        std::cout << "allocate non-pool for " << type << ".\n";
        ptr = new T();
   }

    return ptr;
}

inline void *cache_pop(int size, int tid) {
    int obj_size = GC_size_to_class[(size+7)/8];

    auto& bucket = memory_cache[tid][obj_size];

    if (bucket.empty())
        return malloc(size);
    

    void* ptr = bucket.back();
    bucket.pop_back();
    return ptr;
}

inline void cache_push(void* ptr, int size, int tid) {
    int obj_size = GC_size_to_class[(size + 7) / 8];

    auto& bucket = memory_cache[tid][obj_size];

    bucket.push_back(ptr);
}
