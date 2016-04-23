/**
 * 使用CLONE_NEWNET隔离网络；
 * 父进程建立veth0，veth1 启动veth0并设置ip，clone 子进程
 * 子进程启动veth1，并添加ip；此时父进程和子进程的网络已经被隔离，
 * 父进程中只能看见veth0，子进程可以看到veth1；二者可以ping同；
 * 可以通过如下命令进行通信：
 * 子进程：nc -l 4242(回车后输入字符串回车)
 * 父进程或另外一个终端：nc -l 子进程veth1的ip 4242（nc -l 169.254.1.2 4242 ）
 * 就可以接收到紫禁城发送过来的消息
 **/
#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <sched.h>
#include <signal.h>
#include <unistd.h>

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
	printf("[%5d] - child \n",getpid());
	sethostname("In Namespace",12);
//	mount("proc", "/proc", "proc", 0, NULL);
	read(checkpoint[0], &c, 1);
//setup network
	system("ip link set lo up");
	system("ip link set veth1 up");
	system("ip addr add 169.254.1.2/30 dev veth1");
	execv(child_args[0], child_args);
	printf("EXECV ERROR\n");
	exit(EXIT_FAILURE);
}

int main()
{
	pipe(checkpoint);

	printf("[%5d] - parent \n",getpid());

	int child_pid = clone(child_main, child_stack + STACK_SIZE, CLONE_NEWUTS | CLONE_NEWIPC | CLONE_NEWPID | CLONE_NEWNS | CLONE_NEWNET | SIGCHLD, NULL);
	if(child_pid == -1)
		perror("clone");
//father init: create a veth pair
	char * cmd;
	asprintf(&cmd,"ip link set veth1 netns %d", child_pid);
	system("ip link add veth0 type veth peer name veth1");
	system(cmd);
	system("ip link set veth0 up");
	system("ip addr add 169.254.1.1/30 dev veth0");
	//free(cmd);
	sleep(4);
	close(checkpoint[1]);
	waitpid(child_pid, NULL, 0);
	printf("father\n");
	exit(EXIT_SUCCESS);
}
