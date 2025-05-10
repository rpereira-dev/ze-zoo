/* ************************************************************************** */
/*                                                                            */
/*   logger.h                                                                 */
/*                                                                   .-*-.    */
/*   Author: Romain PEREIRA <romain.pereira@inria.fr>              .'* *.'    */
/*                                                              __/_*_*(_     */
/*   Created: 2024/12/17 13:03:43 by Romain PEREIRA            / _______ \    */
/*   Updated: 2025/04/03 16:04:18 by Romain PEREIRA            \_)     (_/    */
/*                                                                            */
/*   License: CeCILL-C                                                        */
/*                                                                            */
/* ************************************************************************** */

#ifndef __LOGGER_H__
# define __LOGGER_H__

# include "spinlock.h"

# include <time.h>
# include <unistd.h>
# include <stdio.h>
# include <stdlib.h>
# include <stdint.h>

extern spinlock_t LOGGER_PRINT_MTX;

# ifndef LOGGER_FD
#  define LOGGER_FD stderr
# endif

# ifndef LOGGER_HEADER
#  define LOGGER_HEADER "LOGGER"
# endif

# define LOGGER_PRINT_FATAL_ID      0
# define LOGGER_PRINT_ERROR_ID      1
# define LOGGER_PRINT_WARN_ID       2
# define LOGGER_PRINT_INFO_ID       3
# define LOGGER_PRINT_IMPL_ID       4
# define LOGGER_PRINT_DEBUG_ID      5

extern char * LOGGER_PRINT_COLORS[6];
extern char * LOGGER_PRINT_HEADERS[6];

extern int LOGGER_VERBOSE;

extern volatile double   LOGGER_TIME_ELAPSED;
extern volatile uint64_t LOGGER_LAST_TIME;

# define LOGGER_PRINT_LINE() \
    fprintf(LOGGER_FD, "%s:%d (%s)\n", __FILE__, __LINE__, __func__);

# define LOGGER_PRINT(LVL, ...)                                                 \
    do {                                                                        \
        if (LVL <= LOGGER_VERBOSE)                                              \
        {                                                                       \
            SPINLOCK_LOCK(LOGGER_PRINT_MTX);                                    \
            struct timespec _ts;                                                \
            clock_gettime(CLOCK_MONOTONIC, &_ts);                               \
            uint64_t t = (uint64_t)(_ts.tv_sec * 1000000000) +                  \
                            (uint64_t) _ts.tv_nsec;                             \
            if (LOGGER_LAST_TIME != 0)                                          \
                LOGGER_TIME_ELAPSED += (double) (t - LOGGER_LAST_TIME) / 1e9;   \
            LOGGER_LAST_TIME = t;                                               \
            if (isatty(STDOUT_FILENO))                                          \
                fprintf(LOGGER_FD, "[%8lf] "                                    \
                                "[TID=%d] "                                     \
                                "[\033[1;37m" LOGGER_HEADER "\033[0m] "         \
                                "[%s%s\033[0m] ",                               \
                                LOGGER_TIME_ELAPSED,                            \
                                gettid(),                                       \
                                LOGGER_PRINT_COLORS[LVL],                       \
                                LOGGER_PRINT_HEADERS[LVL]);                     \
            else                                                                \
                fprintf(LOGGER_FD, "[%8lf]"                                     \
                                "[TID=%d] "                                     \
                                "[" LOGGER_HEADER "] "                          \
                                "[%s] ",                                        \
                                LOGGER_TIME_ELAPSED,                            \
                                gettid(),                                       \
                                LOGGER_PRINT_HEADERS[LVL]);                     \
            fprintf(LOGGER_FD, __VA_ARGS__);                                    \
            fprintf(LOGGER_FD, "\n");                                           \
            fflush(LOGGER_FD);                                                  \
            SPINLOCK_UNLOCK(LOGGER_PRINT_MTX);                                  \
        }                                                                       \
        if (LVL == LOGGER_PRINT_FATAL_ID)                                       \
        {                                                                       \
            LOGGER_PRINT_LINE();                                                \
            fflush(LOGGER_FD);                                                  \
            abort();                                                            \
        }                                                                       \
    } while (0)

# define LOGGER_NOT_SUPPORTED()   LOGGER_NOT_IMPLEMENTED_WARN("Not supported")
# define LOGGER_NOT_IMPLEMENTED() LOGGER_NOT_IMPLEMENTED_WARN("Not implemented")

# define LOGGER_NOT_IMPLEMENTED_WARN(S)                                 \
    LOGGER_IMPL("'%s' at %s:%d in %s()",                                \
            S, __FILE__, __LINE__, __func__);

# define LOGGER_INFO(...)  LOGGER_PRINT(LOGGER_PRINT_INFO_ID,  __VA_ARGS__)
# define LOGGER_WARN(...)  LOGGER_PRINT(LOGGER_PRINT_WARN_ID,  __VA_ARGS__)
# define LOGGER_ERROR(...) LOGGER_PRINT(LOGGER_PRINT_ERROR_ID, __VA_ARGS__)
# define LOGGER_IMPL(...)  LOGGER_PRINT(LOGGER_PRINT_IMPL_ID,  __VA_ARGS__)
# ifndef NDEBUG
#  define LOGGER_DEBUG(...) LOGGER_PRINT(LOGGER_PRINT_DEBUG_ID, __VA_ARGS__)
# else
#  define LOGGER_DEBUG(...)
# endif
# define LOGGER_FATAL(...) LOGGER_PRINT(LOGGER_PRINT_FATAL_ID, __VA_ARGS__)

# define HWLOC_SAFE_CALL(X)                                 \
    do {                                                    \
        int r = X;                                          \
        if (r)                                              \
            LOGGER_FATAL("`%s` failed with err=%d", #X, r); \
    } while (0)

#endif /* __LOGGER_H__*/
