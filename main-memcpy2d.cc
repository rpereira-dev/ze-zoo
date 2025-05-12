/**
 *
 *  Host memory has continuous tiles
 *
 *  ---------------------------------------------
 *  |          |          |          |          |
 *  |  T0      |   T1     |   T2     |   T3     |
 *  |          |          |          |          |
 *  |          |          |          |          |
 *  ---------------------------------------------
 *
 *  Device memory has non-continuous tiles
 *
 *  ------------              ------------
 *  |          |              |          |
 *  |  T0      |     [...]    |   T3     |
 *  |          |              |          |
 *  |          |              |          |
 *  ------------              ------------
 *
 *  The benchmark:
 *      - set host memory bytes[v] to the value 'v'
 *      - copies host tile 'i' to device tile 'i'
 *      - waits for all copies completion
 *      - set host memory to '0'
 *      - copies device tile 'i' to host tile 'i'
 *      - check that host memory bytes[v] == v
 */

# include <assert.h>
# include <sched.h>
# include <stdlib.h>
# include <string.h>
# include <ze_api.h>
# include "logger-ze.h"

// 2d region size
# define REGION_SX 512
# define REGION_SY 512

// type of the data copied
# define TYPE float

// maximum number of tiles
# define N_TILES_MAX 64

// number of tiles
static unsigned int N_TILES;

// ze events for each tiles
static ze_event_handle_t events[N_TILES_MAX];

// ze command list
static ze_command_list_handle_t list;

// wait for each tile
static void
wait(void)
{
    // if the i-th copy is done
    bool done[N_TILES_MAX];
    for (unsigned int i = 0 ; i < N_TILES ; ++i)
        done[i] = false;

    unsigned int ndone = 0;
    while (ndone != N_TILES)
    {
        ndone = 0;
        for (unsigned int i = 0 ; i < N_TILES ; ++i)
        {
            if (done[i])
            {
                ++ndone;
                continue ;
            }

            ze_event_handle_t event = events[i];
            ze_result_t res;
            for (unsigned int j = 0 ; j < 16 ; ++j)
            {
                res = zeEventQueryStatus(event);
                if (res == ZE_RESULT_NOT_READY)
                    sched_yield();
                else if (res == ZE_RESULT_SUCCESS)
                {
                    done[i] = true;
                    break ;
                }
                else
                    ZE_SAFE_CALL(res);
            }
        }
    }
}

static void
copy(
    unsigned int i,
    void * dst, const void * src,
    const size_t dst_pitch, const size_t src_pitch,
    uint32_t width, uint32_t height
) {
    ze_event_handle_t event = events[i];
    ZE_SAFE_CALL(zeEventHostReset(event));

    const uint32_t num_wait_events = 0;
    ze_event_handle_t * wait_events = NULL;

    const uint32_t dst_slice_pitch = 0;
    const uint32_t src_slice_pitch = 0;

    const ze_copy_region_t dst_region = {
        .originX = 0,
        .originY = 0,
        .originZ = 0,
        .width   = (uint32_t) width,
        .height  = (uint32_t) height,
        .depth   = 1
    };

    const ze_copy_region_t src_region = {
        .originX = 0,
        .originY = 0,
        .originZ = 0,
        .width   = (uint32_t) width,
        .height  = (uint32_t) height,
        .depth   = 1
    };

    ZE_SAFE_CALL(
        zeCommandListAppendMemoryCopyRegion(
            list,
            dst,
           &dst_region,
            dst_pitch,
            dst_slice_pitch,
            src,
           &src_region,
            src_pitch,
            src_slice_pitch,
            event,
            num_wait_events,
            wait_events
        )
    );
}

