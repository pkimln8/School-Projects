/*
 * esh - the 'pluggable' shell *
 * Developed by Godmar Back for CS 3214 Fall 2009
 * Virginia Tech.
 */

#include <stdio.h>
#include <assert.h>
#include <readline/readline.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "esh.h"
#include "BuiltIn.h"

static void sigchld_handler(int sig, siginfo_t *info, void *_ctxt);
static void wait_for_job(struct esh_pipeline *pipeline);
static void child_status_change(pid_t child, int status);
static void give_terminal_to(pid_t pgrp, struct termios *pg_tty_state);

static void esh_command_line_execute(struct esh_command_line *cmdline);
static void esh_pipeline_execute(struct esh_pipeline *pipe, struct list_elem *jobe);

int i = 1;
struct termios *shell_tty;

static void sigchld_handler(int sig, siginfo_t *info, void *_ctxt) {

	pid_t child;
	int status;

	assert(sig == SIGCHLD);

	while ((child = waitpid(-1, &status, WUNTRACED|WNOHANG)) > 0) {
		child_status_change(child, status);
	}
}

static void give_terminal_to(pid_t pgrp, struct termios *pg_tty_state) {

	esh_signal_block(SIGTTOU);
	int rc = tcsetpgrp(esh_sys_tty_getfd(), pgrp);
	if (rc == -1)
		esh_sys_fatal_error("tcsetpgrp: ");

	if (pg_tty_state)
		esh_sys_tty_restore(pg_tty_state);
	esh_signal_unblock(SIGTTOU);
}


//----------------------------------------------

static void
usage(char *progname)
{
    printf("Usage: %s -h\n"
        " -h            print this help\n"
        " -p  plugindir directory from which to load plug-ins\n",
        progname);

    exit(EXIT_SUCCESS);
}

/* Build a prompt by assembling fragments from loaded plugins that 
 * implement 'make_prompt.'
 *
 * This function demonstrates how to iterate over all loaded plugins.
 */
static char *
build_prompt_from_plugins(void)
{
    char *prompt = NULL;
    struct list_elem * e = list_begin(&esh_plugin_list);

    for (; e != list_end(&esh_plugin_list); e = list_next(e)) {
        struct esh_plugin *plugin = list_entry(e, struct esh_plugin, elem);

        if (plugin->make_prompt == NULL)
            continue;

        /* append prompt fragment created by plug-in */
        char * p = plugin->make_prompt();
        if (prompt == NULL) {
            prompt = p;
        } else {
            prompt = realloc(prompt, strlen(prompt) + strlen(p) + 1);
            strcat(prompt, p);
            free(p);
        }
    }

    /* default prompt */
    if (prompt == NULL)
        prompt = strdup("esh> ");

    return prompt;
}

/* The shell object plugins use.
 * Some methods are set to defaults.
 */
struct esh_shell shell =
{
    .build_prompt = build_prompt_from_plugins,
    .readline = readline,       /* GNU readline(3) */ 
    .parse_command_line = esh_parse_command_line /* Default parser */
};

int
main(int ac, char *av[])
{
    int opt;
    list_init(&esh_plugin_list);
    list_init(&joblist);

    /* Process command-line arguments. See getopt(3) */
    while ((opt = getopt(ac, av, "hp:")) > 0) {
        switch (opt) {
        case 'h':
            usage(av[0]);
            break;

        case 'p':
            esh_plugin_load_from_directory(optarg);
            break;
        }
    }

	esh_plugin_initialize(&shell);
	esh_signal_sethandler(SIGCHLD, sigchld_handler);
	shell_tty = esh_sys_tty_init();
	give_terminal_to(getpgrp(), shell_tty);
	setpgid(0, 0);
	shellPid = getpid();

    /* Read/eval loop. */
    for (;;) {
        /* Do not output a prompt unless shell's stdin is a terminal */
        char * prompt = isatty(0) ? shell.build_prompt() : NULL;
        char * cmdline = shell.readline(prompt);
        free (prompt);

        if (cmdline == NULL)  /* User typed EOF */
            break;

        struct esh_command_line *cline = shell.parse_command_line(cmdline);
        free (cmdline);
        
	if (cline == NULL)                  /* Error in command line */
            continue;

        if (list_empty(&cline->pipes)) {    /* User hit enter */
            esh_command_line_free(cline);
            continue;
        }

        esh_command_line_execute(cline);
        esh_command_line_free(cline);

    }
    return 0;
}

static void esh_command_line_execute(struct esh_command_line *cmdline) {


	while (!list_empty (&cmdline->pipes)){

		struct list_elem *e = list_pop_front (&cmdline->pipes);
                struct esh_pipeline *pipe = list_entry(e, struct esh_pipeline, elem);

                esh_pipeline_execute(pipe, e);
	}

}

