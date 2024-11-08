#ifndef CAI_COMMON_H
#define CAI_COMMON_H



#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <limits.h>
#include <stdlib.h>


#ifdef _DEBUG

#  include <stdio.h>

/* NOTE: args will not evaluated in release build, except for expr */
#  define CAI_ASSERT(expr, ...)\
    do{\
        if (!(expr)) {\
            fprintf(stderr, "ASSERTION FAILED in '%s' on line %d: ", __FILE__, __LINE__);\
            fprintf(stderr, __VA_ARGS__);\
            abort();\
        }\
    }while(0)

/* NOTE: args will not be evaluated in release build */
#  define CAI_DEBUG_PRINT(...) fprintf(stderr, __VA_ARGS__)


#else

#  define CAI_ASSERT(expr, ...) (void)(expr) // expression might have effect
#  define CAI_DEBUG_PRINT(...) (void)0

#endif /* _DEBUG */



typedef double flt_t;
typedef size_t usize_t;
typedef struct Car_t Car_t;
#define BITS_IN_WORD (sizeof(uintptr_t)*CHAR_BIT)


#endif /* CAI_COMMON_H */


