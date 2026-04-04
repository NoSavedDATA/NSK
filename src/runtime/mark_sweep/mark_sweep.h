#pragma once

#include <cstdint>
#include <map>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <string>
#include <unordered_map>
#include <vector>

#include "../compiler_frontend/global_vars.h"
#include "../compiler_frontend/logging_v.h"
#include "../clean_up/clean_up.h"
#include "../data_types/list.h"
#include "../mangler/scope_struct.h"
#include "../pool/pool.h"
#include "include.h"

const int word_bits=64;

const int GC_page_size=8192;

const int sweep_after_alloc = 32 << 20;
const int GC_arena_size = 1024 << 20;

const int pages_per_arena = GC_arena_size / GC_page_size;

const int GC_obj_sizes=15;
const int GC_max_object_size = 16384;
extern int gc_sizes[GC_obj_sizes];

extern std::array<char *, 100> arena_base_addr;


constexpr size_t GC_ALIGN = 8; // 8-byte granularity
constexpr size_t GC_N = GC_max_object_size / GC_ALIGN;

extern uint16_t GC_size_to_class[GC_N+1];
extern uint16_t GC_size_to_c[GC_N+1];

struct Scope_Struct;
struct GC_span_traits;
struct GC_Arena;
struct GC_Node;
struct GC;

extern std::array<GC_span_traits*, GC_obj_sizes> GC_span_traits_vec;

struct GC_Node{
    void *ptr;
    uint16_t type;
    GC_Node(void *, uint16_t);
    GC_Node();
};


struct GC_span_traits {
    int pages=0, N=0, size, obj_size;
    GC_span_traits(int);
};

struct GC_Span {
    GC_span_traits *traits;
    GC_Arena *arena;
    void *span_address;
    char *cur_free, *end;
    GC_Span *next_span=nullptr;

    int words, type_words, free_idx=0, elem_size, N;

    // Interpretate type_metadata as int12
    uint64_t *mark_bits, *alloc_bits, *type_metadata;
    
    GC_Span(GC_Arena *, GC_span_traits *, uint64_t);
    inline void *Allocate(uint16_t type_id, uint64_t gc_mark_bit) {
        if(free_idx<N) {
            set_1(mark_bits, free_idx, gc_mark_bit);
            void *ret_ptr = static_cast<char*>(span_address) + elem_size*free_idx;
            alloc_bits[free_idx >> 6] |= (1ULL << (free_idx & 63));
            set_16_r12(type_metadata, free_idx, type_id);
            free_idx++;
            return ret_ptr;
        }
        for (int w=0; w<words; ++w) {
            uint64_t bits = alloc_bits[w] ^ ~0ULL;
            if (bits) {
                int idx = (w<<6) + __builtin_ctzll(bits);
                set_1(mark_bits, idx, gc_mark_bit);
                void *ret_ptr = static_cast<char*>(span_address) + elem_size*idx;
                alloc_bits[idx >> 6] |= (1ULL << (idx & 63));
                set_16_r12(type_metadata, idx, type_id);
                return ret_ptr;
            }
        }
        return nullptr;
    }
};


inline bool Check_Arena_Size_Ok(const int arena_size, const int size_allocated) {
    if(size_allocated>arena_size)
        return false; 
    return true;
}

struct WorkList {
    GC_Node node;
    WorkList *next=nullptr;
    WorkList(GC_Node);
};

struct GC_Arena {
    // Get an arena of 64MB, and set pages size to 8 KB
    const int arena_size=GC_arena_size, page=GC_page_size;
    // const int arena_size=65536, page=8192;
    int size_allocated=0,pages_allocated=0;
    void *arena, *metadata;
    GC *gc;

    std::mutex sweep_mtx, worklist_mtx;

    std::array<std::vector<GC_Span*>, GC_obj_sizes> Spans;
    std::array<std::vector<GC_Span*>, GC_obj_sizes> Free_Spans;
    std::array<GC_Span*, GC_obj_sizes> current_span{};
    std::array<GC_Span*, pages_per_arena> page_to_span;
    std::vector<GC_Node> worklist, mutator_list;
    bool owns_mutator=false;
    WorkList *topw=nullptr;
    // WorkList *mutatorw=nullptr;
    std::atomic<WorkList*> mutatorw{nullptr};

    GC_Arena(int);

