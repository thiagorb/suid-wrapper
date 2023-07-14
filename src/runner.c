#define _XOPEN_SOURCE 500

#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <pwd.h>
#include "wrapper.h"

/**
 * Format a string and return a pointer to the alloc'd result.
 */
#define ssprintf(...) ({                                          \
	int required_bytes = snprintf(NULL, 0, __VA_ARGS__) + 1;      \
	char *result = (char *)malloc(sizeof(char) * required_bytes); \
	snprintf(result, required_bytes, __VA_ARGS__);                \
	result;                                                       \
})

int main(int argc, char **argv)
{
	FILE *self_exe = open_exe("/proc/self/exe");
	wrapper *wrapper = wrapper_build_from_runner(self_exe);
	fclose(self_exe);
	if (wrapper == NULL)
	{
		return 1;
	}

	char *new_argv[wrapper->argc + argc];
	for (int i = 0; i < wrapper->argc; i++)
	{
		new_argv[i] = wrapper->argv[i];
	}

	if (wrapper->allow_extra_args)
	{
		for (int i = 0; i < argc - 1; i++)
		{
			new_argv[wrapper->argc + i] = argv[i + 1];
		}
		new_argv[wrapper->argc + argc - 1] = NULL;
	}
	else
	{
		new_argv[wrapper->argc] = NULL;
	}

	uid_t uid = getuid();
	gid_t gid = getgid();
	char *user = getpwuid(uid)->pw_name;

	char *new_env[] = {
		ssprintf("SUID_WRAPPER_UID=%d", uid),
		ssprintf("SUID_WRAPPER_GID=%d", gid),
		ssprintf("SUID_WRAPPER_USER=%s", user),
		NULL,
	};

	if (setuid(geteuid()) != 0)
	{
		log_error("Failed to set uid\n");
	}

	if (setgid(getegid()) != 0)
	{
		log_error("Failed to set gid\n");
	}

	int result = execve(new_argv[0], &new_argv[0], new_env);
	if (result != 0)
	{
		fprintf(stderr, "Failed to execute program %s:\n\t%s", new_argv[0], strerror(errno));
		return 1;
	}
	wrapper_destroy(wrapper);

	return 0;
}