static void esh_pipeline_execute(struct esh_pipeline *pipel, struct list_elem *jobe) {

        struct list_elem * e = list_begin (&pipel->commands);
	struct esh_command *cmd = list_entry(e, struct esh_command, elem);
	

	int isPlugin = 0;

	//Plug in
	struct list_elem * l = list_begin(&esh_plugin_list);

	for (; l != list_end(&esh_plugin_list); l = list_next (l)) {

		struct esh_plugin *pi = list_entry(l, struct esh_plugin, elem);
		
		if (pi -> process_builtin == NULL) {
			break;		
		}		
		
		if (pi->process_builtin(cmd)) {

			isPlugin = 1;
			break;
		}
	}

	if(isPlugin == 0) {

		//Built in command
		if(strcmp(cmd->argv[0], "jobs") == 0 || strcmp(cmd->argv[0], "bg") == 0 || 
			strcmp(cmd->argv[0], "kill") == 0 || strcmp(cmd->argv[0], "stop") == 0 ||
			strcmp(cmd->argv[0], "exit") == 0) {

			esh_built_in_commands(cmd->pipeline);
		}
		//fg command
		else if (strcmp(cmd->argv[0], "fg") == 0) {

			if(!list_empty(&joblist)) {

				esh_signal_block(SIGCHLD);

				struct list_elem * e = list_begin (&joblist);

				if(cmd->argv[1] == NULL) {

					e = list_back(&joblist);
					struct esh_pipeline *job = list_entry(e, struct esh_pipeline, elem);

					struct list_elem *em = list_begin (&job->commands);
					struct esh_command *cmd = list_entry(em, struct esh_command, elem);

					char **p = cmd->argv;
					printf("%s", *p++);

					while (*p) {

					printf(" %s", *p++);
					}
		
					job->status = FOREGROUND;
					if(kill(job->pgrp * -1, SIGCONT) < 0) 
						esh_sys_fatal_error("SIGCONT ERROR");
		
					wait_for_job(job);
					give_terminal_to(getpgrp(), shell_tty);
				}
				else {
	
					for (; e != list_end (&joblist); e = list_next (e)) {

						struct esh_pipeline *job = list_entry(e, struct esh_pipeline, elem);

						if (atoi(cmd->argv[1]) == job->jid) {

							struct list_elem *em = list_begin (&job->commands);
							struct esh_command *cmd = list_entry(em, struct esh_command, elem);

							char **p = cmd->argv;
							printf("(%s", *p++);
		
							while (*p) {

							printf(" %s", *p++);
							}
							printf(")\n");

							job->status = FOREGROUND;
							if(kill(job->pgrp * -1, SIGCONT) < 0) 
								esh_sys_fatal_error("SIGCONT ERROR");
							give_terminal_to(job->pgrp, shell_tty);
							wait_for_job(job);
						}
					}
				}
			}
		}
		//executable commands
		else {


			//set the jid & status
			if (!pipel->bg_job) {

				pipel->status = FOREGROUND;

				if(list_empty(&joblist))
					i = 1;
				pipel->jid = i;
				i++;
			}
			else {

				pipel->status = BACKGROUND;

				if(list_empty(&joblist))
					i = 1;
				pipel->jid = i;	
				i++;
			}

			pipel->pgrp = -1;

			int prevPipe[2], nextPipe[2];

			//check if it needs to be piped
			int needPiped = 0;
			if (list_size(&pipel->commands) > 1)
				needPiped = 1;

			for (; e != list_end (&pipel->commands); e = list_next (e)) {

				esh_signal_block(SIGCHLD);

				if (needPiped == 1 && list_next(e) != list_tail(&pipel->commands))
					pipe(nextPipe);

				int pid = fork();
				if (pid < 0)
					perror("fork"), exit(-1);

				cmd = list_entry(e, struct esh_command, elem);

				//child process
				if(pid == 0) {

					pid = getpid();
					cmd->pid = pid;

					//set pgrp
					if (cmd->pipeline->pgrp < 0)
						cmd->pipeline->pgrp = pid;

					if (setpgid(pid, cmd->pipeline->pgrp) < 0)
						esh_sys_fatal_error("Error: Set pgid");

					//pipe
					if (needPiped == 1 && e != list_begin(&pipel->commands)) {

						close(prevPipe[1]);
						dup2(prevPipe[0], 0);
						close(prevPipe[0]);
					}

					if (needPiped == 1 && e != list_tail(&pipel->commands)) {

						close(nextPipe[0]);
						dup2(nextPipe[1], 1);
						close(nextPipe[1]);
					}

					//io redirecting
					if (cmd->iored_input != NULL) {

						int inFd = open(cmd->iored_input, O_RDONLY);

						if (dup2(inFd, 0) < 0)
							esh_sys_fatal_error("Error: dup2");

						close(inFd);
					}

					if (cmd->iored_output != NULL) {

						int outFd;

						if (cmd->append_to_output)
							outFd = open(cmd->iored_output, O_WRONLY | O_APPEND | O_CREAT, S_IRWXU | S_IWUSR | S_IRGRP | S_IWGRP);
						else
							outFd = open(cmd->iored_output, O_WRONLY | O_TRUNC | O_CREAT, S_IRWXU | S_IWUSR | S_IRGRP | S_IWGRP);

						if (dup2(outFd, 1) < 0 )
							esh_sys_fatal_error("Error: dup2");

						close(outFd);
					}

					//give control to job
					if(!cmd->pipeline->bg_job)
						give_terminal_to(cmd->pipeline->pgrp, shell_tty);

					//execute
					execvp(*cmd->argv, cmd->argv);
					exit(0);

				}
				//parent process
				else {

					cmd->pid = pid;

					//set pgrp
					if(cmd->pipeline->pgrp < 0)
						cmd->pipeline->pgrp = pid;

					if (setpgid(pid, cmd->pipeline->pgrp) < 0)
						esh_sys_fatal_error("Error Setting Process Group");

					if (needPiped == 1 && e != list_begin(&pipel->commands)) {
					// close the previous pipes
						close(prevPipe[0]);
						close(prevPipe[1]);
					}

					if (needPiped == 1 && list_next(e) != list_tail(&pipel->commands)) {
					//setting the previous pipe to the next pipe
						prevPipe[0] = nextPipe[0];
						prevPipe[1] = nextPipe[1];
					}

					if (needPiped == 1 && list_next(e) == list_end(&pipel->commands)) {

						close(prevPipe[0]);
						close(prevPipe[1]);
						close(nextPipe[0]);
						close(nextPipe[1]);
					}
				}
			}
		

			//push pipeline to job list
			list_push_back(&joblist, jobe);

			if(pipel->status == FOREGROUND)
				wait_for_job(cmd->pipeline);

			if(cmd->pipeline->bg_job) {

				printf("[%d] %d\n", cmd->pipeline->jid, cmd->pipeline->pgrp);
			}

			//give control to terminal & signal unblock
			give_terminal_to(shellPid, shell_tty);
			esh_signal_unblock(SIGCHLD);
		}
	}
}