int
main(int argc, char ** argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "usage: %s [NUMBER_OF_TILES]\n", argv[0]);
        return 1;
    }

    N_TILES = atoi(argv[1]);
    if (N_TILES > N_TILES_MAX)
        N_TILES = N_TILES_MAX;

    LOGGER_INFO("Configured with `%d` tiles", N_TILES);

    ze_init_flag_t initFlags = ZE_INIT_FLAG_GPU_ONLY;
    ZE_SAFE_CALL(zeInit(initFlags));

    //////////////
    //  INIT    //
    //////////////

    LOGGER_INFO("Init");

    // driver
    ze_driver_handle_t driver;
    uint32_t driverCount = 1;
    ZE_SAFE_CALL(zeDriverGet(&driverCount, &driver));

    // device
    ze_device_handle_t device;
    uint32_t deviceCount = 1;
    ZE_SAFE_CALL(zeDeviceGet(driver, &deviceCount, &device));

    // context
    ze_context_handle_t context;
    ze_context_desc_t contextDesc = {
        .stype = ZE_STRUCTURE_TYPE_CONTEXT_DESC,
        .pNext = NULL,
        .flags = ZE_CONTEXT_FLAG_TBD
    };
    ZE_SAFE_CALL(zeContextCreate(driver, &contextDesc, &context));

    // stream
    const int ordinal = 1;
    const int   index = 0;
    const ze_command_queue_desc_t queueDesc = {
        .stype      = ZE_STRUCTURE_TYPE_COMMAND_QUEUE_DESC,
        .pNext      = NULL,
        .ordinal    = ordinal,
        .index      = index,
        .flags      = ZE_COMMAND_QUEUE_FLAG_EXPLICIT_ONLY,
        .mode       = ZE_COMMAND_QUEUE_MODE_ASYNCHRONOUS,
        .priority   = ZE_COMMAND_QUEUE_PRIORITY_PRIORITY_LOW
    };
    ZE_SAFE_CALL(zeCommandListCreateImmediate(context, device, &queueDesc, &list));

    // events pool
    ze_event_pool_handle_t pool;
    const ze_event_pool_desc_t poolDesc = {
        .stype  = ZE_STRUCTURE_TYPE_EVENT_POOL_DESC,
        .pNext  = NULL,
        .flags  = ZE_EVENT_POOL_FLAG_HOST_VISIBLE,
        .count  = N_TILES
    };
    const uint32_t ndevices = 1;
    ZE_SAFE_CALL(zeEventPoolCreate(context, &poolDesc, ndevices, &device, &pool));

    // events
    for (unsigned int i = 0 ; i < N_TILES ; ++i)
    {
        ze_event_desc_t eventDesc = {
            .stype  = ZE_STRUCTURE_TYPE_EVENT_DESC,
            .pNext  = NULL,
            .index  = (uint32_t) i,
            .signal = ZE_EVENT_SCOPE_FLAG_HOST,
            .wait   = ZE_EVENT_SCOPE_FLAG_HOST,
        };
        ZE_SAFE_CALL(zeEventCreate(pool, &eventDesc, events + i));
    }

    const size_t n_one = REGION_SX * REGION_SY;
    const size_t n_all = N_TILES * n_one;
    const size_t size_one = n_one * sizeof(TYPE);
    const size_t size_all = n_all * sizeof(TYPE);

    // allocate host memory ( tiles are continuous )
    TYPE * hst_mem = (TYPE *) malloc(size_all);
    assert(hst_mem);

    // write host memory
    for (size_t i = 0 ; i < n_all ; ++i)
        hst_mem[i] = i;

    // allocate device memory ( 4 tiles, discontinuous )
    const ze_device_mem_alloc_desc_t deviceDesc = {
        .stype   = ZE_STRUCTURE_TYPE_DEVICE_MEMORY_PROPERTIES,
        .pNext   = NULL,
        .flags   = 0,
        .ordinal = 0,
    };
    const size_t alignment = REGION_SX * REGION_SY * sizeof(TYPE);
    TYPE * dev_mem[N_TILES_MAX];
    for (unsigned int i = 0 ; i < N_TILES ; ++i)
    {
        ZE_SAFE_CALL(zeMemAllocDevice(context, &deviceDesc, size_one, alignment, device, (void **) &dev_mem[i]));
        ZE_SAFE_CALL(zeContextMakeMemoryResident(context, device, dev_mem[i], size_one));
    }

    //////////////
    // H2D      //
    //////////////

    LOGGER_INFO("H2D");

    // Enqueue 2D copies in the immediate queue H2D
    for (unsigned int i = 0 ; i < N_TILES ; ++i)
    {
              void * dst    = (      void *) dev_mem[i];
        const void * src    = (const void *) (hst_mem + i * REGION_SX);

        const size_t dst_pitch =           REGION_SX * sizeof(TYPE);
        const size_t src_pitch = N_TILES * REGION_SX * sizeof(TYPE);

        const size_t width  = REGION_SX * sizeof(TYPE);
        const size_t height = REGION_SY;

        copy(i, dst, src, dst_pitch, src_pitch, width, height);

    } /* launch */

    // Poll until completion
    wait();

    //////////////
    // D2H then //
    //////////////

    // Set host memory to 0
    memset(hst_mem, 0, size_all);

    // D2H - Retrieve memory from device, for the last tile
    for (unsigned int i = 0 ; i < N_TILES ; ++i)
    {
              void * dst    = (      void *) (hst_mem + i * REGION_SX);
        const void * src    = (const void *) (dev_mem[i]);

        const size_t dst_pitch = N_TILES * REGION_SX * sizeof(TYPE);
        const size_t src_pitch =           REGION_SX * sizeof(TYPE);

        const size_t width  = REGION_SX * sizeof(TYPE);
        const size_t height = REGION_SY;

        copy(i, dst, src, dst_pitch, src_pitch, width, height);
    } /* launch */

    wait();

    //////////////////////
    // Test correctness //
    //////////////////////

    for (size_t j = 0 ; j < N_TILES * REGION_SX * REGION_SY ; ++j)
        if (hst_mem[j] != j)
            LOGGER_FATAL("FAILURE");
    LOGGER_INFO("SUCCESS");

    //////////////
    //  DEINIT  //
    //////////////

    LOGGER_INFO("Deinit");

    // release host memory
    free(hst_mem);

    // release device memory
    for (unsigned int i = 0 ; i < N_TILES ; ++i)
        ZE_SAFE_CALL(zeMemFree(context, dev_mem[i]));

    // command list
    ZE_SAFE_CALL(zeCommandListDestroy(list));

    // event pool
    ZE_SAFE_CALL(zeEventPoolDestroy(pool));

    // context
    ZE_SAFE_CALL(zeContextDestroy(context));

    // device ?

    // driver ?

    return 0;
}
