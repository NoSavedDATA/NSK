#include <unistd.h>

#include "../codegen/random.h"

#include "../compiler_frontend/logging_v.h"
#include "../mangler/scope_struct.h"
#include "../pool/include.h"
#include "array.h"
#include "list.h"
#include "include.h"

DT_array::DT_array() {}


void DT_array::New(int size, int elem_size, int tid, std::string type) {
    this->virtual_size = size;
    this->elem_size = elem_size;
    this->type = type;

    size = ((size + 7) / 8)*8;
    this->size = size;
    // data = (void*)malloc(size*elem_size);
    data = cache_pop(size*elem_size, tid);
}

void DT_array::New(int size, int tid, std::string type) {
    this->virtual_size = size;
    this->elem_size = 8;
    this->type = type;

    size = ((size + 7) / 8)*8;
    this->size = size;

    
    // data = (void*)malloc(size*8);
    data = cache_pop(size*elem_size, tid);
}


extern "C" DT_array *array_Create(Scope_Struct *scope_struct, Data_Tree *dt)
{ 
  std::string elem_type = dt->Nested_Data[0].Type;
  int elem_size;
  if(data_name_to_size.count(elem_type)>0)
      elem_size = data_name_to_size[elem_type];
  else
      elem_size = 8;


  DT_array *vec = newT<DT_array>(scope_struct, "array");
  vec->New(8, elem_size, scope_struct->thread_id, elem_type);
  vec->virtual_size = 0;
  return vec;
}

void array_Clean_Up(void *data_ptr, int tid) {
    DT_array *array = static_cast<DT_array *>(data_ptr);
    // free(array->data);
    cache_push(array->data, array->size, tid);
}

extern "C" int array_size(Scope_Struct *scope_struct, DT_array *vec) {
    return vec->virtual_size;
}



extern "C" int array_bad_idx(int line, int idx, int size) {
    
    LogErrorC(line, "Tried to index array at " + std::to_string(idx) + ", but the array size is: " + std::to_string(size));
}



extern "C" void array_double_size(Scope_Struct *scope_struct, DT_array *vec, int new_size) {
    // vec->data 
    int tid = scope_struct->thread_id;
    int old_size = vec->virtual_size*vec->elem_size;
    int vec_size = new_size         *vec->elem_size;

    // void *new_data = (void*)malloc(vec_size);
    void *new_data = cache_pop(vec_size, tid);
    memcpy(new_data, vec->data, old_size);

    // free(vec->data);
    cache_push(vec->data, old_size, tid);
    vec->data = new_data;
    vec->size = new_size;
    vec->virtual_size++;
}

extern "C" void array_print_int(Scope_Struct *scope_struct, DT_array *vec) {
    int *ptr = static_cast<int*>(vec->data);
    int size = vec->virtual_size;

    std::cout << "[";
    for (int i=0; i<size-1; ++i)
        std::cout << ptr[i] << ",";
    std::cout << ptr[size-1] << "]\n";
}


extern "C" DT_array *arange_int(Scope_Struct *scope_struct, int begin, int end) {
    DT_array *vec = newT<DT_array>(scope_struct, "array");
    vec->New(end-begin, 4, scope_struct->thread_id, "int");

    int *ptr = static_cast<int*>(vec->data);
    
    int c=0;
    for(int i=begin; i<end; ++i)
    {
        ptr[c] = i;
        c++;
    }

    return vec; 
} 


extern "C" DT_array *zeros_int(Scope_Struct *scope_struct, int N) {
    DT_array *vec = newT<DT_array>(scope_struct, "array");
    vec->New(N, 4, scope_struct->thread_id, "int");

    int *ptr = static_cast<int*>(vec->data);
    
    int c=0;
    for(int i=0; i<N; ++i)
    {
        ptr[c] = 0;
        c++;
    }

    return vec; 
} 



extern "C" DT_array *randint_array(Scope_Struct *scope_struct, int size, int min_val, int max_val) {
    // std::cout << "new array " << scope_struct << "|" << size << " " << min_val << " " << max_val << "\n";
    DT_array *vec = newT<DT_array>(scope_struct, "array");
    vec->New(size,4, scope_struct->thread_id, "int");

    std::uniform_int_distribution<int> dist(min_val, max_val);

    int *ptr = static_cast<int*>(vec->data);
    for (int i = 0; i < size; ++i) {
        int r;
        {
            std::lock_guard<std::mutex> lock(MAIN_PRNG_MUTEX);
            r = dist(MAIN_PRNG);
        }
        ptr[i] = r;
    }

    return vec;
}


extern "C" DT_array *ones_int(Scope_Struct *scope_struct, int N) {
    DT_array *vec = newT<DT_array>(scope_struct, "array");
    vec->New(N, 4, scope_struct->thread_id, "int");

    int *ptr = static_cast<int*>(vec->data);
    
    int c=0;
    for(int i=0; i<N; ++i)
    {
        ptr[c] = 1;
        c++;
    }

    return vec;
}

