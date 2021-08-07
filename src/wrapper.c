#define _XOPEN_SOURCE 500

#include <unistd.h>
#include <linux/limits.h>
#include <string.h>
#include <malloc.h>
#include <stdbool.h>
#include "wrapper.h"
#include <stdarg.h>

const uint64_t MAX_MEMORY = 5 * 1024 * 1024;

FILE *open_exe(const char *path)
{
	char path1[PATH_MAX];
	char path2[PATH_MAX];
	char *current_path = path1;
	char *next_path = path2;
	strcpy(current_path, path);

	ssize_t path_len = readlink(current_path, next_path, PATH_MAX);
	while (path_len > 0)
	{
		log_debug("Following symlink %s...\n", current_path);
		next_path[path_len] = 0;
		char *temp = current_path;
		current_path = next_path;
		next_path = temp;
		path_len = readlink(current_path, next_path, PATH_MAX) != -1;
	}
	log_debug("Executable found in %s.\n", current_path);

	return fopen(current_path, "r");
}

wrapper *wrapper_new(int argc)
{
	wrapper *result = (wrapper *)malloc(sizeof(wrapper));
	result->argc = argc;
	result->argv = (char **)malloc(sizeof(char *) * result->argc);
	return result;
}

wrapper *wrapper_build_from_args(int argc, char **argv)
{
	wrapper *result = wrapper_new(argc);
	for (int i = 0; i < result->argc; i++)
	{
		argv_len_t len = strlen(argv[i]);
		result->argv[i] = (char *)malloc(sizeof(char) * (len + 1));
		strcpy(result->argv[i], argv[i]);
	}

	return result;
}

wrapper *wrapper_build_from_runner(FILE *runner_exe)
{
	uint32_t arguments_offset;
	fseek(runner_exe, -sizeof(arguments_offset), SEEK_END);

	read_or_return(&arguments_offset, runner_exe, NULL, "Failed to read arguments offset");

	argc_t argc;
	fseek(runner_exe, arguments_offset, SEEK_SET);
	read_or_return(&argc, runner_exe, NULL, "Failed to read argument count");

	wrapper *result = wrapper_new(argc);
	for (uint32_t i = 0; i < result->argc; i++)
	{
		argv_len_t len;
		read_or_return(&len, runner_exe, NULL, "Failed to read length of argument %i", i);
		result->argv[i] = (char *)malloc(sizeof(char) * (len + 1));
		char arg[len];
		read_or_return(&arg, runner_exe, NULL, "Failed to read argument %i", i);
		memcpy(result->argv[i], arg, len);
		result->argv[i][len] = 0;
	}

	return result;
}

void wrapper_destroy(wrapper *wrapper)
{
	for (int i = 0; i < wrapper->argc; i++)
	{
		free(wrapper->argv[i]);
	}
	free(wrapper->argv);
	free(wrapper);
}

void log_silent(char *format, ...)
{
}

void log_stderr(char *format, ...)
{
	va_list arg;
	va_start(arg, format);
	vfprintf(stderr, format, arg);
	va_end(arg);
}

log_function log_debug = log_silent;
log_function log_info = log_stderr;
log_function log_error = log_stderr;
