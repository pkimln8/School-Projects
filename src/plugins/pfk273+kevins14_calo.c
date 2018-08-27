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
    printf("Plugin 'calo' initialized...\n");
    return true;
}

/* Implement calo built-in.
 * Returns true if handled, false otherwise. */
static bool
calo_builtin(struct esh_command *cmd)
{
	if (strcmp(cmd->argv[0], "calo") != 0)
	        return false;

	if(cmd->argv[1] != NULL && cmd->argv[2] != NULL && cmd->argv[3] != NULL) {

		int carbs = atoi(cmd->argv[1]);
		int protein = atoi(cmd->argv[2]);
		int fat = atoi(cmd->argv[3]);

		int total = carbs * 4 + protein * 4 + fat * 9;

		printf("%dcal is total calories for %dgrams of carbs, %dgrams of prtein and %dgrams of fat.\n"
			, total, carbs, protein, fat);
	}
	else
		esh_sys_error("Need three arguments \n");

	return true;
}

struct esh_plugin esh_module = {
  .rank = 1,
  .init = init_plugin,
  .process_builtin = calo_builtin
};
