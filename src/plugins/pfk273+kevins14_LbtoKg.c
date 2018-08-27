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
    printf("Plugin 'cd' initialized...\n");
    return true;
}

/* Implement chdir built-in.
 * Returns true if handled, false otherwise. */
static bool
lbtokg_builtin(struct esh_command *cmd)
{
	if (strcmp(cmd->argv[0], "lbtokg") != 0)
	        return false;

	if(cmd->argv[1] != NULL && cmd->argv[2] != NULL) {

		char *unit = cmd->argv[1];
		int numbers = atoi(cmd->argv[2]);
		double converted;

		//convert lb to kg
		if (strcmp(unit, "lb") == 0) {

			converted = numbers * 2.20462;
			printf("%dkg is %.1flbs \n", numbers, converted);

			return true;
		}
		//convert kg to lb
		else if(strcmp(unit, "kg") == 0) {

			converted = numbers * 0.453592;
			printf("%dlbs is %.1fkg \n", numbers, converted);
		}
		else 
			esh_sys_error("Put proper arguments \n");
	}
	else
		esh_sys_error("Need two arguments \n");

	return true;
}

struct esh_plugin esh_module = {
  .rank = 1,
  .init = init_plugin,
  .process_builtin = lbtokg_builtin
};
