#include <atomic>
#include <cmath>
#include <iostream>
#include <limits>
#include <map>
#include <string>
#include <unordered_map>

#include "../../common/extension_functions.h"
#include "../../compiler_frontend/include.h"
#include "../../clean_up/clean_up.h"
#include "../../data_types/array.h"
#include "../../data_types/list.h"
#include "../../data_types/map.h"
#include "../../data_types/str_view.h"
#include "../../data_types/type_info.h"
#include "../../mangler/scope_struct.h"
#include "../../pool/pool.h"
#include "../include.h"



bool GC_Arena::mark_worklist_pointers(Scope_Struct *scope_struct, uint64_t mark_bit) {
    uint16_t type16;
    GC_Node node;

    std::unique_lock<std::mutex> lock(sweep_mtx, std::defer_lock);
    bool locked=false;
    while (scope_struct->alive) {
        if (!topw) {
            // if (!locked) {
            //     lock.lock();
            //     locked = true;
            // }
            topw = mutatorw.exchange(nullptr, std::memory_order_acquire);
            if(!topw) {
                break;
            }
        }

        node = topw->node;
        walkw();
        if(!mark_obj(node.ptr, type16, mark_bit))
            continue;
        TypeInfo *class_info = type_info[type16]; 
        if (class_info!=nullptr) {
            for (int ptr_i=0; ptr_i<class_info->pointers_count; ++ptr_i) {
                PtrInfo *ptr_info = &class_info->ptr_info[ptr_i];
                uint16_t offset = ptr_info->offset;
                uint16_t nested_type = ptr_info->type;
                
                if(!node.ptr)
                    continue;
                void** slot_ptr = reinterpret_cast<void**>(
                    static_cast<char*>(node.ptr) + offset
                );
                void* slot = __atomic_load_n(slot_ptr, __ATOMIC_ACQUIRE);

                if(slot)
                    worklist_push(slot, nested_type);
            }
        }
        gc_list(node.ptr, type16, mark_bit);
    }
    if (!locked)
        lock.lock();

    __atomic_store_n(&gc->marking, false, __ATOMIC_RELEASE);
    topw=nullptr;
    if(scope_struct->alive)
        gc->CleanUp_Unused(scope_struct->thread_id);
    lock.unlock();
    return true;
}


inline bool check_is_in_bounds(char *arena_addr, char *p) {
    bool in_bounds = (p>=arena_addr&&p<arena_addr+GC_arena_size);
    return in_bounds;
}

void *GC_Arena::WriteBarrierAlloc(GC_Span *span, uint16_t type_id, uint64_t mark_bit) {
    void *ptr = span->Allocate(type_id, mark_bit);
    // TypeInfo *class_info = type_info[type_id]; 
    // if (class_info!=nullptr) {
    //     for (int ptr_i=0; ptr_i<class_info->pointers_count; ++ptr_i) {
    //         PtrInfo *ptr_info = &class_info->ptr_info[ptr_i];
    //         uint16_t offset = ptr_info->offset;
            
    //         void** slot_ptr = reinterpret_cast<void**>(
    //             static_cast<char*>(ptr) + offset
    //         );
    //         __atomic_store_n(slot_ptr, nullptr, __ATOMIC_RELEASE);
    //     }
    // }
    // if (gc->marking)
    //     mutator_push(ptr, type_id);
    return ptr;
}

bool GC_Arena::is_safe(void *node_ptr) {
    if (!node_ptr)
        return false;
    
    char *arena_addr = (char*)arena;
    char *p = static_cast<char*>(node_ptr);
    
    if (!check_is_in_bounds(arena_addr, p))
        return false;

    return true;
}

bool GC_Arena::get_is_marked(void *node_ptr, uint64_t mark_bit) {
        if (!node_ptr)
            return true;
        
        char *arena_addr = (char*)arena;
        char *p = static_cast<char*>(node_ptr);
        
        if (!check_is_in_bounds(arena_addr, p))
            return true;
        long arena_offset = p - arena_addr;
        int page  =  (arena_offset / GC_page_size) % pages_per_arena;
 
        GC_Span *span = page_to_span[page];

        long obj_idx = (p - static_cast<char*>(span->span_address)) / span->traits->obj_size;

        return get_1_atomic(span->mark_bits, obj_idx) == mark_bit;
}

