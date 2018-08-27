#include <stdbool.h>
#include <stdio.h>
#include <pwd.h>
#include <unistd.h>
#include <sys/types.h>
#include "../esh.h"
#include <signal.h>
#include "../esh-sys-utils.h"

static bool 
init_plugin(struct esh_shell *shell)
{
    printf("Plugin 'second' initialized...\n");
    return true;
}

/* Implement second built-in.
 * Returns true if handled, false otherwise. */
static bool
second_builtin(struct esh_command *cmd)
{
	if (strcmp(cmd->argv[0], "second") != 0)
	        return false;

	if(cmd->argv[1] != NULL) {

		int temp = atoi(cmd->argv[1]);
		int sec, min, hour, day;


		sec = temp % 60;
		min = (temp/60) % 60;
		hour =  (temp/3600) % 24;
		day = (temp/86400) % 30;

		printf("%dsec is %dday %dhour %dmin %dsec.\n", temp, day, hour, min, sec);

	}
	else
		esh_sys_error("Need an argument \n");

	return true;
}

struct esh_plugin esh_module = {
  .rank = 1,
  .init = init_plugin,
  .process_builtin = second_builtin
};
