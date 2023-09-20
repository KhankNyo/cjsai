

#include <stdlib.h>
#include <stdio.h>

#include "include/common.h"
#include "include/utils.h"





void *mem_alloc(size_t nbytes)
{
    void *ptr = malloc(nbytes);
    if (NULL == ptr)
    {
        fprintf(stderr, "Out of memory\n");
        exit(EXIT_FAILURE);
    }
    return ptr;
}



void *mem_realloc(void *ptr, size_t nbytes)
{
    if (0 == nbytes)
    {
        return ptr;
    }

    void *newptr = realloc(ptr, nbytes);
    if (NULL == newptr)
    {
        fprintf(stderr, "Out of memory\n");
        exit(EXIT_FAILURE);
    }
    return newptr;
}



void mem_free(void *ptr)
{
    free(ptr);
}


