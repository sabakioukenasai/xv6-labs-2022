#include <stddef.h>
#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fs.h"
#include "user/user.h"

#define MAXLINE 128		// 每行最多输入128个字符
#define MAXARGV 64		// 最多接收64个参数
#define MAXEXEV 64		// 最多调用指令64次
#define MAXNAME 64		// 指令最大长度为64

char instruct[MAXNAME];		// 储存指令
char *argvs[MAXEXEV][MAXARGV];	// 储存参数, 每个指针储存一行参数
char line[MAXLINE];		// 行缓冲区，读入每行

/**
 * Parse the command line and build the argv array
 * */
void parseline(char *buf, char **argv) {
	printf("buf points to %p\n", buf);
	char *delim;
	int argc = 0;
	char *bufend = buf + strlen(buf);
	
	// replace trailing '\n' with space
	buf[strlen(buf) - 1] = ' ';
	
	// skipping the leading spaces
	for ( ; buf != bufend && *buf == ' '; ++buf)
		;

	while((delim = strchr(buf, ' ')) != NULL) {
		argv[++argc] = buf;
		*delim = '\0';
		for (buf = delim + 1; buf != bufend && *buf == ' '; ++buf)
			;
	}
	argv[argc + 1] = NULL;		// 参数列表以NULL结尾
	return;
}

int main(int argc, char *argv[]) {	
  	int calltimes = 0;		// 调用函数次数
	
	if (argc == 1) {		// 如果没有指定指令，也没有指定初始参数，则执行echo指令
  		memmove(instruct, "echo", 5);
  	}
  	else {				// 否则，设置argv[1]为待执行指令
   		memmove(instruct, argv[1], strlen(argv[1]));
  		// 转移初始参数到argvs中
  		for (int i = 2; i < argc; ++i) {
			strcpy(argvs[calltimes][i - 1], argv[i]);
  		}
  	}

  	while( read(0, line, sizeof(line)) != 0 && *line != -1) {	// 每次从标准输入读入一行
		char *buf = (char *)malloc(strlen(line) * sizeof(char));// 申请与输入同等大小的缓冲区
		strcpy(buf, line);
		printf("buf points to %p, line points to %p\n", buf, line);
		parseline(buf, argvs[++calltimes]);
		strcpy(argvs[calltimes][0], instruct);			// 将argv[0]设置为待执行的指令
  	}

	printf("here\n");

	for (int i = 0; i < calltimes; ++i) {
		int pid = fork();
		if (pid < 0) {
			printf("xargs: fork error!\n");
		}
		else if (fork() == 0) {
			if (exec(instruct, argvs[i]) < 0) {	// 在子进程直接调用exec执行目标指令
				printf("xargv: fail to execute %s\n", instruct);
				exit(-1);			// 子进程非正常终止
			}
		}
	}

	for (int i = 0; i < calltimes; ++i){
		char **argv = argvs[i];
		while(*argv) {
			free(*argv);
			++argv;
		}
	}
  	exit(0);
}
