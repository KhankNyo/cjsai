

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


#if defined(__GNUC__) || defined(__clang__)
#  define PACKED __attribute__((packed))
#else
#  define PACKED
#endif /* */


static const char s_magic[8] = "ai.data";




typedef struct FileHeader_t
{
    uint8_t magic[8];
    uint64_t level_count;
    uint64_t offsets[];
    /* levels right after */
} PACKED FileHeader_t;

typedef struct fltarrHeader_t
{
    uint64_t count;
    flt_t elems[];
} PACKED fltarrHeader_t;


typedef struct LevelHeader_t
{
    uint32_t input;
    uint32_t output;
    uint32_t weight_count;
    uint32_t weight_offset;
    /* bias data */
    /* weights fltarr */
} PACKED LevelHeader_t;

typedef union HeaderInterp_t
{
    uint8_t *u8;
    FileHeader_t *header;
} PACKED HeaderInterp_t;



typedef struct RawData_t
{
    HeaderInterp_t ptr;
    uint64_t nbytes, capacity;
} RawData_t;


/* data format:
 *  level_count (u64)
 *      level1 offset (u64)
 *      level2 offset (u64)
 *      ...
 *  level1:
 *      input_count (u32)
 *      output_count (u32)
 *      weight count (u32)
 *      weight data offset (u32)
 *          bias data (fltarr)
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


static void free_data(RawData_t data);
static int64_t fwrite_le(const void *buf, size_t elem_size, size_t elem_count, FILE *f);

static RawData_t data_from_nn(const NeuralNet_t nn);
static bool data_from_file(RawData_t *data, const char *filename);
static void data_write_fltarr(RawData_t *data, const fltarr_t array);
static void data_write_level(RawData_t *data, const Level_t level);

static void data_read_level(Level_t *level, const LevelHeader_t *header);
static uint64_t data_read_fltarr(fltarr_t *arr, const fltarrHeader_t *header);
static SaverStatus_t save_to_file(const RawData_t data, const char *filename);



SaverStatus_t Saver_SaveFile(const char *filename, const NeuralNet_t nn)
{
    RawData_t data = data_from_nn(nn);
    SaverStatus_t status = save_to_file(data, filename);
    free_data(data);
    return status;
}


bool Saver_LoadSave(NeuralNet_t *nn, const char *filename)
{
    RawData_t data;
    if (!data_from_file(&data, filename))
        return false;

    /* get the architecture of the neural network */
    uint64_t level_count = data.ptr.header->level_count;
    usize_t *levels = MEM_ALLOCA_ARRAY(level_count, sizeof(levels[0]));
    for (uint64_t i = 0; i < level_count - 1; i++)
    {
        uint64_t offset = data.ptr.header->offsets[i];
        const HeaderInterp_t ptr = data.ptr;

        /* get input and output count of a level */
        uint32_t icount, ocount;
        memcpy(&icount, &ptr.u8[offset], sizeof(icount));
        memcpy(&ocount, &ptr.u8[offset + sizeof(icount)], sizeof(ocount));

        levels[i] = icount;
        levels[i + 1] = ocount;
    }
    const NNArch_t arch = {
        .levels = levels,
        .count = level_count,
        .capacity = level_count,
    };
    *nn = NeuralNet_Init(arch, false);


    /* copy values from the save file */
    for (uint64_t i = 0; i < level_count - 1; i++)
    {
        uint64_t offset = data.ptr.header->offsets[i];
        const LevelHeader_t *header = (LevelHeader_t*)&data.ptr.u8[offset];
        data_read_level(&nn->levels[i], header);
    }


    free_data(data);
    return true;
}









static int64_t fwrite_le(const void *buf, size_t elem_size, size_t elem_count, FILE *f)
{
    return fwrite(buf, elem_size, elem_count, f);
}




static RawData_t data_from_nn(const NeuralNet_t nn)
{
    RawData_t data = { 0 };
    const uint64_t total_size = sizeof(FileHeader_t) 
        + (nn.count + 1) * sizeof(data.ptr.header->offsets[0]);

    /* file header first */
    data.ptr.u8 = mem_alloc(total_size);
    data.ptr.header->level_count = nn.count + 1;


    /* magic constant */
    memcpy(&data.ptr.header->magic, s_magic, sizeof s_magic);
    data.nbytes = total_size;


    /* contents */
    for (uint64_t i = 0; i < nn.count; i++)
    {
        /* write the level's offset to the offset table */
        data.ptr.header->offsets[i] = data.nbytes;

        /* write the level's data */
        data_write_level(&data, nn.levels[i]);
    }

    return data;
}


