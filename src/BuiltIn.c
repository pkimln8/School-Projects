#include "BuiltIn.h"
#include <stdio.h>
#include <readline/readline.h>
#include <unistd.h>
#include <sys/wait.h>

static void command_jobs(void);

void esh_built_in_commands(struct esh_pipeline *pipe) {

	struct list_elem * e = list_begin(&pipe->commands);
	struct esh_command *cmd = list_entry(e, struct esh_command, elem);

	//exit
	if (strcmp(cmd->argv[0], "exit") == 0)
		exit(EXIT_SUCCESS);
	//jobs
	else if (strcmp(cmd->argv[0], "jobs") == 0)
		command_jobs();
	//bg, kill, stop
	else {

		if(!list_empty(&joblist)) {

			struct esh_pipeline *job;

			if(cmd->argv[1] == NULL) {

				struct list_elem *e = list_back(&joblist);

				job = list_entry(e, struct esh_pipeline, elem);
			}
			else {

				struct list_elem *e = list_begin(&joblist);

				if(atoi(cmd->argv[1]) >= list_size(&joblist)) {

					for(; e != list_end(&joblist); e = list_next(e)) {

						job = list_entry(e, struct esh_pipeline, elem);
						if(atoi(cmd->argv[1]) == job->jid)
							break;
					}
				}
			}

			if(job != NULL) {

				esh_signal_block(SIGCHLD);
				//bg
				if (strcmp(cmd->argv[0], "bg") == 0) {

					job->status = BACKGROUND;
				
					printf ("[%d] %d\n", job->jid, job->pgrp);
					if(kill(job->pgrp * -1, SIGCONT) < 0) 
						esh_sys_fatal_error("SIGCONT ERROR");
				}
				//kill
				if (strcmp(cmd->argv[0], "kill") == 0) {

					if(kill(job->pgrp * -1, SIGKILL) < 0) 
						esh_sys_fatal_error("SIGKILL ERROR");
				}
				//stop
				if (strcmp(cmd->argv[0], "stop") == 0) {

					if(kill(job->pgrp * -1, SIGSTOP) < 0) 
						esh_sys_fatal_error("SIGSTOP ERROR");
				}
				esh_signal_unblock(SIGCHLD);
			}
		}
	}

}

static void command_jobs(void) {

	struct list_elem * e = list_begin (&joblist);
	
	for (; e != list_end (&joblist); e = list_next (e)) {

		char *status[] = {"Foreground","Running", "Stopped"};

		struct esh_pipeline *job = list_entry(e, struct esh_pipeline, elem);

		printf("[%d] %s	(", job->jid, status[job->status]);

		struct list_elem *em = list_begin (&job->commands);

		struct esh_command *cmd = list_entry(em, struct esh_command, elem);

		char **p = cmd->argv;

		printf("%s", *p++);

		while (*p)
			printf(" %s", *p++);

		if (list_size(&job->commands) > 1) {

			em = list_next(em);
			for (; em != list_end(&job->commands); em = list_next(em)) {

				cmd = list_entry(em, struct esh_command, elem);

				p = cmd->argv;

				printf(" | %s", *p++);

				while(*p)
					printf(" %s", *p++);
			}
		}
		printf(")\n");
	}

}