bool GC_Arena::mark_obj(void *node_ptr, uint16_t &type, uint64_t mark_bit) {
        if (!node_ptr)
            return false;
        
        char *arena_addr = (char*)arena;
        char *p = static_cast<char*>(node_ptr);
        
        if (!check_is_in_bounds(arena_addr, p))
            return false;
        
        long arena_offset = p - arena_addr;
        int page  =  (arena_offset / GC_page_size) % pages_per_arena;
 
        GC_Span *span = page_to_span[page];

        long obj_idx = (p - static_cast<char*>(span->span_address)) / span->traits->obj_size;

        // prevent cyclic ref infinite loop
        bool was_handled = get_1_atomic(span->mark_bits, obj_idx) == mark_bit;
        if (was_handled) return false;
        
        type = get_16_r12(span->type_metadata, obj_idx);
        set_1_atomic(span->mark_bits, obj_idx, mark_bit);

        return true;
}

void GC_Arena::gc_list(void *ptr, uint16_t root_type, uint64_t mark_bit) {
    if (!is_safe(ptr))
        return;
    uint16_t type16;
    // if (root_type=="channel") {
    //     LogBlue("FROM ROOT OF CHANNEL");
    // }
    // if (root_type==uint16_t{6}) { // list
    //     DT_list *list = static_cast<DT_list*>(ptr);
    //     for (int i=0; i<list->size; ++i) {
    //         const char *type = list->data_types->at(i).c_str(); 
    //         uint16_t list_type = data_name_to_type[type];
    //         if(in_vec(list_type, compound_types)) {
    //             gc_list(list->get<void*>(i), list_type, mark_bit);
    //             continue;
    //         }
    //         if(list_type==uint16_t{100}) { //str
    //             mark_obj(list->get<char*>(i), type16, mark_bit);
    //             continue;
    //         }
    //         if(!in_vec(list_type, primary_data_types)) //not a primary
    //             mark_obj(list->get<void*>(i), type16, mark_bit);
    //     }
    // }
    if (root_type==uint16_t{102}) { // array 
        DT_array *array = static_cast<DT_array*>(ptr);
        // void **data = static_cast<void **>(array->data);
        void *datap = __atomic_load_n(&array->data, __ATOMIC_ACQUIRE);
        void **data = (void**)datap;
        int size = __atomic_load_n(&array->virtual_size, __ATOMIC_ACQUIRE);
        uint16_t type = __atomic_load_n(&array->type, __ATOMIC_ACQUIRE);
        if (!data)
            return;
        // worklist_push((void*)data, 100);
        if(type<100) // is primary
            return;

        for (int i=0; i<size; ++i) {
            void *elem = __atomic_load_n(&data[i], __ATOMIC_ACQUIRE);
            // if(!get_is_marked(elem, type)) {
                worklist_push(elem, type);
            // }
        }
    }
    if (root_type==uint16_t{108}) { // map 
        DT_map *map = static_cast<DT_map*>(ptr);
        uint16_t ktype = data_name_to_type()[map->key_type];
        uint16_t vtype = data_name_to_type()[map->val_type];
        bool mark_key = ktype>=100; //not primary
        bool mark_val = vtype>=100;

        for (int i=0; i<map->capacity; ++i) {
            DT_map_node *node = map->nodes[i];
            while (node!=nullptr) {
                mark_obj(node, type16, mark_bit);
                if (mark_key)
                    mark_obj(node->key, type16, mark_bit);
                if(mark_val)
                    worklist_push(node->value, vtype);
                node = node->next;
            }
        }
    }
}


