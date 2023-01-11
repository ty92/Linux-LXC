/**
 * 使用CLONE_NEWNS隔离挂载点，涉及到挂载私有属性的问题
 * Centos挂载有私有属性问题 shared slave private三个属性；
 * 只有将其设置为private才会真正实现挂载隔离：mount --make-private /proc,设置/proc目录挂载属性为private；
 * 当挂载属性不为private时，子进程挂载后未umount直接exit时，会影响其他进程使用，
 * 此时可以在其它进程时重新挂载一次即可mount -t proc proc /proc
 **/
#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <sched.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

#define STACK_SIZE (1024 * 1024)

int checkpoint[2];
static char child_stack[STACK_SIZE];
char * const child_args[] = {
	"/bin/bash",
	NULL
};

int child_main(void* arg) {
	char c;
	
	close(checkpoint[1]);
	printf("- [%5d] world \n",getpid());
	sethostname("In Namespace",12);
//	mount("proc", "/proc", "proc", 0, NULL);
	read(checkpoint[0], &c, 1);
	execv(child_args[0], child_args);
	printf("EXECV ERROR\n");
	exit(EXIT_FAILURE);
}

int main()
{
	pipe(checkpoint);

	printf("- [%5d] Hello ? \n",getpid());
	int child_pid = clone(child_main, child_stack + STACK_SIZE, CLONE_NEWUTS | CLONE_NEWIPC | CLONE_NEWPID | CLONE_NEWNS | SIGCHLD, NULL);
	if(child_pid == -1)
		perror("clone");
	
	sleep(4);
	close(checkpoint[1]);
	waitpid(child_pid, NULL, 0);
	printf("father\n");
	exit(EXIT_SUCCESS);
}
