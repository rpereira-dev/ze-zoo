# include <assert.h>
# include <sched.h>
# include <stdlib.h>
# include <string.h>
# include <ze_api.h>
# include "logger-ze.h"

// number of concurrent copies
# define N_CONCURRENT_H2D 3

// 2d region size
# define REGION_SX 512
# define REGION_SY 512

// type of the data copied
# define TYPE float

int
main(void)
{
    ze_init_flag_t initFlags = ZE_INIT_FLAG_GPU_ONLY;
    ZE_SAFE_CALL(zeInit(initFlags));

    //////////////
    //  INIT    //
    //////////////

    LOGGER_DEBUG("Init");

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
    ze_command_list_handle_t list;
    ZE_SAFE_CALL(zeCommandListCreateImmediate(context, device, &queueDesc, &list));

    // events pool
    ze_event_pool_handle_t pool;
    const ze_event_pool_desc_t poolDesc = {
        .stype  = ZE_STRUCTURE_TYPE_EVENT_POOL_DESC,
        .pNext  = NULL,
        .flags  = ZE_EVENT_POOL_FLAG_HOST_VISIBLE,
        .count  = N_CONCURRENT_H2D
    };
    const uint32_t ndevices = 1;
    ZE_SAFE_CALL(zeEventPoolCreate(context, &poolDesc, ndevices, &device, &pool));

    // events
    ze_event_handle_t events[N_CONCURRENT_H2D];
    for (int i = 0 ; i < N_CONCURRENT_H2D ; ++i)
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

    // allocate host memory
    const size_t n = N_CONCURRENT_H2D * REGION_SX * REGION_SY;
    const size_t size = n * sizeof(TYPE);
    TYPE * hst_mem = (TYPE *) malloc(size);
    assert(hst_mem);

    // write host memory
    for (size_t i = 0 ; i < n ; ++i)
        hst_mem[i] = i;

    // allocate device memory
    const ze_device_mem_alloc_desc_t deviceDesc = {
        .stype   = ZE_STRUCTURE_TYPE_DEVICE_MEMORY_PROPERTIES,
        .pNext   = NULL,
        .flags   = 0,
        .ordinal = 0,
    };
    const size_t alignment = REGION_SX * REGION_SY * sizeof(TYPE);
    TYPE * dev_mem;
    ZE_SAFE_CALL(zeMemAllocDevice(context, &deviceDesc, size, alignment, device, (void **) &dev_mem));
    ZE_SAFE_CALL(zeContextMakeMemoryResident(context, device, dev_mem, size));

    //////////////
    //  RUN     //
    //////////////

    LOGGER_DEBUG("Running");

    // if the i-th copy is done
    bool done[N_CONCURRENT_H2D];
    for (int i = 0 ; i < N_CONCURRENT_H2D ; ++i)
        done[i] = false;

    // Enqueue 2D copies in the immediate queue H2D
    for (int i = 0 ; i < N_CONCURRENT_H2D ; ++i)
    {
        ze_event_handle_t event = events[i];
        ZE_SAFE_CALL(zeEventHostReset(event));

        const uint32_t num_wait_events = 0;
        ze_event_handle_t * wait_events = NULL;

              void * dst    = (      void *) (dev_mem + i * REGION_SX * REGION_SY);
        const void * src    = (const void *) (hst_mem + i * REGION_SX * REGION_SY);

        const size_t dst_pitch = REGION_SX * sizeof(TYPE);
        const size_t src_pitch = REGION_SX * sizeof(TYPE);

        const size_t width  = REGION_SX * sizeof(TYPE);
        const size_t height = REGION_SY;

        const uint32_t dst_slice_pitch = 0;
        const ze_copy_region_t dst_region = {
            .originX = 0,
            .originY = 0,
            .originZ = 0,
            .width   = (uint32_t) width,
            .height  = (uint32_t) height,
            .depth   = 1
        };

        const uint32_t src_slice_pitch = 0;
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
    } /* launch */

    // Poll until completion
    int ndone = 0;
    while (ndone != N_CONCURRENT_H2D)
    {
        ndone = 0;
        for (int i = 0 ; i < N_CONCURRENT_H2D ; ++i)
        {
            if (done[i])
            {
                ++ndone;
                continue ;
            }

            ze_event_handle_t event = events[i];
            ze_result_t res;
            for (int j = 0 ; j < 16 ; ++j)
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

    // Set host memory to 0
    memset(hst_mem, 0, size);

    // Retrieve memory from device, for the last block
    size_t i = N_CONCURRENT_H2D - 1;
    ze_event_handle_t event = events[i];
    ZE_SAFE_CALL(zeEventHostReset(event));
    {
        const uint32_t num_wait_events = 0;
        ze_event_handle_t * wait_events = NULL;

              void * dst    = (      void *) (hst_mem + i * REGION_SX * REGION_SY);
        const void * src    = (const void *) (dev_mem + i * REGION_SX * REGION_SY);

        const size_t dst_pitch = REGION_SX * sizeof(TYPE);
        const size_t src_pitch = REGION_SX * sizeof(TYPE);

        const size_t width  = REGION_SX * sizeof(TYPE);
        const size_t height = REGION_SY;

        const uint32_t dst_slice_pitch = 0;
        const ze_copy_region_t dst_region = {
            .originX = 0,
            .originY = 0,
            .originZ = 0,
            .width   = (uint32_t) width,
            .height  = (uint32_t) height,
            .depth   = 1
        };

        const uint32_t src_slice_pitch = 0;
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
    } /* launch */

    // Poll until completion
    while (1)
    {
        ze_result_t res = zeEventQueryStatus(event);
        if (res == ZE_RESULT_NOT_READY)
            sched_yield();
        else if (res == ZE_RESULT_SUCCESS)
            break ;
        else
            ZE_SAFE_CALL(res);
    }

    // Test correctness
    for (size_t j = i * REGION_SX * REGION_SY ; j < (i+1) * REGION_SX * REGION_SY ; ++j)
    {
        if (hst_mem[j] != j)
            LOGGER_FATAL("Error with copy");
    }

    //////////////
    //  DEINIT  //
    //////////////

    LOGGER_DEBUG("Deinit");

    // release host memory
    free(hst_mem);

    // release device memory
    ZE_SAFE_CALL(zeMemFree(context, dev_mem));

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