static void wait_for_job(struct esh_pipeline *pipeline) {

		assert(esh_signal_is_blocked(SIGCHLD));
		int status;
		pid_t child = waitpid(-1, &status, WUNTRACED);

		if (child != -1) {
			child_status_change(child, status);
			give_terminal_to(getpgrp(), shell_tty);
		}
}

static void child_status_change(pid_t child, int status) {

	if (WSTOPSIG(status)) {

		struct list_elem * e = list_begin (&joblist);

		for (; e != list_end (&joblist); e = list_next (e)) {

			char *status[] = {"Foreground","Running", "Stopped"};

			struct esh_pipeline *job = list_entry(e, struct esh_pipeline, elem);
			struct list_elem *em = list_begin(&job->commands);
			struct esh_command *cmd = list_entry(em, struct esh_command, elem);

			if(cmd->pid == child) {
				job->status = STOPPED;
			}

			printf("[%d]  %s\t", job->jid, status[job->status]);
			char **p = cmd->argv;

			printf("(");
			printf("%s", *p++);

			while (*p) {

				printf(" %s", *p++);
			}


			printf(")\n");
		}
	}
	else if(WIFEXITED(status)) {

		struct list_elem * e = list_begin (&joblist);

		for (; e != list_end (&joblist); e = list_next (e)) {

			struct esh_pipeline *job = list_entry(e, struct esh_pipeline, elem);
			struct list_elem *em = list_begin(&job->commands);
			struct esh_command *cmd = list_entry(em, struct esh_command, elem);

			if(cmd->pid == child) {
				
				list_remove(e);
			}
		}
	}
	else if(WTERMSIG(status)) {

		struct list_elem * e = list_begin (&joblist);

		for (; e != list_end (&joblist); e = list_next (e)) {

			struct esh_pipeline *job = list_entry(e, struct esh_pipeline, elem);
			struct list_elem *em = list_begin(&job->commands);
			struct esh_command *cmd = list_entry(em, struct esh_command, elem);

			if(cmd->pid == child) {
				list_remove(e);
			}
		}
	}
}
