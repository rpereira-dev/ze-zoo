/* ************************************************************************** */
/*                                                                            */
/*   spinlock.h                                                               */
/*                                                                   .-*-.    */
/*   Author: Romain PEREIRA <romain.pereira@inria.fr>              .'* *.'    */
/*                                                              __/_*_*(_     */
/*   Created: 2024/12/17 13:03:48 by Romain PEREIRA            / _______ \    */
/*   Updated: 2024/12/18 15:26:20 by Romain PEREIRA            \_)     (_/    */
/*                                                                            */
/*   License: CeCILL-C                                                        */
/*                                                                            */
/* ************************************************************************** */

#ifndef __SPINLOCK_H__
# define __SPINLOCK_H__

# include "mem.h"

#if 1

typedef volatile int spinlock_t;

# define SPINLOCK_LOCK(L)                                       \
    do {                                                        \
        int zero = 0;                                           \
        while (__sync_val_compare_and_swap(&L, zero, 1) == 1)   \
            mem_pause();                                        \
    } while (0)

# define SPINLOCK_UNLOCK(L)             \
    do {                                \
        __sync_fetch_and_xor(&L, L);    \
    } while (0)

# define SPINLOCK_INITIALIZER 0

# else

# include <pthread.h>
typedef pthread_mutex_t spinlock_t;
# define SPINLOCK_LOCK(L)       pthread_mutex_lock(&L)
# define SPINLOCK_UNLOCK(L)     pthread_mutex_unlock(&L)
# define SPINLOCK_INITIALIZER   PTHREAD_MUTEX_INITIALIZER

# endif

#endif /* __SPINLOCK_H__ */
