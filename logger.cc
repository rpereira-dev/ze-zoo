/* ************************************************************************** */
/*                                                                            */
/*   logger.cc                                                                */
/*                                                                   .-*-.    */
/*   Author: Romain PEREIRA <romain.pereira@inria.fr>              .'* *.'    */
/*                                                              __/_*_*(_     */
/*   Created: 2024/12/17 13:03:43 by Romain PEREIRA            / _______ \    */
/*   Updated: 2024/12/19 18:55:58 by Romain PEREIRA            \_)     (_/    */
/*                                                                            */
/*   License: CeCILL-C                                                        */
/*                                                                            */
/* ************************************************************************** */

# include <stdint.h>

# include "spinlock.h"

volatile spinlock_t LOGGER_PRINT_MTX;

volatile double     LOGGER_TIME_ELAPSED = 0.0;
volatile uint64_t   LOGGER_LAST_TIME    = 0;

# define NLVL 6

char const * LOGGER_PRINT_COLORS[NLVL] = {
    "\033[1;31m",
    "\033[1;31m",
    "\033[1;33m",
    "\033[1;32m",
    "\033[1;35m",
    "\033[1;36m",
};

char const * LOGGER_PRINT_HEADERS[NLVL] = {
    "FATAL",
    "ERROR",
    "WARN",
    "INFO",
    "IMPL",
    "DEBUG",
};

int LOGGER_VERBOSE = NLVL;
