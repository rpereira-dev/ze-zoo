all:
	icpx -Wall -Werror -Wextra -g -O0 -I /usr/include/level_zero/ main-memcpy2d.cc logger.cc -lze_loader -o memcpy2d
	icpx -Wall -Werror -Wextra -g -O0 -I /usr/include/level_zero/ main-memory-usage.cc logger.cc  -lze_loader -o zes-leak
	icpx -Wall -Werror -Wextra -g -O0 -fiopenmp omp-bind.cc -o omp-bind

