

#include <stdio.h>
#include <string.h>

#include "include/mem.h"
#include "include/utils.h"
#include "include/save.h"

#ifdef _WIN32
#  include <io.h>
#  define F_OK 0
#  define W_OK 2
#  define access _access
#else
#  include <unistd.h>
#endif /* _WIN32 */




typedef struct RawData_t
{
    uint8_t *ptr;
    usize_t count /* count in bytes, != numlevels */, capacity;
} RawData_t;

/* data format:
 *  num levels (u32)
 *  level1:
 *      bias data (fltarr)
 *      weight count (u32)
 *          weight1 data (fltarr)
 *          weight2 data (fltarr)
 *          ... 
 *  level2:
 *      ...
 *-----------------------------------*
 *  fltarr:
 *      count (u32)
 *      data (float32)
 *      ...
 */

typedef struct FileHeader_t
{
    uint32_t level_count;
    /* levels right after */
} FileHeader_t;

typedef struct fltarrHeader_t
{
    uint32_t count;
    /* elems of fltarr right after */
} fltarrHeader_t;



static void free_data(RawData_t data);


static int64_t fwrite_le(const void *buf, size_t elem_size, size_t elem_count, FILE *f);
static RawData_t data_from_nn(const NeuralNet_t nn);
static void data_write_fltarr(RawData_t *data, const fltarr_t array);
static SaverStatus_t save_to_file(const RawData_t data, const char *filename);



SaverStatus_t Saver_SaveNN(const char *filename, const NeuralNet_t nn)
{
    RawData_t data = data_from_nn(nn);
    SaverStatus_t status = save_to_file(data, filename);
    free_data(data);
    return status;
}


NeuralNet_t Saver_LoadSave(const char *filename)
{
    RawData_t data = data_from_file(filename);
    for ()
    {
        for ()
        {
        }
    }
    NeuralNet_t nn = NeuralNet_Init();
}









static int64_t fwrite_le(const void *buf, size_t elem_size, size_t elem_count, FILE *f)
{
    return fwrite(buf, elem_size, elem_count, f);
}




static RawData_t data_from_nn(const NeuralNet_t nn)
{
    RawData_t data = {
        .ptr = mem_alloc(sizeof(FileHeader_t)),
        .count = 0,
        .capacity = sizeof(FileHeader_t)
    };

    /* file header first */
    memcpy(data.ptr, &(FileHeader_t){.level_count = nn.count}, 4);
    data.count += sizeof(FileHeader_t);

    for (usize_t i = 0; i < nn.count; i++)
    {
        /* biases */
        data_write_fltarr(&data, nn.levels[i].biases);

        /* weights */
        for (usize_t k = 0; k < nn.levels[i].weight_count; k++)
        {
            data_write_fltarr(&data, nn.levels[i].weights[k]);
        }
    }

    return data;
}


static void free_data(RawData_t data)
{
    MEM_FREE_ARRAY(data.ptr);
}



static SaverStatus_t save_to_file(const RawData_t data, const char *filename)
{
    SaverStatus_t status = SAVE_NEW_FILE;
    if (access(filename, F_OK) == 0)
        status = SAVE_OLD_FILE;

    FILE *savefile = fopen(filename, "wb+");
    if (NULL == savefile)
        return SAVE_FAILED;

    if (data.count != (usize_t)fwrite_le(data.ptr, 1, data.count, savefile))
        status = SAVE_FAILED;

    fclose(savefile);
    return status;
}

static void data_write_fltarr(RawData_t *data, const fltarr_t array)
{
    const uint32_t arrcount = array.count;
    const usize_t newsize = data->count 
        + arrcount * sizeof(array.at[0]) 
        + sizeof arrcount;
    if (newsize > data->capacity)
    {
        data->capacity = MEM_GROW_CAPACITY(data->capacity);
        data->ptr = MEM_REALLOC_ARRAY(data->ptr, data->capacity, 1);
    }

    uint8_t *curr = &data->ptr[data->count];
    memcpy(curr, &arrcount, sizeof arrcount);
    memcpy(curr + arrcount, array.at, array.count * sizeof(array.at[0]));
    data->count = newsize;
}
