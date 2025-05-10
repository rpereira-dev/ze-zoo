# ifndef __LOGGER_ZE_H__
#  define __LOGGER_ZE_H__

#  include "logger.h"
#  include <ze_api.h>

static const char *
ze_error_to_str(const ze_result_t & r)
{
    switch (r)
    {
        case ZE_RESULT_SUCCESS:                                 return "ZE_RESULT_SUCCESS";
        case ZE_RESULT_NOT_READY:                               return "ZE_RESULT_NOT_READY";
        case ZE_RESULT_ERROR_DEVICE_LOST:                       return "ZE_RESULT_ERROR_DEVICE_LOST";
        case ZE_RESULT_ERROR_OUT_OF_HOST_MEMORY:                return "ZE_RESULT_ERROR_OUT_OF_HOST_MEMORY";
        case ZE_RESULT_ERROR_OUT_OF_DEVICE_MEMORY:              return "ZE_RESULT_ERROR_OUT_OF_DEVICE_MEMORY";
        case ZE_RESULT_ERROR_MODULE_BUILD_FAILURE:              return "ZE_RESULT_ERROR_MODULE_BUILD_FAILURE";
        case ZE_RESULT_ERROR_MODULE_LINK_FAILURE:               return "ZE_RESULT_ERROR_MODULE_LINK_FAILURE";
        case ZE_RESULT_ERROR_DEVICE_REQUIRES_RESET:             return "ZE_RESULT_ERROR_DEVICE_REQUIRES_RESET";
        case ZE_RESULT_ERROR_DEVICE_IN_LOW_POWER_STATE:         return "ZE_RESULT_ERROR_DEVICE_IN_LOW_POWER_STATE";
        case ZE_RESULT_EXP_ERROR_DEVICE_IS_NOT_VERTEX:          return "ZE_RESULT_EXP_ERROR_DEVICE_IS_NOT_VERTEX";
        case ZE_RESULT_EXP_ERROR_VERTEX_IS_NOT_DEVICE:          return "ZE_RESULT_EXP_ERROR_VERTEX_IS_NOT_DEVICE";
        case ZE_RESULT_EXP_ERROR_REMOTE_DEVICE:                 return "ZE_RESULT_EXP_ERROR_REMOTE_DEVICE";
        case ZE_RESULT_EXP_ERROR_OPERANDS_INCOMPATIBLE:         return "ZE_RESULT_EXP_ERROR_OPERANDS_INCOMPATIBLE";
        case ZE_RESULT_EXP_RTAS_BUILD_RETRY:                    return "ZE_RESULT_EXP_RTAS_BUILD_RETRY";
        case ZE_RESULT_EXP_RTAS_BUILD_DEFERRED:                 return "ZE_RESULT_EXP_RTAS_BUILD_DEFERRED";
        case ZE_RESULT_ERROR_INSUFFICIENT_PERMISSIONS:          return "ZE_RESULT_ERROR_INSUFFICIENT_PERMISSIONS";
        case ZE_RESULT_ERROR_NOT_AVAILABLE:                     return "ZE_RESULT_ERROR_NOT_AVAILABLE";
        case ZE_RESULT_ERROR_DEPENDENCY_UNAVAILABLE:            return "ZE_RESULT_ERROR_DEPENDENCY_UNAVAILABLE";
        case ZE_RESULT_WARNING_DROPPED_DATA:                    return "ZE_RESULT_WARNING_DROPPED_DATA";
        case ZE_RESULT_ERROR_UNINITIALIZED:                     return "ZE_RESULT_ERROR_UNINITIALIZED";
        case ZE_RESULT_ERROR_UNSUPPORTED_VERSION:               return "ZE_RESULT_ERROR_UNSUPPORTED_VERSION";
        case ZE_RESULT_ERROR_UNSUPPORTED_FEATURE:               return "ZE_RESULT_ERROR_UNSUPPORTED_FEATURE";
        case ZE_RESULT_ERROR_INVALID_ARGUMENT:                  return "ZE_RESULT_ERROR_INVALID_ARGUMENT";
        case ZE_RESULT_ERROR_INVALID_NULL_HANDLE:               return "ZE_RESULT_ERROR_INVALID_NULL_HANDLE";
        case ZE_RESULT_ERROR_HANDLE_OBJECT_IN_USE:              return "ZE_RESULT_ERROR_HANDLE_OBJECT_IN_USE";
        case ZE_RESULT_ERROR_INVALID_NULL_POINTER:              return "ZE_RESULT_ERROR_INVALID_NULL_POINTER";
        case ZE_RESULT_ERROR_INVALID_SIZE:                      return "ZE_RESULT_ERROR_INVALID_SIZE";
        case ZE_RESULT_ERROR_UNSUPPORTED_SIZE:                  return "ZE_RESULT_ERROR_UNSUPPORTED_SIZE";
        case ZE_RESULT_ERROR_UNSUPPORTED_ALIGNMENT:             return "ZE_RESULT_ERROR_UNSUPPORTED_ALIGNMENT";
        case ZE_RESULT_ERROR_INVALID_SYNCHRONIZATION_OBJECT:    return "ZE_RESULT_ERROR_INVALID_SYNCHRONIZATION_OBJECT";
        case ZE_RESULT_ERROR_INVALID_ENUMERATION:               return "ZE_RESULT_ERROR_INVALID_ENUMERATION";
        case ZE_RESULT_ERROR_UNSUPPORTED_ENUMERATION:           return "ZE_RESULT_ERROR_UNSUPPORTED_ENUMERATION";
        case ZE_RESULT_ERROR_UNSUPPORTED_IMAGE_FORMAT:          return "ZE_RESULT_ERROR_UNSUPPORTED_IMAGE_FORMAT";
        case ZE_RESULT_ERROR_INVALID_NATIVE_BINARY:             return "ZE_RESULT_ERROR_INVALID_NATIVE_BINARY";
        case ZE_RESULT_ERROR_INVALID_GLOBAL_NAME:               return "ZE_RESULT_ERROR_INVALID_GLOBAL_NAME";
        case ZE_RESULT_ERROR_INVALID_KERNEL_NAME:               return "ZE_RESULT_ERROR_INVALID_KERNEL_NAME";
        case ZE_RESULT_ERROR_INVALID_FUNCTION_NAME:             return "ZE_RESULT_ERROR_INVALID_FUNCTION_NAME";
        case ZE_RESULT_ERROR_INVALID_GROUP_SIZE_DIMENSION:      return "ZE_RESULT_ERROR_INVALID_GROUP_SIZE_DIMENSION";
        case ZE_RESULT_ERROR_INVALID_GLOBAL_WIDTH_DIMENSION:    return "ZE_RESULT_ERROR_INVALID_GLOBAL_WIDTH_DIMENSION";
        case ZE_RESULT_ERROR_INVALID_KERNEL_ARGUMENT_INDEX:     return "ZE_RESULT_ERROR_INVALID_KERNEL_ARGUMENT_INDEX";
        case ZE_RESULT_ERROR_INVALID_KERNEL_ARGUMENT_SIZE:      return "ZE_RESULT_ERROR_INVALID_KERNEL_ARGUMENT_SIZE";
        case ZE_RESULT_ERROR_INVALID_KERNEL_ATTRIBUTE_VALUE:    return "ZE_RESULT_ERROR_INVALID_KERNEL_ATTRIBUTE_VALUE";
        case ZE_RESULT_ERROR_INVALID_MODULE_UNLINKED:           return "ZE_RESULT_ERROR_INVALID_MODULE_UNLINKED";
        case ZE_RESULT_ERROR_INVALID_COMMAND_LIST_TYPE:         return "ZE_RESULT_ERROR_INVALID_COMMAND_LIST_TYPE";
        case ZE_RESULT_ERROR_OVERLAPPING_REGIONS:               return "ZE_RESULT_ERROR_OVERLAPPING_REGIONS";
        case ZE_RESULT_WARNING_ACTION_REQUIRED:                 return "ZE_RESULT_WARNING_ACTION_REQUIRED";
        case ZE_RESULT_ERROR_UNKNOWN:                           return "ZE_RESULT_ERROR_UNKNOWN";
        default:                                                return "ZE_RESULT_FORCE_UINT32";
    }
}

# define ZE_SAFE_CALL(X)                                                                \
    do {                                                                                \
        ze_result_t r = X;                                                              \
        if (r != ZE_RESULT_SUCCESS)                                                     \
            LOGGER_FATAL("`%s` failed with err=%s (%d)", #X, ze_error_to_str(r), r);    \
    } while (0)



# endif
