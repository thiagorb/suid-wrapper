#ifndef __SRC_WRAPPER_H
#define __SRC_WRAPPER_H

#include <stdint.h>
#include <stdio.h>

typedef void (*log_function)(char*, ...);

void log_silent(char *format, ...);
void log_stderr(char *format, ...);

extern log_function log_debug;
extern log_function log_info;
extern log_function log_error;

#define read_or_return(target, file, value, ...) \
	if (fread(target, sizeof(*target), 1, file) != 1) \
	{ \
		log_error(__VA_ARGS__); \
		return value; \
	}

typedef int argc_t;
typedef int argv_len_t;

typedef struct
{
	argc_t argc;
	char **argv;
} wrapper;

wrapper *wrapper_build_from_args(argc_t argc, char **argv);
wrapper *wrapper_build_from_runner(FILE *runner_exe);
void wrapper_destroy(wrapper *wrapper);

FILE *open_exe(const char *path);

#endif // __SRC_WRAPPER_H
