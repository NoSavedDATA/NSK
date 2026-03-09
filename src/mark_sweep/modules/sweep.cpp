#include <cmath>
#include <iostream>
#include <limits>
#include <map>
#include <string>
#include <unordered_map>

#include "../../common/extension_functions.h"
#include "../../compiler_frontend/global_vars.h"
#include "../../compiler_frontend/logging_v.h"
#include "../../compiler_frontend/logging.h"
#include "../../compiler_frontend/tokenizer.h"
#include "../../clean_up/clean_up.h"
#include "../../data_types/array.h"
#include "../../data_types/list.h"
#include "../../data_types/map.h"
#include "../../data_types/type_info.h"
#include "../../mangler/scope_struct.h"
#include "../../pool/pool.h"
#include "../include.h"







// void Reset_Pools(GC_Arena *arena, uint64_t mark_bit) {
//     for (int span_group=0; span_group<GC_obj_sizes; span_group++) {
//         for (const auto &span : arena->Spans[span_group]) {
//             GC_span_traits *traits = span->traits;
//             for (int i=0; i<traits->N; ++i) {
//                 set_16_L1(span->type_metadata, i, 0u);
//                 // mark_bits_free(span->mark_bits, mark_bit);
//             }
//         }
//     }
// }

inline void check_is_in_bounds(char *arena_addr, char *p) {
    bool in_bounds = (p>=arena_addr&&p<arena_addr+GC_arena_size);
    if (!in_bounds) {
        std::cout << "Variable of type " << p << " address does not reside in any memory pool..\n";
        std::cout << p << ".\n";
        std::exit(0);
    }
}

inline void mark_obj(GC_Arena *arena, char *arena_addr, void *node_ptr, uint16_t &type, uint64_t mark_bit) {
        char *p = static_cast<char*>(node_ptr);
        check_is_in_bounds(arena_addr, p);

        long arena_offset = p - arena_addr;
        int page  =  (arena_offset / GC_page_size) % pages_per_arena;
 
        GC_Span *span = arena->page_to_span[page];

        long obj_idx = (p - static_cast<char*>(span->span_address)) / span->traits->obj_size;

        type = get_16_r12(span->type_metadata, obj_idx);
        set_16_L1(span->type_metadata, obj_idx, 1u);
        // set_1(span->mark_bits, obj_idx, mark_bit);
        // std::cout << "marked as " << mark_bit << ", got " << (get_1(span->mark_bits, obj_idx)) << " at idx " << obj_idx << "\n"; 
}



inline void gc_list(GC_Arena *arena, char *arena_addr, void *ptr, uint16_t root_type, std::vector<GC_Node> &work_list, uint64_t mark_bit) {
    uint16_t type16;
    // if (root_type=="channel") {
    //     LogBlue("FROM ROOT OF CHANNEL");
    // }
    if (root_type==uint16_t{6}) { // list
        DT_list *list = static_cast<DT_list*>(ptr);
        for (int i=0; i<list->size; ++i) {
            const char *type = list->data_types->at(i).c_str(); 
            uint16_t list_type = data_name_to_type[type];
            if(in_vec(list_type, compound_types)) {
                gc_list(arena, arena_addr, list->get<void*>(i), list_type, work_list, mark_bit);
                continue;
            }
            if(list_type==uint16_t{5}) { //str
                mark_obj(arena, arena_addr, list->get<char*>(i), type16, mark_bit);
                continue;
            }
            if(!in_vec(list_type, primary_data_types)) //not a primary
                mark_obj(arena, arena_addr, list->get<void*>(i), type16, mark_bit);
        }
    }
    if (root_type==uint16_t{12}) { // array 
        DT_array *array = static_cast<DT_array*>(ptr);
        void **data = static_cast<void **>(array->data);
        
        if (in_str(array->type, compound_tokens)) {
            for (int i=0; i<array->virtual_size; ++i) {
                mark_obj(arena, arena_addr, data[i], type16, mark_bit);
                gc_list(arena, arena_addr, data[i], data_name_to_type[array->type], work_list, mark_bit);
            }
        }
        else if(!in_str(array->type, primary_data_tokens)) {
            for (int i=0; i<array->virtual_size; ++i)
                mark_obj(arena, arena_addr, data[i], type16, mark_bit);
        } 
    }
    if (root_type==uint16_t{8}) { // map 
        DT_map *map = static_cast<DT_map*>(ptr);
        bool is_value_compound = in_vec(data_name_to_type[map->val_type], compound_types);

        for (int i=0; i<map->capacity; ++i) {
            DT_map_node *node = map->nodes[i];
            while (node!=nullptr) {
                mark_obj(arena, arena_addr, node, type16, mark_bit);
                if(is_value_compound);
                    gc_list(arena, arena_addr, node, data_name_to_type[map->val_type], work_list, mark_bit);
                node = node->next;
            }
        }
    }
}


