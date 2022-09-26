#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

void
find(char *path, char *filename)
{
  	// 调用此函数是可以确定path是一个路径
  	char buf[512], *p;
  	int fd;
  	struct dirent de;
  	struct stat st;

  	if((fd = open(path, 0)) < 0){
    		fprintf(2, "find: cannot open %s\n", path);
    		return;
  	}

  	if(fstat(fd, &st) < 0){
    		fprintf(2, "find: cannot stat %s\n", path);
    		close(fd);
    		return;
  	}

  	if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
  	  	fprintf(2, "find: path too long\n");
  	  	return;
  	}
  	strcpy(buf, path);
  	p = buf+strlen(buf);
  	*p++ = '/';						// 在路径末尾添加/
  	
  	while(read(fd, &de, sizeof(de)) == sizeof(de)){	// 读取当前目录下所有文件
  	  	if(de.inum == 0 || strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
  	    		continue;
  	  	memmove(p, de.name, DIRSIZ);
  	  	p[DIRSIZ] = 0;

  	  	if(stat(buf, &st) < 0){
  	  	  	fprintf(2, "find: cannot stat %s\n", buf);
  	  	  	continue;
  	  	}
  	    
  	  	if (strcmp(de.name, filename) == 0) {
  	  	  	printf("%s\n", buf);
  	  	}
		
		if (st.type == T_DIR) {
	        	find(buf, filename);
	    	}
	
	}
	close(fd);
	return ;
}

int
main(int argc, char *argv[])
{
	if(argc != 3) {
    		printf("find: needs exactly two argument!\n");
    		exit(-1);
  	}

  	int fd;		// 文件描述符
  	struct stat st;

  	if((fd = open(argv[1], 0)) < 0){
    		fprintf(2, "find: cannot open %s\n", argv[1]);
		close(fd);
    		return -1;
  	}

  	if(fstat(fd, &st) < 0){
    		fprintf(2, "find: cannot stat %s\n", argv[1]);
    		close(fd);
    		return -1;
  	}

  	switch(st.type){
  	case T_FILE:
    		fprintf(2, "find: %s is not a dictionary!\n", argv[1]);			// 如果搜索的路径不是一个目录，打印异常信息并且退出
   		break;

  	case T_DIR:				// 如果搜索路径是一个目录，递归调用find函数
    		find(argv[1], argv[2]);
    		break;
  	}
	close(fd);
	exit(0);
}
