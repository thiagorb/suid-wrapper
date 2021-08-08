#define _XOPEN_SOURCE 500

#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include "wrapper.h"

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

	for (int i = 0; i < argc - 1; i++)
	{
		new_argv[wrapper->argc + i] = argv[i + 1];
	}
	new_argv[wrapper->argc + argc - 1] = NULL;

	if (setuid(geteuid()) != 0)
	{
		log_error("Failed to set uid\n");
	}

	if (setgid(getegid()) != 0)
	{
		log_error("Failed to set gid\n");
	}

	char *new_env[] = { NULL };
	execve(new_argv[0], &new_argv[0], new_env);
	wrapper_destroy(wrapper);

	return 0;
}
