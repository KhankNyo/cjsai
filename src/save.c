

#include <stdio.h>
#include <string.h>

#include "include/mem.h"
#include "include/utils.h"
#include "include/save.h"




typedef struct RawData_t
{
    void *ptr;
    usize_t count, capacity;
} RawData_t;

typedef struct FileHeader_t
{
    uint32_t level_count;
} FileHeader_t;

typedef struct fltarrHeader_t
{
    uint32_t count;
} fltarrHeader_t;



int64_t fwrite_le(const void *buf, size_t elem_size, size_t elem_count, FILE *f);
usize_t fsize_from_nn(const NeuralNet_t nn);
RawData_t data_from_nn(const NeuralNet_t nn);
void free_data(RawData_t data);


bool Saver_SaveNN(const char *filename, const NeuralNet_t nn)
{
    RawData_t data = data_from_nn(nn);

    FILE *fsave = fopen(filename, "wb+");
    if (0 > fwrite_le(data.ptr, 1, data.count, fsave))
        goto fail;


    free_data(data);
    fclose(fsave);
    return true;
fail:
    free_data(data);
    fclose(fsave);
    return false;
}







int64_t fwrite_le(const void *buf, size_t elem_size, size_t elem_count, FILE *f)
{
    return fwrite(buf, elem_size, elem_count, f);
}




RawData_t data_from_nn(const NeuralNet_t nn)
{
    RawData_t data = {
        .ptr = mem_alloc(sizeof(FileHeader_t)),
        .count = 0,
        .capacity = sizeof(FileHeader_t)
    };

    memcpy(data.ptr, &(FileHeader_t){.level_count = nn.count}, 4);

}


void free_data(RawData_t data)
{

}




