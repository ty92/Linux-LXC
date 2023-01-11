/**
 * 此程序执行需要使用root权限，因为涉及到uts
 * CLONE_NEWIPC隔离信号量、共享内存、消息队列，不隔离管道pipe；
 * father process create pipe,father process sleep(4)，
 * child process 先执行，关闭管道写端，read 管道checkpoint[0],阻塞
 * 父进程sleep over，关闭写端(触发一个“close”事件)，传输“EOF”结束符到child的读取端
 * child接收到"EOF"后，继续运行后边的语句。
 *
 * 该程序主要是隔离IPC（进程间通信），但是不隔离pipe通信，此处程序使用管道主要是为了体现出
 * 隔离了IPC，但是还可以使用管道通信
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
	printf("- world !\n");
	sethostname("In Namespace",12);
	execv(child_args[0], child_args);
	printf("EXECV ERROR\n");
	exit(EXIT_FAILURE);
}

int main()
{
	pipe(checkpoint);

	printf("- Hello ? \n");
	int child_pid = clone(child_main, child_stack + STACK_SIZE, CLONE_NEWUTS | CLONE_NEWIPC | SIGCHLD, NULL);
	if(child_pid == -1)
		perror("clone");
	
	sleep(4);		//parent睡眠，子进程读管道（zuse），等到父进程唤醒，关闭写端，子进程从管道接收到关闭管道消息，继续执行后边语句
	close(checkpoint[1]);	
	waitpid(child_pid, NULL, 0);
	printf("father\n");
	exit(EXIT_SUCCESS);
}
