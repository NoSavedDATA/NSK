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
#include "../../mangler/scope_struct.h"
#include "../../pool/pool.h"
#include "../include.h"







void Reset_Pools(GC_Arena *arena) {
    for (int span_group=0; span_group<GC_obj_sizes; span_group++) {
        int span_id=0;
        for (const auto &span : arena->Spans[span_group]) {
            if(span_id==0)
                arena->current_span[span_group] = span;
            span->free_idx=0;
            GC_span_traits *traits = span->traits;
            for (int i=0; i<traits->N; ++i) {
                mark_bits_free(span->mark_bits, i);
                set_16_L1(span->type_metadata, i, 0u);
            }
            span_id++;
        }
    }
}

inline void check_is_in_bounds(char *arena_addr, char *p) {
    bool in_bounds = (p>=arena_addr&&p<arena_addr+GC_arena_size);
    if (!in_bounds) {
        std::cout << "Variable of type " << p << " address does not reside in any memory pool..\n";
        std::cout << p << ".\n";
        std::exit(0);
    }
}

inline void mark_obj(GC_Arena *arena, char *arena_addr, void *node_ptr) {
        char *p = static_cast<char*>(node_ptr);
        check_is_in_bounds(arena_addr, p);

        long arena_offset = p - arena_addr;
        int page  =  (arena_offset / GC_page_size) % pages_per_arena;
 
        GC_Span *span = arena->page_to_span[page];

        long obj_idx = (p - static_cast<char*>(span->span_address)) / span->traits->obj_size;

        set_16_L1(span->type_metadata, obj_idx, 1u);
}



inline void gc_list(GC_Arena *arena, char *arena_addr, void *ptr, const std::string &root_type, std::vector<GC_Node> &work_list) {
    if (root_type=="channel") {
        LogBlue("FROM ROOT OF CHANNEL");
    }
    if (root_type=="list") {
        DT_list *list = static_cast<DT_list*>(ptr);
        for (int i=0; i<list->size; ++i) {
            const char *type = list->data_types->at(i).c_str(); 
            if(!strcmp(type, "list")) {
                gc_list(arena, arena_addr, list->get<void*>(i), "list", work_list);
                continue;
            }
            if(!strcmp(type, "str")) {
                mark_obj(arena, arena_addr, list->get<char*>(i));
                continue;
            }
            if(strcmp(type, "int")&&strcmp(type, "float")&&strcmp(type, "bool")) // not a primary
                mark_obj(arena, arena_addr, list->get<void*>(i));
            //     work_list.push_back(GC_Node(list->get<void*>(i), type));
        }
    }
    if (root_type=="array") {
        DT_array *array = static_cast<DT_array*>(ptr);
        void **data = static_cast<void **>(array->data);
        
        if (in_str(array->type, compound_tokens)) {
            for (int i=0; i<array->virtual_size; ++i) {
                mark_obj(arena, arena_addr, data[i]);
                gc_list(arena, arena_addr, data[i], array->type, work_list);
            }
        }
        else if(!in_str(array->type, primary_data_tokens)) {
            for (int i=0; i<array->virtual_size; ++i)
                mark_obj(arena, arena_addr, data[i]);
        } 
    }
    if (root_type=="map") {
        DT_map *map = static_cast<DT_map*>(ptr);
        bool is_value_compound = in_str(map->val_type, compound_tokens);

        for (int i=0; i<map->capacity; ++i) {
            DT_map_node *node = map->nodes[i];
            while (node!=nullptr) {
                mark_obj(arena, arena_addr, node);
                if(is_value_compound);
                    gc_list(arena, arena_addr, node, map->val_type, work_list);
                node = node->next;
            }
        }
    }
}


void mark_worklist_pointers(GC_Arena *arena, char *arena_addr, std::vector<GC_Node> &work_list) {
    for (int i=0; i<work_list.size(); ++i) {
        GC_Node &node = work_list[i];
        mark_obj(arena, arena_addr, node.ptr);
        // std::cout << "push obj attr of type: " << node.type << "/" << node.ptr << ".\n";

        if (ClassPointers.count(node.type)>0) {
            for (int j=0; j<ClassPointers[node.type].size(); ++j) {
                int offset = ClassPointers[node.type][j];
                std::string type = ClassPointersType[node.type][j];

                void **slot = (void **)(static_cast<char*>(node.ptr)+offset);

                if(check_initialized_field(slot))
                    work_list.push_back(GC_Node(*slot, type));
            }
        }
        gc_list(arena, arena_addr, node.ptr, node.type, work_list);
    }
}


void check_roots_worklist(Scope_Struct *scope_struct, GC_Arena *arena, char *arena_addr) {
    std::vector<GC_Node> work_list;

    // std::cout << "stack top: " << scope_struct->stack_top << ".\n";
    for (int i=0; i<scope_struct->stack_top; ++i) {
        void *root_ptr = scope_struct->pointers_stack[i];
        mark_obj(arena, arena_addr, root_ptr);

        // std::cout << "PUSH BACK ROOT: " << i << "/" << scope_struct->stack_top << "/" <<  root_ptr << ".\n";

        std::string root_type = get_pool_obj_type(scope_struct, root_ptr);
        // std::cout << "PUSH BACK ROOT: " << root_type << "/" << root_ptr << ".\n";
        
        if (ClassPointers.count(root_type)>0) {
            for (int i=0; i<ClassPointers[root_type].size(); ++i) {
                int offset = ClassPointers[root_type][i];
                std::string type = ClassPointersType[root_type][i];
                
                void **slot = (void **)(static_cast<char*>(root_ptr)+offset);
                
                if(check_initialized_field(slot))
                    work_list.push_back(GC_Node(*slot, type));
            }
        }
        gc_list(arena, arena_addr, root_ptr, root_type, work_list);
    }
    mark_worklist_pointers(arena, arena_addr, work_list);
}



void GC::Sweep(Scope_Struct *scope_struct) {
    Reset_Pools(arena);

    int tid = scope_struct->thread_id;
    char *arena_addr = arena_base_addr[tid];

    check_roots_worklist(scope_struct, arena, arena_addr);

    CleanUp_Unused(tid); // Trigger clean_up functions
    // Reset_Free_Lists(arena);
    allocations=0;
    size_occupied=0;
}


void GC::CleanUp_Unused(int tid) {
    
    for (int span_group=0; span_group<arena->Spans.size(); span_group++) {
        for (const auto &span : arena->Spans[span_group]) {
            GC_span_traits* traits = span->traits;
            int obj_size = traits->obj_size;
            

            for (int idx=0; idx<traits->N; ++idx) {
                if (get_16_l2(span->type_metadata, idx)==0) { // free slot
                    // Clean up pointer
                    uint16_t u_type = get_16_r12(span->type_metadata, idx);
                    if(u_type!=0) {
                        std::string obj_type = data_type_to_name[u_type]; 
                        if(obj_type!="str"&&ClassSize.count(obj_type)==0) {
                            void *obj_addr = static_cast<char*>(span->span_address) + idx*obj_size;
                            // if (obj_type!="list"&&obj_type!="tensor")
                            // std::cout << "CLEANing: addr " << obj_addr << " got object: " << u_type << "/" << obj_type << ".\n";
                            clean_up_functions[obj_type](obj_addr, tid);
                        }
                    }
                }
            }
        }
    }
}
