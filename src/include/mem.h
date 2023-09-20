#ifndef CJSAI_MEM_H
#define CJSAI_MEM_H


#include "common.h"



#define MEM_GROW_CAPACITY(oldcap) \
    ((oldcap) < 8 ? 8 : (oldcap)*2)

void *mem_alloc(usize_t nbytes);
#define MEM_ALLOC_ARRAY(nelem, elem_size)\
    mem_alloc((elem_size)*(nelem))

void *mem_realloc(void *ptr, usize_t nbytes);
#define MEM_REALLOC_ARRAY(ptr, nelem, elem_size) \
    mem_realloc(ptr, (elem_size)*(nelem))

void mem_free(void *ptr);
#define MEM_FREE_ARRAY(ptr) mem_free(ptr)


#endif /* CJSAI_MEM_H_*/