    inline void mutator_push(void *ptr, uint16_t type_id) {
        std::unique_lock<std::mutex> lock(worklist_mtx);
        mutator_list.push_back(GC_Node(ptr, type_id));
        // WorkList* node = new WorkList(GC_Node(ptr, type_id));
        // WorkList* old_head;
        // do {
        //     old_head = mutatorw.load(std::memory_order_relaxed);
        //     node->next = old_head;
        // } while (!mutatorw.compare_exchange_weak(
        //     old_head,
        //     node,
        //     std::memory_order_release,
        // std::memory_order_relaxed));
    }

    inline void walkw() {
        WorkList *old = topw;
        topw = topw->next;
        delete old;
    }
    inline void worklist_push(void *ptr, uint16_t type_id) {
        std::unique_lock<std::mutex> lock(worklist_mtx);
        mutator_list.push_back(GC_Node(ptr, type_id));
        // WorkList *node = new WorkList(GC_Node(ptr, type_id));
        // node->next = topw;
        // topw = node;
    }

    inline bool worklist_pop(GC_Node &node) {
        std::unique_lock<std::mutex> lock(worklist_mtx);
        if (worklist.empty()) {
            if (mutator_list.empty())
                return false;
            worklist.insert(worklist.end(), mutator_list.begin(), mutator_list.end());
            mutator_list.clear();
        }
        node = worklist.back();
        worklist.pop_back();
        return true;
    }

    inline void* Allocate(int size_class, uint16_t type_id, uint64_t gc_mark_bit) {
        std::unique_lock<std::mutex> lock(sweep_mtx);

        GC_span_traits* traits = GC_span_traits_vec[size_class];
        GC_Span* span = current_span[size_class];
        GC_Span *prev_span = span;


        
        // FAST PATH
        if (span != nullptr)
        {
            void* ptr = span->Allocate(type_id, gc_mark_bit);
            if (ptr != nullptr)
                return ptr;
            while(span->next_span!=nullptr) { // only happens after resets
                span = span->next_span;
                ptr = span->Allocate(type_id, gc_mark_bit);
                if (ptr!=nullptr) {
                    current_span[size_class] = span;
                    return ptr;
                }
            }
        }


        // SLOW PATH — need new span
        if (!Check_Arena_Size_Ok(arena_size, size_allocated + traits->size))
            return nullptr;


        span = new GC_Span(this, traits, gc_mark_bit);
        if (prev_span!=nullptr)
            prev_span->next_span = span;

        current_span[size_class] = span;
        Spans[size_class].push_back(span);

        return span->Allocate(type_id, gc_mark_bit);
    }

    bool get_is_marked(void *node_ptr, uint64_t mark_bit);
    bool mark_obj(void *node_ptr, uint16_t &type, uint64_t mark_bit);
    void gc_list(void *ptr, uint16_t root_type, uint64_t mark_bit);
    void mark_worklist_pointers(Scope_Struct *scope_struct, uint64_t mark_bit);
    void check_roots_worklist(Scope_Struct *scope_struct, uint64_t mark_bit);
};

struct GC {
    int allocations=0;
    uint64_t size_occupied=0, mark_bit=1ULL;
    std::atomic<bool> marking{false};
    GC_Arena *arena;
    
    GC(int);
    inline void *Allocate(Scope_Struct *scope_struct, int size, uint16_t type_id, int tid) {

        // std::cout << "Allocate " << scope_struct << " | " << size << " - " << type_id << " | " << tid << "\n";
        int obj_class = GC_size_to_c[(size+7)/8];
        // std::cout << "obj class " << obj_class << "\n";

        if(size>GC_max_object_size) {
            LogErrorC(-1, "Allocated object of size " + std::to_string(size) + ", but the maximum supported object size is " + std::to_string(GC_max_object_size) + ".");
            return nullptr;
         }
        
        void *ptr = arena->Allocate(obj_class, type_id, mark_bit);
        if(ptr==nullptr) {
            Sweep(scope_struct);
            ptr = arena->Allocate(obj_class, type_id, mark_bit);
            if (ptr==nullptr) {
                LogErrorC(-1, "ARENA FULL.");
                std::exit(0);
            }
        }
        return ptr;
    }


    void Sweep(Scope_Struct *);
    void CleanUp_Unused(int);
};


//---------------------------------------------------------//






void protect_pool_addr(Scope_Struct *scope_struct, void *addr);
bool unprotect_pool_addr(Scope_Struct *scope_struct, void *addr);