int marks = 0;
extern "C" void GC_array_append_barrier(Scope_Struct *ctx, DT_array *vec, void *ptr, uint16_t type) {
    GC *gc = ctx->gc;
    int size = __atomic_load_n(&vec->size, __ATOMIC_ACQUIRE);
    int idx = __atomic_load_n(&vec->virtual_size, __ATOMIC_ACQUIRE);
    if (idx>=size)
        array_double_size(ctx, vec);
    
    if (type<100) {
        std::cout << "APPEND PRIMARY" << "\n";
        std::exit(0);
    }

    void *tmp = __atomic_load_n(&vec->data, __ATOMIC_ACQUIRE);
    void **data = (void**)tmp;
    void **slot = &data[idx];
    void *old = __atomic_load_n(slot, __ATOMIC_ACQUIRE);

    GC_Arena *arena = gc->arena;
    marks++;
    
    // bool marking = __atomic_load_n(&gc->marking, __ATOMIC_ACQUIRE);
    // if (marking) {
        // bool is_marked = arena->get_is_marked(vec, gc->mark_bit);
        // if(!is_marked&&old!=nullptr)
        //     arena->mutator_push(old, type);
    // // }
    __atomic_store_n(slot, ptr, __ATOMIC_RELEASE);
    int vsize = 1 + __atomic_load_n(&vec->virtual_size, __ATOMIC_ACQUIRE);
    __atomic_store_n(&vec->virtual_size, vsize, __ATOMIC_RELEASE);
    // bool marking = __atomic_load_n(&gc->marking, __ATOMIC_ACQUIRE);
    // if (marking) {
        bool is_marked = arena->get_is_marked(vec, gc->mark_bit);
        if(is_marked&&ptr!=nullptr)
            arena->mutator_push(ptr, type);
    // }


    // if(old!=nullptr)
    //     arena->mutator_push(old, type);
    // bool is_marked = arena->get_is_marked(vec, gc->mark_bit);
    // if(is_marked&&ptr!=nullptr)
    //     arena->mutator_push(ptr, type);
    // __atomic_store_n(slot, ptr, __ATOMIC_RELEASE);
    // int vsize = 1 + __atomic_load_n(&vec->virtual_size, __ATOMIC_ACQUIRE);
    // __atomic_store_n(&vec->virtual_size, vsize, __ATOMIC_RELEASE);
}


extern "C" void GC_write_barrier_str(uint16_t type, GC *gc, void *src, void **slot, char *ptr, int size) {
    marks++;
    GC_Arena *arena = gc->arena;

    // bool marking = __atomic_load_n(&gc->marking, __ATOMIC_ACQUIRE);
    // if (marking) {
        // void* old = __atomic_load_n(slot, __ATOMIC_ACQUIRE);
        // bool is_marked = arena->get_is_marked(src, gc->mark_bit);
        // if(!is_marked&&old!=nullptr)
        //     arena->mutator_push(old, type);
    // }
    __atomic_store_n(slot, ptr, __ATOMIC_RELEASE);
    __atomic_store_n((int*)((char*)slot+8), size, __ATOMIC_RELEASE);
    // bool marking = __atomic_load_n(&gc->marking, __ATOMIC_ACQUIRE);
    // if (marking) {
        bool is_marked = arena->get_is_marked(src, gc->mark_bit);
        if(is_marked&&ptr!=nullptr)
            arena->mutator_push(ptr, type);
    // }

    // void* old = __atomic_load_n(slot, __ATOMIC_ACQUIRE);
    // bool is_marked = arena->get_is_marked(src, gc->mark_bit);
    // if(old!=nullptr)
    //     arena->mutator_push(old, type);
    // if(is_marked&&ptr!=nullptr)
    //     arena->mutator_push(ptr, type);
    // __atomic_store_n(slot, ptr, __ATOMIC_RELEASE);
    // __atomic_store_n((int*)((char*)slot+8), size, __ATOMIC_RELEASE);
}
extern "C" void GC_write_barrier_obj(uint16_t type, GC *gc, void *src, void **slot, void *ptr) {
    marks++;
    GC_Arena *arena = gc->arena;
    
    // Yuasa
    // bool marking = __atomic_load_n(&gc->marking, __ATOMIC_ACQUIRE);
    // if (marking) {
        // void* old = __atomic_load_n(slot, __ATOMIC_ACQUIRE);
        // bool is_marked = arena->get_is_marked(src, gc->mark_bit);
        // if(!is_marked&&old!=nullptr)
        //     arena->mutator_push(old, type);
        // std::cout << "mark barrier " << old << ", set to " << " ptr " << "\n";
    // }
    __atomic_store_n(slot, ptr, __ATOMIC_RELEASE);

    // Dijkstra
    // __atomic_store_n(slot, ptr, __ATOMIC_RELEASE);
    // bool marking = __atomic_load_n(&gc->marking, __ATOMIC_ACQUIRE);
    // if (marking) {
        bool is_marked = arena->get_is_marked(src, gc->mark_bit);
        if(is_marked&&ptr!=nullptr)
            arena->mutator_push(ptr, type);
    // }
    

    // void* old = __atomic_load_n(slot, __ATOMIC_ACQUIRE);
    // bool is_marked = arena->get_is_marked(src, gc->mark_bit);
    // if(old!=nullptr)
    //     arena->mutator_push(old, type);
    // if(is_marked&&ptr!=nullptr)
    //     arena->mutator_push(ptr, type);
    // __atomic_store_n(slot, ptr, __ATOMIC_RELEASE);
}


