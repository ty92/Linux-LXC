/**
 * 此程序执行需要使用root权限，因为涉及到高级操作
 * 显示子进程pid == 1(pid隔离映射的新的PID)；在子进程/bin/bash中kill父进程号；
 * 会显示报错：No such process;
 * 因为namespacs PID隔离了PID
 * 在父进程时可以使用top 或 "ps exf"命令显示自己和子进程（未映射的）的PID
 * 会发现在子进程时使用ps命令和父进程时的内容一模一样，
 * 是因为这些工具否是从真实的"/proc"文件系统中获取信息，而/proc是尚未隔离的
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
	read(checkpoint[0], &c, 1);
	printf("I am [%5d] child \n",getpid());
	sethostname("In Namespace",12);
	execv(child_args[0], child_args);
	printf("EXECV ERROR\n");
	exit(EXIT_FAILURE);
}

int main()
{
	pipe(checkpoint);

	printf("I am [%5d] parent ? \n",getpid());
	int child_pid = clone(child_main, child_stack + STACK_SIZE, CLONE_NEWUTS | CLONE_NEWIPC | CLONE_NEWPID | SIGCHLD, NULL);
	if(child_pid == -1)
		perror("clone");
	
	sleep(4);
	close(checkpoint[1]);
	waitpid(child_pid, NULL, 0);
	printf("father\n");
	exit(EXIT_SUCCESS);
}
