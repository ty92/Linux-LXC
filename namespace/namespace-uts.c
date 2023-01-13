/**
 * 此程序执行需要使用root权限，因为涉及到uts
 **/
#define _GNU_SOURCE   //功能测试宏，可以在程序中定义（必须程序开头），也可以在编译时指定 gcc -D_GNU_SOURCE 程序名
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

#define STACK_SIZE (1024 * 1024)

static char child_stack[STACK_SIZE];   //定义成void *child_stack时，子进程不执行
char * const child_args[] = {
	"/bin/bash",
	NULL
};

int child_main(void* arg) {
	printf("Child inside Namespace\n");
	sethostname("In Namespace",12);		//子进程中设置hostname，区分父子进程UTS命令空间
	execv(child_args[0], child_args);
	printf("execv error\n");
	return -1;
}

int main()
{
	printf("Parent outside Namespace \n");
	int child_pid = clone(child_main, child_stack + STACK_SIZE, CLONE_NEWUTS | SIGCHLD, NULL); //函数名代表函数的首地址，此处也可以写成&child_main
												  //子进程栈空间，不加STACK_SIZE时，无法执行子进程的/bin/bash
	if(child_pid == -1)
		perror("clone");
	waitpid(child_pid, NULL, 0);
	printf("child exit...\n");
	
	exit(EXIT_SUCCESS);	
}
