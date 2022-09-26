#include "kernel/types.h"
#include "user.h"

void recursion(int parentpip[]) {
	char prime;
	close(parentpip[1]);
	if (read(parentpip[0], &prime, 1) == 0)	// 如果从父进程没有得到输入终止递归
		exit(0);

	printf("prime %d\n", (int)prime);		// 打印质数
	int childpip[2];
	if (pipe(childpip) < 0) {
		printf("Fail to construct a pipeline!\n");
		exit(-1);
	}

	int pid = fork();
	if (pid < 0) {
		printf("Fail to fork a subprocess!\n");
		exit(-1);
	}
	else if (pid == 0) {		// 子进程
		recursion(childpip);
	}
	else {				// 父进程
		char num;
		close(childpip[0]);
		while(read(parentpip[0], &num, 1) == 1) {
			if ((int)num % prime != 0)
				write(childpip[1], &num, 1);
		}
		close(childpip[1]);
		close(parentpip[0]);
		wait((int *) 0);
	}
	exit(0);
}

int main(int argc, int *argv) {
	// 检查参数个数
	if (argc > 1) {
		printf("Prime needs no argument\n");
		exit(-1);
	}

	int pip[2];
	if (pipe(pip) < 0) {
		printf("Fail to construct a pipeline!\n");
		exit(-1);
	}

	int pid = fork();
	if (pid < 0) {
		printf("Fail to fork a subprocess!\n");
		exit(-1);
	}
	else if (pid == 0) {		// 子进程
		recursion(pip);
	}
	else {				// 父进程
		close(pip[0]);
		for (int i = 2; i < 36; ++i) {
			write(pip[1], &i, 1);
		}
		close(pip[1]);
		wait((int*) 0);
	}
	exit(0);
}
