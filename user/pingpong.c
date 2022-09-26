#include "kernel/types.h"
#include "user.h"

int main(int argc, char* argv[]){
    if (argc > 1) {
    	printf("Pingpong needs no argument!\n");
	exit(-1);
    }
    int p1[2], p2[2];           // 定义管道出入口
    if (pipe(p1) < 0) { 
	printf("Fail to construct a pipe!\n");
	exit(-1);
    }
    if (pipe(p2) < 0) { 
	printf("Fail to construct a pipe!\n");
	exit(-1);
    }
    
    char revbuf[8];
    int pid = 0;
    if ((pid = fork()) < 0) {
    	printf("Fork error!\n");
    } 
    else if (pid == 0) {        // 子进程
    	close(p1[1]);		// 关闭管道一的写端
	if (read(p1[0], revbuf, strlen("ping")) != strlen("ping")) {
	    printf("Child process read error!\n");
	    exit(-1);
	}
	close(p1[0]);		// 读完毕关闭读端
	printf("%d: received %s\n", getpid(), revbuf);
	
    	close(p2[0]);		// 关闭管道二的读端
	if (write(p2[1], "pong", strlen("pong")) != strlen("pong")) {
	    printf("Child process write error!\n");
	    exit(-1);
	}
	close(p2[1]);		// 写完毕关闭写端
	

	exit(0);		// 子进程显式返回
    }
    else {			// 父进程
	close(p1[0]);
	if (write(p1[1], "ping", strlen("ping")) != strlen("ping")) {
	    printf("Parent process write error!\n");
	    exit(-1);
	}
	close(p1[1]);		// 写完毕关闭写端

    	close(p2[1]);		// 关闭管道一的写端
	if (read(p2[0], revbuf, strlen("pong")) != strlen("pong")) {
	    printf("Parent process read error!\n");
	    exit(-1);
	}
	close(p2[0]);		// 读完毕关闭读端
	printf("%d: received %s\n", getpid(), revbuf);
	
    	if ( wait((int *) 0) == pid)
		exit(0);
	else
		exit(-1);
    }
    exit(0);
}