void mark_worklist_pointers(GC_Arena *arena, char *arena_addr, std::vector<GC_Node> &work_list, uint64_t mark_bit) {
    uint16_t type16;
    for (int i=0; i<work_list.size(); ++i) {
        GC_Node &node = work_list[i];
        mark_obj(arena, arena_addr, node.ptr, type16, mark_bit);

        TypeInfo *class_info = type_info[type16]; 
        if (class_info!=nullptr) {
            for (int ptr_i=0; ptr_i<class_info->pointers_count; ++ptr_i) {
                PtrInfo *ptr_info = &class_info->ptr_info[ptr_i];
                uint16_t offset = ptr_info->offset;
                uint16_t nested_type = ptr_info->type;

                void **slot = (void **)(static_cast<char*>(node.ptr)+offset);
                if(check_initialized_field(slot))
                    work_list.push_back(GC_Node(*slot, nested_type));
            }
        }
        gc_list(arena, arena_addr, node.ptr, type16, work_list, mark_bit);
    }
}


void check_roots_worklist(Scope_Struct *scope_struct, GC_Arena *arena, char *arena_addr, uint64_t mark_bit) {
    uint16_t type16;
    std::vector<GC_Node> work_list;

    for (int i=0; i<scope_struct->stack_top; ++i) {
        void *root_ptr = scope_struct->pointers_stack[i];
        mark_obj(arena, arena_addr, root_ptr, type16, mark_bit);


        TypeInfo *class_info = type_info[type16]; 
        if (class_info!=nullptr) {
            for (int ptr_i=0; ptr_i<class_info->pointers_count; ++ptr_i) {
                PtrInfo *ptr_info = &class_info->ptr_info[ptr_i];
                int offset = ptr_info->offset;
                uint16_t nested_type = ptr_info->type;

                void **slot = (void **)(static_cast<char*>(root_ptr)+offset);

                if(check_initialized_field(slot))
                    work_list.push_back(GC_Node(*slot, nested_type));
               
            }
        }

        gc_list(arena, arena_addr, root_ptr, type16, work_list, mark_bit);
    }
    mark_worklist_pointers(arena, arena_addr, work_list, mark_bit);
}



// sweep
void GC::Sweep(Scope_Struct *scope_struct) {
    // Reset_Pools(arena, mark_bit^1);

    int tid = scope_struct->thread_id;
    char *arena_addr = arena_base_addr[tid];

    check_roots_worklist(scope_struct, arena, arena_addr, mark_bit^1);

    CleanUp_Unused(tid); // Trigger clean_up functions
    allocations=0;
    size_occupied=0;
}



void GC::CleanUp_Unused(int tid) {
    int get_mask = mark_bit ? 1 : 0;


    for (int span_group=0; span_group<GC_obj_sizes; span_group++) {
        GC_Span *span_ST = nullptr, *free_span_ST = nullptr, *last_free = nullptr;
        
        for (const auto &span : arena->Spans[span_group]) {

            GC_span_traits* traits = span->traits;
            int obj_size = span->elem_size;
            int freed_slots = 0; 
            for (int idx=0; idx<traits->N; ++idx) {
                if (get_1(span->mark_bits, idx)==get_mask) { // free slot
                    freed_slots++; 
                    // Clean up pointer
                    uint16_t u_type = get_16_r12(span->type_metadata, idx);
                    if(u_type!=0) {
                        std::string obj_type = data_type_to_name[u_type]; 
                        if(u_type!=5&&type_info[u_type]==nullptr) { // Not str and not class
                            // std::cout << "Clean of " << u_type << "|" << obj_type << "\n";     
                            void *obj_addr = static_cast<char*>(span->span_address) + idx*obj_size;
                            clean_up_functions[obj_type](obj_addr, tid);
                        }
                    }
                    set_16_r12(span->type_metadata, idx, uint16_t{0}); // avoid cleaning twice
                }
            }

            if (freed_slots>0)
            std::cout << freed_slots << "/" << span->free_idx << "/" << span->N<< "\n";


            if (span->is_free) {
                uint64_t *mark_bits = span->mark_bits;
                for (int i=span->free_idx; i<span->N; ++i) {
                    set_1(mark_bits, i, mark_bit);
                }
            }

            // bool is_free = ((span->is_free && span->free_idx<span->N) ||\
            //                 (freed_slots==span->N));
            // if (is_free) {
            //     if(!last_free)
            //         last_free = span;
            //     span->next_span = free_span_ST;
            //     free_span_ST = span;
            // } else {
            //     span->next_span = span_ST;
            //     span_ST = span;
            // }

            span->is_free = is_free;
            span->cur_free = (char*)span->span_address;
            span->free_idx=0;
        }
        GC_Span *first_span = span_ST;
        // if (last_free) {
        //     last_free->next_span = span_ST;
        //     first_span = free_span_ST;
        // }
        arena->current_span[span_group] = first_span;
    }

    mark_bit ^= 1;
}
