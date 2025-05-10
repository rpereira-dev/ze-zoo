# include <assert.h>
# include <sched.h>
# include <stdlib.h>
# include <string.h>
# include <ze_api.h>
# include <zes_api.h>
# include "logger-ze.h"

# define ZE_MAX_DRIVERS     4
# define ZE_MAX_DEVICES     32
# define ZE_MAX_MEMORIES    4

static uint32_t             zes_n_drivers;
static zes_driver_handle_t  zes_drivers[ZE_MAX_DRIVERS];
static uint32_t             zes_n_devices[ZE_MAX_DRIVERS];
static zes_device_handle_t  zes_devices[ZE_MAX_DRIVERS][ZE_MAX_DEVICES];
static uint32_t             zes_device_n_memories[ZE_MAX_DRIVERS][ZE_MAX_DEVICES];
static zes_mem_handle_t     zes_device_memories[ZE_MAX_DRIVERS][ZE_MAX_DEVICES][ZE_MAX_MEMORIES];

int
main(void)
{
    //////////////
    //  INIT    //
    //////////////

    LOGGER_DEBUG("Initializing");
    zes_init_flags_t zes_flags = ZES_INIT_FLAG_PLACEHOLDER;
    ZE_SAFE_CALL(zesInit(zes_flags));

    // driver
    ZE_SAFE_CALL(zesDriverGet(&zes_n_drivers, NULL));
    assert(zes_n_drivers < ZE_MAX_DRIVERS);
    ZE_SAFE_CALL(zesDriverGet(&zes_n_drivers, zes_drivers));

    for (unsigned int zes_driver_id = 0 ; zes_driver_id < zes_n_drivers ; ++zes_driver_id)
    {
        // devices
        ZE_SAFE_CALL(zesDeviceGet(zes_drivers[zes_driver_id], &zes_n_devices[zes_driver_id], NULL));
        assert(zes_n_devices[zes_driver_id] < ZE_MAX_DEVICES);
        ZE_SAFE_CALL(zesDeviceGet(zes_drivers[zes_driver_id], &zes_n_devices[zes_driver_id], zes_devices[zes_driver_id]));

        // memories
        for (unsigned int zes_device_id = 0 ; zes_device_id < zes_n_devices[zes_driver_id] ; ++zes_device_id)
        {
            zes_device_handle_t zes_device = zes_devices[zes_driver_id][zes_device_id];

            ZE_SAFE_CALL(zesDeviceEnumMemoryModules(zes_device, &zes_device_n_memories[zes_driver_id][zes_device_id], nullptr));
            assert(zes_device_n_memories[zes_driver_id][zes_device_id] < ZE_MAX_MEMORIES);
            ZE_SAFE_CALL(zesDeviceEnumMemoryModules(zes_device, &zes_device_n_memories[zes_driver_id][zes_device_id], zes_device_memories[zes_driver_id][zes_device_id]));
        }
    }

    //////////////
    //  RUN     //
    //////////////

    LOGGER_DEBUG("Running");
    while (1)
    {
        for (unsigned int zes_driver_id = 0 ; zes_driver_id < zes_n_drivers ; ++zes_driver_id)
        {
            LOGGER_INFO("#################################################");
            LOGGER_INFO("Driver `%u`", zes_driver_id);
            for (unsigned int zes_device_id = 0 ; zes_device_id < zes_n_devices[zes_driver_id] ; ++zes_device_id)
            {
                LOGGER_INFO("-------------------------------------------------");
                LOGGER_INFO("Device `%u`", zes_device_id);

                for (unsigned int zes_device_memory_id = 0 ; zes_device_memory_id  < zes_device_n_memories[zes_driver_id][zes_device_id] ; ++zes_device_memory_id)
                {
                    zes_mem_handle_t memory = zes_device_memories[zes_driver_id][zes_device_id][zes_device_memory_id];
                    zes_mem_state_t state = {
                        .stype = ZES_STRUCTURE_TYPE_MEM_STATE,
                        .pNext = NULL,
                        .health = ZES_MEM_HEALTH_UNKNOWN,
                        .free = 0,
                        .size = 0,
                    };
                    ZE_SAFE_CALL(zesMemoryGetState(memory, &state));

                    size_t used     = state.size - state.free;
                    size_t capacity = state.size;
                    LOGGER_INFO("Memory `%u` : %lu/%lu", zes_device_memory_id, used, capacity);
                }
            }
        }
        usleep(1000000);
    }

    //////////////
    //  DEINIT  //
    //////////////

    LOGGER_DEBUG("Deinit");
    // TODO

    return 0;
}
