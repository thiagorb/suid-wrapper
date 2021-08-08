#ifndef __SRC_WRAPPER_H
#define __SRC_WRAPPER_H

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#ifndef APP_NAME
	#define APP_NAME "suid-wrapper"
#endif

#ifndef APP_VERSION
	#define APP_VERSION "0.1"
#endif

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

typedef struct wrapper
{
	argc_t argc;
	char **argv;
	bool allow_extra_args;
} wrapper;

typedef struct version
{
	uint8_t major;
	uint8_t minor;
	uint8_t patch;
} version;

version get_version();

wrapper *wrapper_new(int argc);
wrapper *wrapper_build_from_runner(FILE *runner_exe);
void wrapper_destroy(wrapper *wrapper);

FILE *open_exe(const char *path);

#endif // __SRC_WRAPPER_H
