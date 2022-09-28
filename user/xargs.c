#include <stddef.h>
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define MAXLINE 128		// 每行最多输入128个字符
#define MAXARGV 64		// 最多接收64个参数

static char *argvs[MAXARGV];	// 储存参数, 最多储存64个参数
static char line[MAXLINE];	// 行缓冲区，读入每行
static char *freebuf[MAXARGV];	// 记录每次malloc申请内存的首地址，方便回收
static char *buf;		// 用于处理当前行输入的字符指针
static size_t bufcnt = 0;	// 记录总共使用malloc分配了多少块buf
static size_t argvsc  = 0;	// 记录最终执行时的参数个数
static int  st;			// 退出xargs时的变量，为0表示执行xargs成功

int parseline(void) {
	char *bufend = buf + strlen(buf);
	
	// replace trailing '\0' with space
	*bufend = ' ';
	
	// skipping the leading spaces and leading empty line
	for ( ; buf != bufend && (*buf == ' ' || *buf == '\n'); ++buf)
		;

	char *delim = buf;
	while( buf < bufend) {
		while(delim < bufend  && *delim != ' ' && *delim != '\n')
			++delim;
		argvs[argvsc++] = buf;
		*delim = '\0';
		for (buf = delim + 1; buf < bufend && (*buf == ' ' || *buf == '\n'); ++buf)
			;			// 跳过空格和空行
		if (argvsc == MAXARGV) {
			/*
			 * 如果储存的参数个数已经打到
			 * 最大能储存的参数个数，
			 * 设置st为-1，返回-1表示失败
			 */
			printf("xargs: Too much argument!\n");
			st = -1;
			return -1;
		}
	}
	return 0;
}

int main(int argc, char *argv[]) {	
	st = 0;
	if (argc == 1) {
		/* 如果没有指定指令，也没有指定初始参数，则执行echo指令 */
		argvs[0] = malloc(5 * sizeof(char));
  		memcpy(argvs[0], "echo", 5);
		freebuf[bufcnt++] = argvs[0];
		++argvsc;
	}

	for (int i = 1; i < argc; ++i) {	// 移入初始字符串
		argvs[argvsc++] = argv[i];
	}

	int readlen = 0;		// 从标准输入读取到的字节数
  	while( (readlen = read(0, line, MAXLINE)) != 0 && *line != -1) {
		/* 每次从标准输入读入一块字符串。
		 * 需要注意的是，在管道语句中，
		 * 每次读入的字符串可以会使用
		 * 换行符作为参数间的分隔符 */
		if (readlen == MAXLINE && line[MAXLINE - 1] != '\0') {
			/* 我们需要提醒用户可能发生的截断 */
			printf("xargs: Lines with more than 128 chars might be truncted!\n");
			st = -1;
			/* 
			 * 设置退出状态，释放内存
			 * */
			goto FREEBUF;
		}
		if (readlen == 1 && line[0] == '\n')
			/* 跳过只含有'\n'的空行 */
			continue;

		freebuf[bufcnt++] = buf = (char *)malloc( readlen * sizeof(char));
		memcpy(buf, line, readlen);
		*(buf + readlen - 1) = '\0';
		if (parseline() < 0)
			goto FREEBUF;
  	}
	argvs[argvsc] = NULL;
	
	// 执行程序
	switch(fork()) {
		case -1: 
			printf("xargs: fork error!\n");
			st = -1;
			goto FREEBUF;
		case 0:
			if (exec(argvs[0], argvs) < 0) {
				printf("xargs: fail to execute %s\n", argvs[0]);
				st = -1;
				goto FREEBUF;
			}
		default:
			wait(0);	
	}

FREEBUF:
	for (int i = 0; i < bufcnt; ++i){
		free(freebuf[i]);
		freebuf[i] = NULL;
	}
  	exit(st);
}
