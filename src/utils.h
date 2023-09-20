#ifndef CJSAI_UTILS_H
#define CJSAI_UTILS_H


#include "common.h"


void *utils_alloc(size_t nbytes);
#define utils_alloc_array(nelem, elem_size)\
    utils_alloc((elem_size)*(nelem))

void *utils_realloc(void *ptr, size_t nbytes);
#define utils_realloc_array(ptr, nelem, elem_size) \
    utils_realloc(ptr, (elem_size)*(nelem))

void utils_free(void *ptr);
#define utils_free_array(ptr) utils_free(ptr)


#endif /* CJSAI_UTILS_H_*/

