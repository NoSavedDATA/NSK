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
#include "../../data_types/type_info.h"
#include "../../mangler/scope_struct.h"
#include "../../pool/pool.h"
#include "../include.h"




inline bool check_is_in_bounds(char *arena_addr, char *p) {
    bool in_bounds = (p>=arena_addr&&p<arena_addr+GC_arena_size);
    return in_bounds;
}

void *GC_Arena::WriteBarrierAlloc(GC_Span *span, uint16_t type_id, uint64_t mark_bit) {
    void *ptr = span->Allocate(type_id, mark_bit);
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

        return get_1(span->mark_bits, obj_idx) == mark_bit;
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
        bool was_handled = get_1(span->mark_bits, obj_idx) == mark_bit;
        
        type = get_16_r12(span->type_metadata, obj_idx);
        set_1(span->mark_bits, obj_idx, mark_bit);

        // return !was_handled;
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
    //         if(list_type==uint16_t{5}) { //str
    //             mark_obj(list->get<char*>(i), type16, mark_bit);
    //             continue;
    //         }
    //         if(!in_vec(list_type, primary_data_types)) //not a primary
    //             mark_obj(list->get<void*>(i), type16, mark_bit);
    //     }
    // }
    if (root_type==uint16_t{12}) { // array 
        DT_array *array = static_cast<DT_array*>(ptr);
        if(in_vec(array->type, primary_data_tokens))
            return;

        void **data = static_cast<void **>(array->data);
        if (!data)
            return;
        for (int i=0; i<array->virtual_size; ++i) {
            if(!get_is_marked(data[i], data_name_to_type[array->type]))
                worklist_push(data[i], data_name_to_type[array->type]);
        }
    }
    if (root_type==uint16_t{8}) { // map 
        DT_map *map = static_cast<DT_map*>(ptr);
        uint16_t ktype = data_name_to_type[map->key_type];
        uint16_t vtype = data_name_to_type[map->val_type];
        bool mark_key = !in_vec(ktype, primary_data_types);
        bool mark_val = !in_vec(vtype, primary_data_types);

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


extern "C" void GC_array_append_barrier(GC *gc, DT_array *vec, int idx, void *ptr, uint16_t type) {
    if (idx>=vec->size) {
        std::cout << "VEC BAD SIZE" << "\n";
        std::exit(0);
    } 
    void **data = (void**)vec->data;
    void **slot = &((void**)vec->data)[idx];
    void *old = __atomic_load_n(slot, __ATOMIC_ACQUIRE);

    GC_Arena *arena = gc->arena;
    
    std::unique_lock<std::mutex> lock(arena->worklist_mtx);
    

    bool is_marked = arena->get_is_marked(vec, gc->mark_bit);

    // Yuasa
    if(!is_marked&&old!=nullptr)
        arena->mutator_push(old, type);
    __atomic_store_n(slot, ptr, __ATOMIC_RELEASE);
    // Dijkstra
    // if(is_marked&&ptr!=nullptr)
    //     arena->mutator_push(ptr, type);
}

extern "C" void GC_write_barrier_obj(GC *gc, void *src, void **slot, void *ptr, uint16_t type) {
    GC_Arena *arena = gc->arena;
    std::unique_lock<std::mutex> lock(arena->worklist_mtx);
    // Yuasa
    bool is_marked = arena->get_is_marked(src, gc->mark_bit);

    void* old = __atomic_load_n(slot, __ATOMIC_ACQUIRE);
    if(!is_marked&&old!=nullptr)
        arena->mutator_push(old, type);
    __atomic_store_n(slot, ptr, __ATOMIC_RELEASE);
    // Dijkstra
    // if(is_marked&&ptr!=nullptr)
    //     arena->mutator_push(ptr, type);
}


void GC_Arena::mark_worklist_pointers(Scope_Struct *scope_struct, uint64_t mark_bit) {
    uint16_t type16;
    GC_Node node;

    std::unique_lock<std::mutex> lock(sweep_mtx, std::defer_lock);
    bool locked=false;
    while (scope_struct->alive) {
        std::unique_lock<std::mutex> lock2(worklist_mtx);
        if (!topw) {
            if (!locked) {
                lock.lock();
                locked = true;
            }
            WorkList *stolen = mutatorw;
            mutatorw = nullptr;
            if(!stolen) {
                if (!mutatorw)
                    break;
                continue;
            }
            topw = stolen;
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

    gc->marking = false;
    topw=nullptr;
    if(scope_struct->alive)
        gc->CleanUp_Unused(scope_struct->thread_id);
    lock.unlock();
}


void GC_Arena::check_roots_worklist(Scope_Struct *scope_struct, uint64_t mark_bit) {
    {
        std::unique_lock<std::mutex> lock(sweep_mtx);
        uint16_t type16;

        for (int i=0; i<scope_struct->stack_top; ++i) {
            void *root_ptr = scope_struct->pointers_stack[i];
            if (!mark_obj(root_ptr, type16, mark_bit))
                continue;

            TypeInfo *class_info = type_info[type16]; 
            if (class_info!=nullptr) {
                for (int ptr_i=0; ptr_i<class_info->pointers_count; ++ptr_i) {
                    PtrInfo *ptr_info = &class_info->ptr_info[ptr_i];
                    int offset = ptr_info->offset;
                    uint16_t nested_type = ptr_info->type;

                    void *slot = *(void **)(static_cast<char*>(root_ptr)+offset);
                    // std::cout << "slot " << slot << "\n";
                    if(slot!=nullptr)
                        worklist_push(slot, nested_type);
                }
            }
            gc_list(root_ptr, type16, mark_bit);
        }
    }
    mark_worklist_pointers(scope_struct, mark_bit);
}



// sweep
void GC::Sweep(Scope_Struct *scope_struct) {
    int tid = scope_struct->thread_id;

    marking = true;
    arena->check_roots_worklist(scope_struct, mark_bit);
    marking = false;

    std::cout << "sweep " << "\n";


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
                uint64_t bits = span->mark_bits[w];
                bits = bits^mark_mask; 
                while (bits) {
                    int idx = (w<<6) + __builtin_ctzll(bits);
                    if(idx>=span->N)
                        break;
                    if (get_1(span->alloc_bits, idx)) {
                        uint16_t u_type = get_16_r12(span->type_metadata, idx);
                        if(u_type!=0) {
                            if(u_type!=5&&type_info[u_type]==nullptr) { // Not str and not class
                                const std::string &obj_type = data_type_to_name[u_type]; 
                                // if (u_type!=12)
                                // std::cout << "clean " << u_type << "|" << obj_type << "\n";
                                void *obj_addr = static_cast<char*>(span->span_address) + idx*obj_size;
                                clean_up_functions[obj_type](obj_addr, tid);
                            }
                        }
                        set_1(span->alloc_bits, idx, 0ULL); 
                    }
                    uint64_t set_mask = 1ULL << (idx&63);
                    bits = bits & ~set_mask;
                }
                span->mark_bits[w] = mark_mask;

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
            bool is_free = (span->free_idx<span->N||free_slots==span->N);
            if (is_free) {
                if(!last_free)
                    last_free = span;
                span->next_span = free_span_ST;
                free_span_ST = span;
            } else {
                span->next_span = span_ST;
                span_ST = span;
            }
            if (free_slots>=span->N) {
                // std::cout << "ALL CLEAR" << "\n";
                span->cur_free = (char*)span->span_address;
                span->free_idx=0;
            }
        }
        GC_Span *first_span = span_ST;
        if (last_free) {
            last_free->next_span = span_ST;
            first_span = free_span_ST;
        }
        arena->current_span[span_group] = first_span;
    }
    // std::exit(0);
    // std::cout << "all alloc " << all_alloc << "\n";
    mark_bit ^= 1;
    arena->owns_mutator = false;
}