extern "C" DT_array *array_int_add(Scope_Struct *scope_struct, DT_array *array, int x) {
    DT_array *new_array = newT<DT_array>(scope_struct, "array");
    new_array->New(array->virtual_size, 4, scope_struct->thread_id, "int");
    
    int *data = static_cast<int *>(array->data);
    int *new_data = static_cast<int *>(new_array->data);
    for (int i=0; i<array->virtual_size; ++i) {
        new_data[i] = data[i] + x;
    }

    return new_array;
}



extern "C" DT_array *randfloat_array(Scope_Struct *scope_struct, int size, float min_val, float max_val) {
    DT_array *vec = newT<DT_array>(scope_struct, "array");
    vec->New(size,4, scope_struct->thread_id,"float");

    std::uniform_real_distribution<float> dist(min_val, max_val);

    float *ptr = static_cast<float*>(vec->data);
    for (int i = 0; i < size; ++i) {
        float r;
        {
            std::lock_guard<std::mutex> lock(MAIN_PRNG_MUTEX);
            r = dist(MAIN_PRNG);
        }
        ptr[i] = r;
    }

    return vec;
}

extern "C" int array_print_float(Scope_Struct *scope_struct, DT_array *vec) {
    float *ptr = static_cast<float*>(vec->data);
    int size = vec->virtual_size;

    std::cout << "[";
    for (int i=0; i<size-1; ++i)
        printf("%.3f, ",ptr[i]);
    printf("%.3f]\n",ptr[size-1]);
    return 0;
}


extern "C" DT_array *arange_float(Scope_Struct *scope_struct, float begin, float end) {
    DT_array *vec = newT<DT_array>(scope_struct, "float_vec");
    vec->New(end-begin, 4, scope_struct->thread_id, "float");

    float *ptr = static_cast<float*>(vec->data);
    
    int c=0;
    for(int i=begin; i<end; ++i)
    {
        ptr[c] = i;
        c++;
    }

    return vec; 
} 

extern "C" DT_array *zeros_float(Scope_Struct *scope_struct, int N) {
    DT_array *vec = newT<DT_array>(scope_struct, "array");
    vec->New(N, 4, scope_struct->thread_id, "float");

    float *ptr = static_cast<float*>(vec->data);
    
    int c=0;
    for(int i=0; i<N; ++i)
    {
        ptr[c] = 0.0f;
        c++;
    }

    return vec; 
}


extern "C" DT_array *ones_float(Scope_Struct *scope_struct, int N) {
    DT_array *vec = newT<DT_array>(scope_struct, "array");
    vec->New(N, 4, scope_struct->thread_id, "float");

    float *ptr = static_cast<float*>(vec->data);
    
    int c=0;
    for(int i=0; i<N; ++i)
    {
        ptr[c] = 1.0f;
        c++;
    }

    return vec; 
}





extern "C" DT_array *array_Split_Parallel(Scope_Struct *scope_struct, DT_array *vec) {
    int threads_count = scope_struct->asyncs_count;
    int thread_id = scope_struct->thread_id-1;

    int vec_size = vec->virtual_size;
    int elem_size = vec->elem_size;
    int segment_size;

    segment_size = ceilf(vec_size/(float)threads_count);


    int size = segment_size;
    if((thread_id+1)==threads_count)
      size = vec_size - segment_size*thread_id;
      

    int copy_size;
    if(segment_size*(thread_id+1)>vec_size) 
        copy_size = (vec_size - segment_size*thread_id)*elem_size;
    else
        copy_size = segment_size*elem_size;


    DT_array *out_vector = newT<DT_array>(scope_struct, "array");
    out_vector->New(size, elem_size, scope_struct->thread_id, vec->type);

    
    memcpy(out_vector->data,
           static_cast<char*>(vec->data) + segment_size*thread_id*elem_size,
           copy_size);

    return out_vector;
}


extern "C" int array_print_str(Scope_Struct *scope_struct, DT_array *arr) {
    char **data = static_cast<char**>(arr->data);
    int len = arr->virtual_size;
    std::cout << data[0];
    for (int i=1; i<len; ++i)
        std::cout << ", " << data[i];
    std::cout << "\n";
    return 0;
}

extern "C" int array_print_str_view(Scope_Struct *scope_struct, DT_array *arr) {
    DT_str *data = static_cast<DT_str*>(arr->data);
    int len = arr->virtual_size;

    int offset = 0, elem_size=data[0].size;
    memcpy(scope_struct->print_buffer, data[0].str, elem_size);
    write(1, scope_struct->print_buffer, elem_size);
    for (int i = 1; i < len; ++i) {
        scope_struct->print_buffer[0] = ',';
        write(1, scope_struct->print_buffer, 1);

        elem_size = data[i].size;
        memcpy(scope_struct->print_buffer, data[i].str, elem_size);
        write(1, scope_struct->print_buffer, elem_size);
    }
    return 0;
}