static bool data_from_file(RawData_t *data, const char *filename)
{
    FILE *save = fopen(filename, "rb");
    if (NULL == save)
        return false;

    fseek(save, 0, SEEK_END);
    data->nbytes = ftell(save);
    fseek(save, 0, SEEK_SET);
    data->capacity = data->nbytes;

    data->ptr.u8 = mem_alloc(data->nbytes);
    if (data->nbytes != fread(data->ptr.u8, 1, data->nbytes, save))
    {
        fclose(save);
        return false;
    }

    fclose(save);
    return true;
}




static void free_data(RawData_t data)
{
    mem_free(data.ptr.u8);
}



static SaverStatus_t save_to_file(const RawData_t data, const char *filename)
{
    SaverStatus_t status = SAVE_NEW_FILE;
    if (access(filename, F_OK) == 0)
        status = SAVE_OLD_FILE;

    FILE *savefile = fopen(filename, "wb+");
    if (NULL == savefile)
        return SAVE_FAILED;


    if (data.nbytes != (usize_t)fwrite_le(data.ptr.u8, 1, data.nbytes, savefile))
        status = SAVE_FAILED;


    fclose(savefile);
    return status;
}

static void data_write_fltarr(RawData_t *data, const fltarr_t array)
{
    const uint64_t arrcount = array.count;
    const uint64_t newsize = data->nbytes
        + arrcount * sizeof(array.at[0]) 
        + sizeof arrcount;
    if (newsize > data->capacity)
    {
        data->capacity = MEM_GROW_CAPACITY(newsize);
        data->ptr.u8 = MEM_REALLOC_ARRAY(data->ptr.u8, data->capacity, sizeof(data->ptr.u8[0]));
    }

    /* copy the array count */
    uint8_t *curr = &data->ptr.u8[data->nbytes];
    memcpy(curr, &arrcount, sizeof arrcount);

    /* now skip over and copy array data */
    curr += sizeof arrcount;
    memcpy(curr, array.at, array.count * sizeof(array.at[0]));
    data->nbytes = newsize;
}



static void data_write_level(RawData_t *data, const Level_t level)
{
    LevelHeader_t header = {
        .input = level.inputs.count,
        .output = level.output_count,
        .weight_count = level.weight_count,
        .weight_offset = 0,
    };
    const uint64_t newsize = data->nbytes + sizeof(header);
    if (newsize > data->capacity)
    {
        data->capacity = MEM_GROW_CAPACITY(newsize);
        data->ptr.u8 = MEM_REALLOC_ARRAY(data->ptr.u8, data->capacity, sizeof(data->ptr.u8[0]));
    }

    /* write input and output count */
    uint8_t *curr = &data->ptr.u8[data->nbytes];
    memcpy(curr, &header, sizeof header);
    uint64_t header_begin = data->nbytes; /* to the beginning of the header */
    data->nbytes = newsize;

    /* biases */
    data_write_fltarr(data, level.biases);

    /* weight data offset */
    curr = &data->ptr.u8[header_begin];
    ((LevelHeader_t*)curr)->weight_offset = 
        data->nbytes - (header_begin + sizeof(header));


    /* weights */
    for (usize_t k = 0; k < level.weight_count; k++)
    {
        data_write_fltarr(data, level.weights[k]);
    }
}


static void data_read_level(Level_t *level, const LevelHeader_t *header)
{
    /* appease strict aliasing, hopefully */
    union {
        fltarrHeader_t *fltarr;    
        uint8_t *u8;
    } ptr = {.u8 = (uint8_t*)header + sizeof(*header)};

    /* copy elems */
    ptr.u8 += data_read_fltarr(&level->biases, ptr.fltarr);

    /* skip to weights data */
    for (uint64_t i = 0; i < header->weight_count; i++)
    {
        ptr.u8 += data_read_fltarr(&level->weights[i], ptr.fltarr);
    }
}


static uint64_t data_read_fltarr(fltarr_t *arr, const fltarrHeader_t *header)
{
    uint64_t size = arr->count;
    if (header->count < size)
        size = header->count;

    memcpy(arr->at, header->elems, size * sizeof(arr->at[0]));
    return header->count * sizeof(arr->at[0]) + sizeof(*header);
}

