#define _XOPEN_SOURCE 500

#include <unistd.h>
#include <inttypes.h>
#include <linux/limits.h>
#include <string.h>
#include <malloc.h>
#include <stdbool.h>
#include <stdarg.h>
#include "wrapper.h"

const uint64_t MAX_MEMORY = 5 * 1024 * 1024;

FILE *open_exe(const char *path)
{
	char path1[PATH_MAX];
	char path2[PATH_MAX];
	char *current_path = path1;
	char *next_path = path2;
	strncpy(current_path, path, PATH_MAX - 1);
	current_path[PATH_MAX - 1] = 0;

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
	if (result == NULL)
	{
		log_error("Failed to allocate wrapper!\n");
		return NULL;
	}

	result->argc = argc;
	uint32_t argv_size = sizeof(char *) * result->argc;
	result->argv = (char **)malloc(argv_size);
	if (result->argv == NULL)
	{
		log_error("Failed to allocate wrapper argv with %i elements!\n", result->argc);
		free(result);
		return NULL;
	}
	memset(result->argv, 0, argv_size);
	result->allow_extra_args = false;

	return result;
}

version get_version()
{
	version version = {0};
	sscanf(APP_VERSION, "%" SCNu8 ".%" SCNu8 ".%" SCNu8, &version.major, &version.minor, &version.patch);
	return version;
}

int version_compare(version a, version b)
{
	int major = ((int)a.major) - ((int)b.major);
	if (major != 0)
	{
		return major;
	}

	int minor = ((int)a.minor) - ((int)b.minor);
	if (minor != 0)
	{
		return minor;
	}

	return ((int)a.patch) - ((int)b.patch);
}

void version_format(char output[12], version version)
{
	snprintf(output, 12, "%i.%i.%i", version.major, version.minor, version.patch);
}

wrapper *wrapper_build_from_runner(FILE *runner_exe)
{
	uint32_t arguments_offset;
	fseek(runner_exe, -sizeof(arguments_offset), SEEK_END);
	read_or_return(&arguments_offset, runner_exe, NULL, "Failed to read arguments offset\n");
	log_debug("Arguments offset: 0x%x\n", arguments_offset);
	fseek(runner_exe, arguments_offset, SEEK_SET);

	char marker[strlen(APP_NAME) + 1];
	read_or_return(&marker, runner_exe, NULL, "Failed to read marker\n");
	marker[strlen(APP_NAME)] = 0;
	if (strcmp(marker, APP_NAME) != 0)
	{
		log_error("Executable was not created with " APP_NAME ", or was created with an incompatible version\n");
		return NULL;
	}
	log_debug("Marker check passed\n");

	version version;
	read_or_return(&version, runner_exe, NULL, "Failed to read runner version\n");
	char version_string[12];
	version_format(version_string, version);
	log_debug("Runner version: %s\n", version_string);
	if (version_compare(version, get_version()) != 0)
	{
		log_error("Runner was created with an incompatible version of " APP_NAME "\n");
		return NULL;
	}
	log_debug("Version check passed\n");

	argc_t argc;
	read_or_return(&argc, runner_exe, NULL, "Failed to read argument count\n");
	log_debug("Arguments count: %i\n", argc);

	wrapper *result = wrapper_new(argc);
	if (result == NULL)
	{
		return NULL;
	}

	for (uint32_t i = 0; i < result->argc; i++)
	{
		argv_len_t len;
		read_or_return(&len, runner_exe, NULL, "Failed to read length of argument %i\n", i);
		result->argv[i] = (char *)malloc(sizeof(char) * (len + 1));
		if (result->argv[i] == NULL)
		{
			log_error("Failed to allocate wrapper argv[%i] with length %i!\n", i, len);
			wrapper_destroy(result);
			return NULL;
		}

		char arg[len];
		read_or_return(&arg, runner_exe, NULL, "Failed to read argument %i\n", i);
		memcpy(result->argv[i], arg, len);
		result->argv[i][len] = 0;
	}

	read_or_return(&result->allow_extra_args, runner_exe, NULL, "Failed to read allow_extra_args\n");

	return result;
}

void wrapper_destroy(wrapper *wrapper)
{
	for (int i = 0; i < wrapper->argc; i++)
	{
		if (wrapper->argv[i])
		{
			free(wrapper->argv[i]);
		}
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
