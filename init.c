#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct actions_ {
	int signal;
	void (*action)(void);
} actions;

void action_init(void);
void action_shutdown(void);
void action_reap(void);

actions init_actions[] = {
	{ SIGUSR1, action_init },
	{ SIGUSR2, action_shutdown },
	{ SIGCHLD, action_reap },
};

int main(void)
{
	sigset_t set;
	int signal;
	int i;

	if (getpid() != 1) return 1;

	sigfillset(&set);
	sigprocmask(SIG_BLOCK, &set, NULL);

	if (fork())
		for (;;) {
			sigwait(&set, &signal);
			for (i = 0; i < 3; i++) {
				if (init_actions[i].signal == signal) {
					init_actions[i].action();
					break;
				}
			}				
		}
	return 0;
}

void action_init(void) {
	printf("Welcome new world! I am performing the system initialization!\n");
	if (fork() == 0) {
		execv("/etc/rc.init", NULL);
		perror("execv");
		exit(-1);
	} 
}

void action_shutdown(void) {
	printf("Goodbye cruel world! I am shutting down!\n");
	if (fork() == 0) {
		execv("/etc/rc.shutdown", NULL);
		perror("execv");
		exit(-1);
	}
}

void action_reap(void) {
	printf("Alas! One of my children died, and now I have to reap the zombie it has become!! :(((\n");
	(waitpid(-1, NULL, WNOHANG) > 0)
		;
}

