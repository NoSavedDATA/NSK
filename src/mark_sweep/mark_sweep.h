#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

#include "../compiler_frontend/global_vars.h"
#include "../compiler_frontend/logging_v.h"
#include "../compiler_frontend/logging.h"
#include "../clean_up/clean_up.h"
#include "../data_types/list.h"
#include "../mangler/scope_struct.h"
#include "../pool/pool.h"
#include "include.h"

const int word_bits=64;

const int GC_page_size=8192;

const int sweep_after_alloc = 32 << 20;
const int GC_arena_size = 64 << 20;
// const int GC_arena_size = 8192;

const int pages_per_arena = GC_arena_size / GC_page_size;

const int GC_obj_sizes=15;
const int GC_max_object_size = 16384;
extern int gc_sizes[GC_obj_sizes];

extern std::unordered_map<int, std::vector<char *>> arena_base_addr;


constexpr size_t GC_ALIGN = 8; // 8-byte granularity
constexpr size_t GC_N = GC_max_object_size / GC_ALIGN;

extern uint16_t GC_size_to_class[GC_N+1];
extern uint16_t GC_size_to_c[GC_N+1];

struct Scope_Struct;
struct GC_span_traits;
struct GC_Arena;
struct GC_Node;

extern std::array<GC_span_traits*, GC_obj_sizes> GC_span_traits_vec;


struct GC_span_traits {
    int pages=0, N=0, size, obj_size;
    GC_span_traits(int);
};

struct GC_Span {
    GC_span_traits *traits;
    GC_Arena *arena;
    void *span_address;

    int words, type_words;

    // Interpretate type_metadata as int12
    uint64_t *mark_bits, *type_metadata;
    
    GC_Span(GC_Arena *, GC_span_traits *);
    inline void *Allocate(uint16_t type_id) {
        // int free_idx = find_free(mark_bits, words);
        int free_idx = find_free_16_l2(type_metadata, type_words);
        if (free_idx==-1)
            return nullptr;
        // std::cout << "" << free_idx << "/" << traits->obj_size << "/" << traits->obj_size*free_idx << "\n";
        
        set_16_r12_mark(type_metadata, free_idx, type_id);
        return static_cast<char*>(span_address) + traits->obj_size*free_idx;
    }
};


inline bool Check_Arena_Size_Ok(const int arena_size, const int size_allocated) {
    if(size_allocated>arena_size)
        return false; 
    return true;
}

struct GC_Arena {
    // Get an arena of 64MB, and set pages size to 8 KB
    const int arena_size=GC_arena_size, page=GC_page_size;
    // const int arena_size=65536, page=8192;
    int size_allocated=0,pages_allocated=0;
    void *arena, *metadata;

    std::array<std::vector<GC_Span*>, GC_obj_sizes> Spans;
    std::array<GC_Span*, GC_obj_sizes> current_span{};
    std::array<GC_Span*, pages_per_arena> page_to_span;

    GC_Arena(int);
    inline void* Allocate(int size_class, uint16_t type_id) {
        GC_span_traits* traits = GC_span_traits_vec[size_class];

        GC_Span* span = current_span[size_class];

        // FAST PATH
        if (span != nullptr)
        {
            void* ptr = span->Allocate(type_id);
            if (ptr != nullptr)
                return ptr;
        }

        // SLOW PATH — need new span
        if (!Check_Arena_Size_Ok(arena_size, size_allocated + traits->size))
            return nullptr;

        span = new GC_Span(this, traits);

        current_span[size_class] = span;
        Spans[size_class].push_back(span);

        return span->Allocate(type_id);
    }

    // inline void *Allocate(int size_class, uint16_t type_id) {
    //     GC_Span *span;
    //     GC_span_traits *traits = GC_span_traits_vec[size_class];
    //     auto &span_list = Spans[size_class];

    //     if (span_list.empty()) {
    //         if (!Check_Arena_Size_Ok(arena_size, size_allocated+traits->size))
    //             return nullptr;        
    //         span = new GC_Span(this, traits);
    //         Spans[size_class].push_back(span);
    //         return span->Allocate(type_id);
    //     }

    //     int spans_count = Spans[size_class].size();

    //     void *ptr=nullptr;
    //     int i=0;
    //     while(ptr==nullptr) {
    //         if(i<spans_count)
    //             span = Spans[size_class][i];
    //         else {
    //             if (!Check_Arena_Size_Ok(arena_size, size_allocated+traits->size))
    //                 return nullptr;
    //             span = new GC_Span(this, traits);
    //             Spans[size_class].push_back(span);
    //         }
    //         ptr = span->Allocate(type_id);
    //         ++i;
    //     }
        
    //     return ptr;
    // }


};

struct GC {
    int allocations=0;
    uint64_t size_occupied=0;
    std::vector<GC_Arena*> arenas;
    
    GC(int);
    inline void *Allocate(int size, uint16_t type_id, int tid) {
        int obj_class = GC_size_to_c[(size+7)/8];
        // std::cout << "Allocate size: " << size << "/" << obj_class << "/" << type_id << ".\n";

        if(size>GC_max_object_size) {
            LogErrorC(-1, "Allocated object of size " + std::to_string(size) + ", but the maximum supported object size is " + std::to_string(GC_max_object_size) + ".");
            return nullptr;
        }


        void *address=nullptr;
        for (const auto &arena : arenas) {
            address = arena->Allocate(obj_class, type_id);
            // std::cout << "Arena: " << arena->arena << ".\n";
            if (address!=nullptr)
                break;
        }

        if (address==nullptr) {
            GC_Arena *new_arena = new GC_Arena(tid);
            arenas.push_back(new_arena);
            address = new_arena->Allocate(obj_class, type_id);
        }
        // std::cout << "got addr " << address << ".\n";
        return address;
    }

    void Sweep(Scope_Struct *);
    void CleanUp_Unused();
};



//---------------------------------------------------------//


struct GC_Node{
    void *ptr;
    std::string type;
    
    GC_Node(void *, std::string);
};




void protect_pool_addr(Scope_Struct *scope_struct, void *addr);
bool unprotect_pool_addr(Scope_Struct *scope_struct, void *addr);
