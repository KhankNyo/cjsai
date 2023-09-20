
#include "common.h"
#include "utils.h"

#include <stdlib.h>
#include <stdio.h>





void *utils_alloc(size_t nbytes)
{
    void *ptr = malloc(nbytes);
    if (NULL == ptr)
    {
        fprintf(stderr, "Out of memory\n");
        exit(EXIT_FAILURE);
    }
    return ptr;
}



void *utils_realloc(void *ptr, size_t nbytes)
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



void utils_free(void *ptr)
{
    free(ptr);
}