bool GC_Arena::mark_worklist_pointers2(Scope_Struct *scope_struct, uint64_t mark_bit) {
    uint16_t type16;
    GC_Node node;

    while (scope_struct->alive) {
        if (!topw) {
            topw = mutatorw.exchange(nullptr, std::memory_order_acquire);
            return topw==nullptr;
        }

        node = topw->node;
        walkw();
        if(!mark_obj(node.ptr, type16, mark_bit))
            continue;
        TypeInfo *class_info = type_info[type16]; 
        if (class_info!=nullptr) {
            for (int ptr_i=0; ptr_i<class_info->pointers_count; ++ptr_i) {
                PtrInfo *ptr_info = &class_info->ptr_info[ptr_i];
                uint16_t offset = ptr_info->offset;
                uint16_t nested_type = ptr_info->type;
                
                if(!node.ptr)
                    continue;
                void** slot_ptr = reinterpret_cast<void**>(
                    static_cast<char*>(node.ptr) + offset
                );
                void* slot = __atomic_load_n(slot_ptr, __ATOMIC_ACQUIRE);

                if(slot)
                    worklist_push(slot, nested_type);
            }
        }
        gc_list(node.ptr, type16, mark_bit);
    }
    return true;
}

void GC_Arena::check_roots_worklist(Scope_Struct *scope_struct, uint64_t mark_bit) {
    uint16_t type16;

    int stack_size = __atomic_load_n(&scope_struct->stack_top, __ATOMIC_ACQUIRE); 
    // std::cout << "stack size: " << stack_size << "\n";
    for (int i=0; i<stack_size; ++i) {
        void *root_ptr = scope_struct->pointers_stack[i];
        // std::cout << "root " << root_ptr << "\n";
        if (!mark_obj(root_ptr, type16, mark_bit))
            continue;

        TypeInfo *class_info = type_info[type16]; 
        if (class_info!=nullptr) {
            for (int ptr_i=0; ptr_i<class_info->pointers_count; ++ptr_i) {
                PtrInfo *ptr_info = &class_info->ptr_info[ptr_i];
                int offset = ptr_info->offset;
                uint16_t nested_type = ptr_info->type;

                void** slot_ptr = reinterpret_cast<void**>(
                    static_cast<char*>(root_ptr) + offset
                );
                void* slot = __atomic_load_n(slot_ptr, __ATOMIC_ACQUIRE);
                if(slot!=nullptr)
                    worklist_push(slot, nested_type);
            }
        }
        gc_list(root_ptr, type16, mark_bit);
    }
}



// sweep
void GC::Sweep(Scope_Struct *scope_struct) {
    int tid = scope_struct->thread_id;
    __atomic_store_n(&marking, true, __ATOMIC_RELEASE);
    std::unique_lock<std::mutex> lock(arena->sweep_mtx, std::defer_lock);

    // lock.lock();
    // arena->check_roots_worklist(scope_struct, mark_bit);
    // lock.unlock();
    // arena->mark_worklist_pointers(scope_struct, mark_bit);
    
    bool is_first=true;
    while (scope_struct->alive) {
        lock.lock();
        arena->check_roots_worklist(scope_struct, mark_bit);
        lock.unlock();

        bool empty = arena->mark_worklist_pointers2(scope_struct, mark_bit);
        void *mutatorw = arena->mutatorw.load(std::memory_order_acquire);
        if (empty&&!mutatorw) {
            lock.lock();
            std::cout << "sweep " << "\n";
            if(scope_struct->alive)
                CleanUp_Unused(scope_struct->thread_id);
            __atomic_store_n(&marking, false, __ATOMIC_RELEASE);
            allocations=0;
            size_occupied=0;
            marks=0;

            lock.unlock();
            return;
        } 
        arena->mark_worklist_pointers2(scope_struct, mark_bit);
    }
    std::cout << "sweep " << "\n";

    __atomic_store_n(&marking, false, __ATOMIC_RELEASE);
    allocations=0;
    size_occupied=0;
}



void GC::CleanUp_Unused(int tid) {
    int get_mask = mark_bit ? 0 : 1;
    uint64_t mark_mask = mark_bit ? ~0ULL : 0ULL;
    int all_alloc=0;

    for (int span_group=0; span_group<GC_obj_sizes; span_group++) {
        GC_Span *span_ST = nullptr, *free_span_ST = nullptr, *last_free = nullptr;
        
        int span_id=-1;

        for (const auto &span : arena->Spans[span_group]) {
            span_id++;

            int obj_size = span->elem_size;
            int free_slots = 0; 
            
            for (int w=0; w<span->words; ++w) {
                uint64_t bits = span->mark_bits[w].load(std::memory_order_acquire);
                bits = bits^mark_mask; 
                while (bits) {
                    int idx = (w<<6) + __builtin_ctzll(bits);
                    if(idx>=span->N)
                        break;
                    if (get_1(span->alloc_bits, idx)) {
                        uint16_t u_type = get_16_r12(span->type_metadata, idx);
                        if(u_type!=0) {
                            if(u_type!=100&&type_info[u_type]==nullptr) { // Not str and not class
                                std::string obj_type = data_type_to_name()[u_type]; 
                                void *obj_addr = static_cast<char*>(span->span_address) + idx*obj_size;
                                // std::cout << "CLEAN " << obj_type << " - " << obj_addr << "\n";
                                clean_up_functions[obj_type](obj_addr, tid);
                            }
                        }
                        set_1(span->alloc_bits, idx, 0ULL); 
                    }
                    uint64_t set_mask = 1ULL << (idx&63);
                    bits = bits & ~set_mask;
                }
                span->mark_bits[w].store(mark_mask, std::memory_order_release);

                bits = span->alloc_bits[w] ^ ~0ULL;
                while (bits) {
                    int idx = (w<<6) + __builtin_ctzll(bits);
                    free_slots++;
                    all_alloc++;
                    if(idx>=span->N)
                        break;
                    uint64_t set_mask = 1ULL << (idx&63);
                    bits = bits & ~set_mask;
                }
            }

            // std::cout << free_slots << " | " << span->N << " -- " << obj_size << "|" << span_id << "\n";
            // bool is_free = (span->free_idx<span->N||free_slots==span->N);
            // if (is_free) {
            //     if(!last_free)
            //         last_free = span;
            //     span->next_span = free_span_ST;
            //     free_span_ST = span;
            // } else {
                span->next_span = span_ST;
                span_ST = span;
            // }
            // if (free_slots>=span->N) {
            //     // std::cout << "ALL CLEAR" << "\n";
                // span->cur_free = (char*)span->span_address;
                // span->free_idx=0;
            // }
        }
        GC_Span *first_span = span_ST;
        // if (last_free) {
        //     last_free->next_span = span_ST;
        //     first_span = free_span_ST;
        // }
        arena->current_span[span_group] = first_span;
    }
    // std::exit(0);
    // std::cout << "all alloc " << all_alloc << "\n";
    mark_bit ^= 1;
}
